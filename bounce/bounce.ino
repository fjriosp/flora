// Googly Eye Goggles
// By Bill Earl
// For Adafruit Industries
//
// The googly eye effect is based on a physical model of a pendulum.
// The pendulum motion is driven by accelerations in 2 axis.
// Eye color varies with orientation of the magnetometer
 
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_LSM303_U.h>
#include <Adafruit_NeoPixel.h>

#define DEBUG

#ifdef DEBUG
  #define LOG(x)  Serial.print(x)
#else
  #define LOG(x)
#endif

#define RING_IO 12

Adafruit_NeoPixel strip = Adafruit_NeoPixel(16, RING_IO, NEO_GRB + NEO_KHZ800);
Adafruit_LSM303_Accel_Unified accel = Adafruit_LSM303_Accel_Unified(1000);
Adafruit_LSM303_Mag_Unified   mag   = Adafruit_LSM303_Mag_Unified(1001);

const float FRICTION = 0.95;
const float POS_DIFF = -0.5;
const float GRAVITY  = 100;

float pos = 0;  // Starting center position of pupil
float increment = 2 * PI / 16; // distance between pixels in radians
float speedX = 0;
float speedY = 0;

#define M_GAME   0
#define M_BRIGHT 1
#define M_LIGHT  2
uint8_t mode = M_GAME;

long hintT     = millis();

// GAME
long physicsT  = millis();
// BRIGHT
long brightT   = millis();
uint8_t bright = 16;

void setup(void) {
  Serial.begin(9600);
  
  strip.begin();
  strip.setBrightness(bright);
  strip.show();
   
  accel.begin();
  mag.begin();
}

// main processing loop
void loop(void) {
  hint();
  
  if(mode == M_GAME) {
    updatePhysics();
    gameView();
  } else if(mode == M_BRIGHT) {
    adjustBright();
  }
}

/* MENU */
void hint(void) {
  sensors_event_t event; 
  accel.getEvent(&event);
  
  long t = millis() - hintT;
  
  if(event.acceleration.x > 9) {
    if(t >= 1000) {
      brightBegin();
    }
  } else if(event.acceleration.x < -9) {
    if(t >= 1000) {
      lightBegin();
    }
  } else {
    hintT = millis();
  }
}

/* LIGHT */
void lightBegin(void) {
  mode = M_LIGHT;
  brightT = millis();
  strip.setBrightness(0xFF);
  for (uint8_t i = 0; i < 16; i++)
    strip.setPixelColor(i, 0xFFFFFF);
  strip.show();
}

/* BRIGHT */
void brightBegin(void) {
  mode = M_BRIGHT;
  brightT = millis();
  
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

void adjustBright(void) {
  sensors_event_t event; 
  mag.getEvent(&event);

  float heading = abs(atan2(event.magnetic.y,event.magnetic.x) * 180) / PI;
  
  uint8_t b = min(round(heading/15 + 1) * 8, 255);
  
  LOG("heading: "); LOG(heading); LOG("\n");
  LOG("bright:  "); LOG(bright); LOG("\n");
  
  if(bright != b)
    brightT = millis();
  
  bright = b;
  strip.setBrightness(bright);
  strip.show();

  if(millis() - brightT >= 5000)
    gameBegin();
}

/* GAME */
void gameBegin(void) {
  mode = M_GAME;
  physicsT = millis();
}

void updatePhysics(void) {
  float incT = (millis() - physicsT) / 1000.0;
  physicsT = millis();
  
  // Now read the accelerometer to control the motion.
  sensors_event_t event; 
  accel.getEvent(&event);
  
  // Calculate the horizontal and vertical effect on the virtual pendulum
  // 'pos' is a pixel address, so we multiply by 'increment' to get radians.
  float incX = cos(pos * increment);  // peaks at top and bottom of the swing
  float incY = sin(pos * increment);  // peaks when the pendulum is horizontal
  
  // Slowdown the current movement
  speedX *= FRICTION;
  speedY *= FRICTION;
  
  // Accelerate
  speedX -= incX * (event.acceleration.x * GRAVITY * incT);
  speedY -= incY * (event.acceleration.y * GRAVITY * incT);
  
  LOG("speed: "); LOG(speedX); LOG(" / "); LOG(speedY); LOG("\n");
  
  // Calculate the new position
  pos += speedX * incT;
  pos += speedY * incT;
  
  LOG("pos: "); LOG(pos); LOG("\n");
  
  while (round(pos) < 0)  pos += 16.0;
  while (round(pos) > 15) pos -= 16.0;
  
  LOG("pos: "); LOG(pos); LOG("\n");
}

void gameView() {
  float showPos = pos + POS_DIFF;
  
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
