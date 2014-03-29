#ifndef __PCM_H__
#define __PCM_H__

#include <Arduino.h>

// Tone Frequencies Mask
typedef enum {
  PCM_N   = 0x00,
  PCM_D6  = 0x08,
  PCM_s5  = 0x10,
  PCM_sb5 = 0x18,
  PCM_L5  = 0x20,
  PCM_Lb5 = 0x28,
  PCM_S5  = 0x30,
  PCM_Sb5 = 0x38,
  PCM_F5  = 0x40,
  PCM_M5  = 0x48,
  PCM_Mb5 = 0x50,
  PCM_R5  = 0x58,
  PCM_Rb5 = 0x60,
  PCM_D5  = 0x68,
  PCM_s4  = 0x70,
  PCM_sb4 = 0x78,
  PCM_L4  = 0x80,
  PCM_Lb4 = 0x88,
  PCM_S4  = 0x90,
  PCM_Sb4 = 0x98,
  PCM_F4  = 0xA0,
  PCM_M4  = 0xA8,
  PCM_Mb4 = 0xB0,
  PCM_R4  = 0xB8,
  PCM_Rb4 = 0xC0,
  PCM_D4  = 0xC8,
  PCM_s3  = 0xD0,
  PCM_sb3 = 0xD8,
  PCM_L3  = 0xE0,
  PCM_Lb3 = 0xE8,
  PCM_S3  = 0xF0,
  PCM_Sb3 = 0xF8
} pcm_tone_mask;

// Note Time Mask
typedef enum {
  PCM_0  = 0x00,
  PCM_1  = 0x01,
  PCM_2  = 0x02,
  PCM_4  = 0x03,
  PCM_8  = 0x04,
  PCM_16 = 0x05,
  PCM_32 = 0x06,
  PCM_64 = 0x07
} pcm_time_mask;

const prog_uint16_t PCM_TONE[] PROGMEM = {0,477,505,535,567,601,637,675,715,757,803,850,901,955,1011,1072,1135,1203,1275,1350,1431,1516,1606,1702,1803,1910,2024,2144,2272,2407,2550,2702,2862};
const prog_uint16_t PCM_TIME[] PROGMEM = {0,15624,7812,3905,1952,976,487,243};

void pcm_begin();
void pcm_play(uint16_t _pcm);
void pcm_play(uint16_t _pcm, boolean _loop);
void pcm_stop();

#endif
