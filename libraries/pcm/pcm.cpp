#include <pcm.h>

boolean  pcm_loop   = 0;
uint16_t pcm_init   = NULL;
uint16_t pcm_next   = NULL;
float    pcm_volume = 1.0;

void pcm_tone(uint16_t _tone) {
  cli();
  
  // Stop timer1
  TCCR1B = 0;
  TCCR1A = 0;
  TCNT1 = 0;
  OCR1A = 0;
  
  if(_tone > 0) {
    // Set tone
    OCR1A = _tone;
    OCR1B = OCR1A * pcm_volume;
    // Enable timer1
    TCCR1A = _BV(COM1B0);
    TCCR1B = _BV(WGM12) | _BV(CS11);
  }
  
  sei();
}

boolean pcm_time(uint16_t _time) {
  cli();
  
  // Stop timer3
  TCCR3B = 0;
  TCCR3A = 0;
  TCNT3 = 0;
  OCR3A = 0;
  TIMSK3 = 0;
  
  if(_time > 0) {
    // Set time
    OCR3A = _time;
    // Enable timer3
    TCCR3B = _BV(WGM32) | _BV(CS00) | _BV(CS32);
    TIMSK3 = _BV(OCIE3A);
  }
  
  sei();
  
  return (_time > 0);
}

void pcm_beat() {
  uint8_t  data = pgm_read_byte(pcm_next);
  uint16_t tone = pgm_read_word(PCM_TONE + (data >> 3));
  uint16_t time = pgm_read_word(PCM_TIME + (data & 0x07));
  
  pcm_tone(tone);
  boolean hasNext = pcm_time(time);
  
  pcm_next++;
  
  if(!hasNext && pcm_loop) {
    pcm_next = pcm_init;
    pcm_beat();
  }
}

void pcm_begin() {
  pinMode(10, OUTPUT);
  digitalWrite(10, LOW);
}

void pcm_play(uint16_t _pcm) {
  pcm_play(_pcm, false);
}

void pcm_play(uint16_t _pcm, boolean _loop) {
  pcm_loop = _loop;
  pcm_init = _pcm;
  pcm_next = _pcm;
  pcm_beat();
}

void pcm_stop() {
  pcm_time(0);
  pcm_tone(0);
}

ISR(TIMER3_COMPA_vect) {
  pcm_beat();
}