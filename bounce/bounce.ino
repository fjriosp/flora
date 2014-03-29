#include <pcm.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_LSM303_U.h>
#include <Adafruit_NeoPixel.h>

//#define DEBUG
#ifdef DEBUG
  #define LOG(x)  Serial.print(x)
#else
  #define LOG(x)
#endif

// Music
const prog_uint8_t  CUCARACHA[] PROGMEM = {
PCM_N |PCM_8, PCM_D4|PCM_8, PCM_D4|PCM_8, PCM_D4|PCM_8, 
PCM_F4|PCM_4, PCM_L4|PCM_8, PCM_D4|PCM_8, PCM_D4|PCM_8, PCM_D4|PCM_8,
PCM_F4|PCM_4, PCM_L4|PCM_4, PCM_N |PCM_4,
PCM_F4|PCM_8, PCM_F4|PCM_8, PCM_M4|PCM_8, PCM_M4|PCM_8, PCM_R4|PCM_8, PCM_R4|PCM_8,
PCM_D4|PCM_2, PCM_N |PCM_8, PCM_D4|PCM_8, PCM_D4|PCM_8, PCM_D4|PCM_8,
PCM_F4|PCM_4, PCM_L4|PCM_8, PCM_D4|PCM_8, PCM_D4|PCM_8, PCM_D4|PCM_8,
PCM_F4|PCM_4, PCM_L4|PCM_4, PCM_N |PCM_4,
PCM_R5|PCM_8, PCM_M5|PCM_8, PCM_R5|PCM_8, PCM_D5|PCM_8, PCM_s4|PCM_8, PCM_L4|PCM_8, 
PCM_S4|PCM_4, PCM_N |PCM_2,
PCM_N |PCM_0
};

const prog_uint8_t  SIMON[4][2] PROGMEM = {
  {PCM_D4|PCM_4, PCM_N |PCM_0},
  {PCM_M4|PCM_4, PCM_N |PCM_0},
  {PCM_S4|PCM_4, PCM_N |PCM_0},
  {PCM_s4|PCM_4, PCM_N |PCM_0}
};

// Constants
const uint8_t RING_IO   = 12;
const uint8_t BOTTOM_LED  = 0;
const float   RAD_PIXEL = 2 * PI / 16; // distance between pixels in radians

const float   FRICTION  = 0.975;
const float   GRAVITY   = 150;

const uint8_t M_MENU   = 0;
const uint8_t M_BALL   = 1;
const uint8_t M_BRIGHT = 2;
const uint8_t M_LIGHT  = 3;

const uint8_t B_NONE  = 0;
const uint8_t B_BACK  = 1;
const uint8_t B_UP    = 2;
const uint8_t B_DOWN  = 3;
const uint8_t B_LEFT  = 4;
const uint8_t B_RIGHT = 5;

const long    H_MINT =  100;
const long    H_MAXT =  500;

typedef void (*call_t)(void);

// Globals
Adafruit_NeoPixel strip = Adafruit_NeoPixel(16, RING_IO);
Adafruit_LSM303_Accel_Unified accel = Adafruit_LSM303_Accel_Unified(1000);
Adafruit_LSM303_Mag_Unified   mag   = Adafruit_LSM303_Mag_Unified(1001);

call_t  mode_loop = NULL;

uint8_t bright = 16;

// Gesture
uint8_t gest_tmp   = B_NONE;
long    gest_time  = millis();
call_t  gest_back  = NULL;
call_t  gest_up    = NULL;
call_t  gest_down  = NULL;
call_t  gest_left  = NULL;
call_t  gest_right = NULL;

// BALL
long  ball_time  = millis();
float ball_pos   = 0; // Ball position
float ball_speed = 0; // Ball speed

// SIMON
long     simon_seed  = millis();
uint8_t  simon_levl  = 1;
uint8_t  simon_curr  = 0;
uint32_t simon_color[] = {0xFF0000,0x00FF00,0x0000FF,0xFFFF00};

// Setup
void setup(void) {
  Serial.begin(9600);
  
  pcm_begin();
  
  strip.begin();
  strip.setBrightness(bright);
  strip.show();
   
  accel.begin();
  
  menu_begin();
}

// Main loop
void loop(void) {
  uint8_t h = gest_check();
  
  if(mode_loop != NULL)
    mode_loop();
    
  if(h != B_NONE) {
    LOG("GESTURE: "); LOG(h); LOG("\n");
  }
  
  if     (h == B_BACK  && gest_back  != NULL) gest_back();
  else if(h == B_UP    && gest_up    != NULL) gest_up();
  else if(h == B_DOWN  && gest_down  != NULL) gest_down();
  else if(h == B_LEFT  && gest_left  != NULL) gest_left();
  else if(h == B_RIGHT && gest_right != NULL) gest_right();
}

/* GENERAL */
uint8_t gest_check(void) {
  sensors_event_t e;
  accel.getEvent(&e);
  
  long t = millis() - gest_time;
  uint8_t res = B_NONE;
  
  // Check current position
  if(e.acceleration.z < -6) {
    gest_tmp = B_BACK;
  } else if(e.acceleration.x < -4) {
    if(gest_tmp == B_NONE)
      gest_tmp = B_RIGHT;
  } else if(e.acceleration.x > 4) {
    if(gest_tmp == B_NONE)
      gest_tmp = B_LEFT;
  } else if(e.acceleration.y < -4) {
    if(gest_tmp == B_NONE)
      gest_tmp = B_UP;
  } else if(e.acceleration.y > 4) {
    if(gest_tmp == B_NONE)
      gest_tmp = B_DOWN;
  } else { // Neutral => Execute Hint
    res = gest_tmp;
    gest_tmp = B_NONE;
    gest_time = millis();
  }
  
  // Hint Timing
  if(t < H_MINT || t > H_MAXT)
    gest_tmp = B_NONE;
  
  return res;
}

/* MENU */
void menu_begin(void) {
  mode_loop   = NULL;
  gest_back   = NULL;
  gest_up     = *ball_begin;
  gest_down   = *simon_begin;
  gest_left   = *bright_begin;
  gest_right  = *light_begin;
  
  pcm_play((uint16_t)CUCARACHA);
  
  for (int8_t i = 0; i < 16; i++) {
    strip.setPixelColor(i, 0x000000);
  }
  
  for (int8_t i = -1; i <= 1; i++) {
    strip.setPixelColor(( 0 + i + 16 + BOTTOM_LED) % 16, 0xFF0000);
    strip.setPixelColor(( 4 + i + 16 + BOTTOM_LED) % 16, 0x00FF00);
    strip.setPixelColor(( 8 + i + 16 + BOTTOM_LED) % 16, 0x0000FF);
    strip.setPixelColor((12 + i + 16 + BOTTOM_LED) % 16, 0xFFFF00);
  }
  strip.setBrightness(bright);
  
  strip.show();
}

/* SIMON */
void simon_begin(void) {
  mode_loop   = NULL;
  gest_back   = *menu_begin;
  gest_up     = *simon_up;
  gest_down   = *simon_down;
  gest_left   = *simon_left;
  gest_right  = *simon_right;
  
  pcm_stop();
  
  simon_seed = millis();
  simon_levl = 1;
  simon_curr = 0;

  strip.setBrightness(bright);
  simon_clear();
  delay(250);
  for(uint8_t i=0; i<4; i++) {
    simon_show(i,false);
    delay(250);
  }
  simon_clear();
  delay(500);
  
  simon_train();
}

void simon_train(void) {
  randomSeed(simon_seed);
  
  for(int i = 0; i < simon_levl; i++) {
    simon_clear();
    simon_show((uint8_t)random(4));
    delay(500);
    simon_clear();
    delay(100);
  }
  
  randomSeed(simon_seed);
}

void simon_clear(void) {
  for (int8_t i = 0; i < 16; i++) {
    strip.setPixelColor(i, 0x000000);
  }
  strip.show();
}

void simon_show(uint8_t v) {
  simon_show(v,true);
}

void simon_show(uint8_t v, boolean sound) {
  if(sound)
    pcm_play((uint16_t)SIMON[v]);
  strip.setPixelColor(((v*4) - 1 + 16 + BOTTOM_LED) % 16, simon_color[v]);
  strip.setPixelColor(((v*4)     + 16 + BOTTOM_LED) % 16, simon_color[v]);
  strip.setPixelColor(((v*4) + 1 + 16 + BOTTOM_LED) % 16, simon_color[v]);
  strip.show();
}

void simon_next(void) {
  simon_curr++;
  if(simon_curr >= simon_levl) {
    simon_win();
    simon_train();
  }
}

void simon_move(uint8_t v) {
  simon_clear();
  simon_show(v);
  delay(250);
  simon_clear();
  if(random(4) != v) {
    simon_gameover();
  } else {
    simon_next();
  }
}

void simon_down(void) {
  simon_move(0);
}

void simon_right(void) {
  simon_move(1);
}

void simon_up(void) {
  simon_move(2);
}

void simon_left(void) {
  simon_move(3);
}

void simon_win(void) {
  simon_curr = 0;
  simon_levl++;
    
  for(int8_t t = 0; t < 4; t++) {
    for (int8_t i = 0; i < 16; i++) {
      strip.setPixelColor(i, 0x00FF00);
    }
    strip.show();
    delay(250);
    
    for (int8_t i = 0; i < 16; i++) {
      strip.setPixelColor(i, 0x000000);
    }
    strip.show();
    delay(250);
  }
}

void simon_gameover(void) {
  for(int8_t t = 0; t < 4; t++) {
    for (int8_t i = 0; i < 16; i++) {
      strip.setPixelColor(i, 0xFF0000);
    }
    strip.show();
    delay(250);
    
    for (int8_t i = 0; i < 16; i++) {
      strip.setPixelColor(i, 0x000000);
    }
    strip.show();
    delay(250);
  }
  
  simon_begin();
}

/* LIGHT */
void light_begin(void) {
  mode_loop   = NULL;
  gest_back   = *menu_begin;
  gest_up     = NULL;
  gest_down   = NULL;
  gest_left   = NULL;
  gest_right  = NULL;
  
  pcm_stop();
  
  strip.setBrightness(0xFF);
  for (uint8_t i = 0; i < 16; i++)
    strip.setPixelColor(i, 0xFFFFFF);
  strip.show();
}

/* BRIGHT */
void bright_begin(void) {
  mode_loop   = NULL;
  gest_back   = *menu_begin;
  gest_up     = *bright_add;
  gest_down   = *bright_sub;
  gest_left   = *bright_div;
  gest_right  = *bright_mul;
  
  pcm_stop();
  
  for (uint8_t i = 0; i < 16; i++) {
    if(i%4 == 0)
      strip.setPixelColor(i, 0xFF0000);
    else if(i%4 == 1)
      strip.setPixelColor(i, 0x00FF00);
    else if(i%4 == 2)
      strip.setPixelColor(i, 0x0000FF);
    else
      strip.setPixelColor(i, 0xFFFFFF);
  }
  
  strip.show();
}

void bright_add(void) {
  if(bright < 254)
    bright += 2;
  
  strip.setBrightness(bright);
  strip.show();
}

void bright_sub(void) {
  if(bright > 2)
    bright -= 2;
    
  strip.setBrightness(bright);
  strip.show();
}

void bright_div(void) {
  if(bright > B00000001)
    bright >>= 1;
  
  strip.setBrightness(bright);
  strip.show();
}

void bright_mul(void) {
  if(bright < B10000000)
    bright <<= 1;
    
  strip.setBrightness(bright);
  strip.show();
}

/* BALL */
void ball_begin(void) {
  mode_loop   = *ball_loop;
  gest_back   = *menu_begin;
  gest_up     = NULL;
  gest_down   = NULL;
  gest_left   = NULL;
  gest_right  = NULL;
  
  pcm_stop();
  
  ball_time = millis();
}

void ball_loop(void) {
  ball_physics();
  ball_view();
}

void ball_physics(void) {
  float incT = (millis() - ball_time) / 1000.0;
  ball_time = millis();
  
  // Now read the accelerometer to control the motion.
  sensors_event_t e; 
  accel.getEvent(&e);
  
  // Calculate the horizontal and vertical effect on the virtual pendulum
  // 'pos' is a pixel address, so we multiply by 'RAD_PIXEL' to get radians.
  float incX = cos(ball_pos * RAD_PIXEL);  // peaks at top and bottom of the swing
  float incY = sin(ball_pos * RAD_PIXEL);  // peaks when the pendulum is horizontal
  
  // Slowdown the current movement
  ball_speed *= FRICTION;
  
  // Accelerate
  ball_speed += incX * (-e.acceleration.x * GRAVITY * incT);
  ball_speed += incY * (-e.acceleration.y * GRAVITY * incT);
  
  LOG("speed: "); LOG(ball_speed); LOG("\n");
  
  // Calculate the new position
  ball_pos += ball_speed * incT;
  
  LOG("pos: "); LOG(ball_pos); LOG("\n");
  
  while (round(ball_pos) < 0)  ball_pos += 16.0;
  while (round(ball_pos) > 15) ball_pos -= 16.0;
  
  LOG("pos: "); LOG(ball_pos); LOG("\n");
}

void ball_view() {
  float showPos = ball_pos + BOTTOM_LED;
  
  LOG("showPos: "); LOG(showPos); LOG("\n");
  
  while (round(showPos) < 0)  showPos += 16.0;
  while (round(showPos) > 15) showPos -= 16.0;
  
  LOG("showPos: "); LOG(showPos); LOG("\n");
  
  for (int i = 0; i < 16; i++)
  {
    strip.setPixelColor(i, 0x000000);
  }
  
  strip.setPixelColor(showPos, 0x00FF00);
  
  strip.show();
}
