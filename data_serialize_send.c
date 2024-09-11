#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/socket.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>
#include "resource.h"

#define CPU_DATA_PATH "/sys/devices/system/cpu/cpu%d/cpufreq/scaling_cur_freq"
#define CPU_DATA_BUFFER_SIZE 128

uint8_t* resc_data_serialize(resc_serialize resc_s){
  uint8_t *resc_data = malloc(sizeof(resc_serialize) + 1);
  if(!resc_data){
    return NULL;
  } 
  resc_data[0] = 0x00;
  memcpy(resc_data + 1, &resc_s, sizeof(resc_serialize));
  return resc_data; 
}

uint16_t get_cur_average_cpu_freq() {
  float total_freq = 0.0;
  int cpu_count = 0;
  char path[CPU_DATA_BUFFER_SIZE];
  FILE *fp;

  for (int i = 0; ; i++) {
    snprintf(path, sizeof(path), CPU_DATA_PATH, i);

    fp = fopen(path, "r");
    if (!fp) {
      break;  
    }

    int freq;
    if (fscanf(fp, "%d", &freq) == 1) {
      total_freq += freq;
      cpu_count++;
    }

    fclose(fp);
  }

  return (cpu_count > 0) ? (uint16_t)((total_freq / cpu_count)/1000) : 0.0;
}

resc_serialize getResourceData(){
  resc_serialize resc_s;
  FILE *fp;
  char buffer[128];
  //RAM
  fp = popen("free | awk 'NR==2 {print $2; exit}'", "r");
  if (fp == NULL) {
    perror("popen failed");
    return resc_s;
  }
  if (fgets(buffer, sizeof(buffer), fp) != NULL) {
    resc_s.ram = (uint8_t)((float)atoi(buffer) / (1024 * 1024));
  }

  fp = popen("free | awk 'NR==2 {print $3; exit}'", "r");
  if (fp == NULL) {
    perror("popen failed");
    return resc_s;
  }
  if (fgets(buffer, sizeof(buffer), fp) != NULL) {
    resc_s.ram_cur = atof(buffer) / (1024 * 1024);
  }

  //CPU
  resc_s.cpu_freq_cur = get_cur_average_cpu_freq();
  fp = popen("cat /sys/devices/system/cpu/cpu0/cpufreq/scaling_max_freq", "r");
  if (fp == NULL) {
    perror("popen failed");
    return resc_s;
  }
  if (fgets(buffer, sizeof(buffer), fp) != NULL) {
    resc_s.cpu_freq = atoi(buffer) / 1000;    
  }
  fp = popen("sensors |  awk '/Tctl:/ {match($2, /[0-9]+/); print substr($2, RSTART, RLENGTH)}'", "r");
  if (fp == NULL) {
    perror("popen failed");
    return resc_s;
  }
  if (fgets(buffer, sizeof(buffer), fp) != NULL) {
    resc_s.cpu_tmp = atoi(buffer);    
  }

  fp = popen("nvidia-smi --query-gpu=clocks.current.graphics --format=csv,noheader | awk '{print $1}'", "r");
  if (fp == NULL) {
    perror("popen failed");
    return resc_s;
  }
  if (fgets(buffer, sizeof(buffer), fp) != NULL) {
    resc_s.gpu_freq_cur = atoi(buffer);    
  }

  fp = popen("nvidia-smi --query-gpu=clocks.max.graphics --format=csv,noheader | awk '{print $1}'", "r");
  if (fp == NULL) {
    perror("popen failed");
    return resc_s;
  }
  if (fgets(buffer, sizeof(buffer), fp) != NULL) {
    resc_s.gpu_freq = atoi(buffer);    
  }

  fp = popen("nvidia-smi --query-gpu=memory.total --format=csv,noheader | awk '{print $1}'", "r");
  if (fp == NULL) {
    perror("popen failed");
    return resc_s;
  }
  if (fgets(buffer, sizeof(buffer), fp) != NULL) {
    resc_s.gpu_vram = atoi(buffer) / 1024;    
  }

  fp = popen("nvidia-smi --query-gpu=memory.used --format=csv,noheader | awk '{print $1}'", "r");
  if (fp == NULL) {
    perror("popen failed");
    return resc_s;
  }
  if (fgets(buffer, sizeof(buffer), fp) != NULL) {
    resc_s.gpu_vram_cur = atof(buffer) / 1024;    
  }

  fp = popen("nvidia-smi --query-gpu=temperature.gpu --format=csv,noheader | awk '{print $1}'", "r");
  if (fp == NULL) {
    perror("popen failed");
    return resc_s;
  }
  if (fgets(buffer, sizeof(buffer), fp) != NULL) {
    resc_s.gpu_tmp = atoi(buffer);    
  }

  if (pclose(fp) == -1) {
    perror("pclose failed");
  }

  return resc_s;
}

//void show_byte(resc_serialize *resc){
//  uint8_t *p = resc;
//  for(int i = 0; i < sizeof(resc_serialize); i++){
//    printf("0x%.2x ",*(p + i));
//  }
//}

int main(int argc, char **argv)
{
  struct sockaddr_rc addr = { 0 };
  int s, status;
  char dest[18] = "28:CD:C1:11:A7:20";

  while(1){
    resc_serialize resc_t = getResourceData();
    uint8_t *data = resc_data_serialize(resc_t);

   // printf("cpu_Freq:%d\n",resc_t.cpu_freq);
   // printf("cpu_Freq_cur:%d\n",resc_t.cpu_freq_cur);
   // printf("cpu_tmp:%d\n",resc_t.cpu_tmp);
   // printf("gpu_Freq:%d\n",resc_t.gpu_freq);
   // printf("gpu_Freq_cur:%d\n",resc_t.gpu_freq_cur);
   // printf("gpu_tmp:%d\n",resc_t.gpu_tmp);
   // printf("gpu_vram:%d\n",resc_t.gpu_vram);
   // printf("gpu_vram_cur:%.2f\n",resc_t.gpu_vram_cur);
   // printf("ram:%d\n",resc_t.ram);
   // printf("ram_cur:%.2f\n",resc_t.ram_cur);

    //printf("source data:\n");
    //show_byte(&resc_t);
    //printf("\ndata array:\n");
    //for(int i = 0; i < sizeof(resc_serialize) + 1; i++){
    //  printf("0x%.2x ",data[i]);
    //}


    s = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);

    addr.rc_family = AF_BLUETOOTH;
    addr.rc_channel = (uint8_t) 1;
    str2ba( dest, &addr.rc_bdaddr );

    status = connect(s, (struct sockaddr *)&addr, sizeof(addr));

    if( status == 0 ) {
      status = write(s,data, sizeof(resc_serialize) + 1);
    }
    if( status < 0 ) perror("uh oh");
    close(s);
    sleep(2);
  }
  return 0;
}
