#ifndef __resource
#define __resource

#include<stdint.h>

typedef struct 
{
  //cpu
  uint16_t cpu_freq;
  uint16_t cpu_freq_cur;    
  uint8_t  cpu_tmp;

  //gpu
  uint16_t gpu_freq;
  uint16_t gpu_freq_cur;
  uint8_t  gpu_tmp;
  uint8_t  gpu_vram;
  float    gpu_vram_cur; 

  //ram
  uint8_t ram;
  float ram_cur;
}resc_serialize;



#endif
