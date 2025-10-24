/***************************************************************************************
* Copyright (c) 2014-2024 Zihao Yu, Nanjing University
*
* NEMU is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/

#include "debug.h"
#include "difftest-def.h"
#include <SDL2/SDL_audio.h>
#include <common.h>
#include <device/map.h>
#include <SDL2/SDL.h>
#include <stdint.h>
#include <stdio.h>

enum {
  reg_freq,
  reg_channels,
  reg_samples,
  reg_sbuf_size,
  reg_init,
  reg_count,
  reg_present,
  nr_reg
};


static void update_count();

// 生成1KHz正弦波的简单函数
// 参数:
//   buffer: 存储生成数据的缓冲区
//   samples: 要生成的样本数量
void generate_1k_sine_wave(int16_t *buffer, int samples) {
  static uint32_t phase = 0;
  const int frequency = 1000;  // 1KHz
  const int sample_rate = 44100;  // 44.1KHz采样率
  const int16_t amplitude = 20000;  // 固定振幅
  
  for (int i = 0; i < samples; i++) {
    double angle = 2.0 * M_PI * phase * frequency / sample_rate;
    buffer[i] = (int16_t)(amplitude * sin(angle));
    phase++;
  }
  
  // 防止phase溢出
  phase %= sample_rate;
}


static uint8_t *sbuf = NULL;
static uint32_t *audio_base = NULL;

SDL_AudioCallback sdl_callback;
SDL_AudioSpec s = {};
void SDLCALL audio_callback(void* userdata, Uint8* stream, int len) {
  // generate_1k_sine_wave((int16_t *)stream, len / 2); // 生成1KHz正弦波(测试用)
  int16_t *sdl_buf = (int16_t *)stream;
  uint32_t count = audio_base[reg_count];

  uint16_t *sbuf_headptr = (uint16_t *)sbuf;
  uint16_t head_index = sbuf_headptr[0];
  uint16_t tail_index = sbuf_headptr[1];

  // Log("Head_index: %d, Tail_index: %d\n", head_index, tail_index);

  if(len <= count) {
    for(int i = 0; i < len / 2; i ++) { // len以字节为单位，data以int16_t为单位
    //  Log("head_index: %d, Tail_index: %d, i: %d, count: %d\n", head_index, tail_index, i, count);
      Assert(tail_index != head_index, "Audio Buffer Underflow!");
      sdl_buf[i] = sbuf_headptr[tail_index];
      // if(sbuf_headptr[tail_index]){ Log("Read audio data: %d from sbuf index: %d\n", sbuf_headptr[tail_index], tail_index);}
      tail_index = (tail_index + 1U) % (CONFIG_SB_SIZE / 2U);
      if(tail_index == 0) tail_index = 2; // 跳过头尾指针位置
    }
    sbuf_headptr[1] = tail_index;
    // Log("Tail_index after callback: %d\n", tail_index);
  } else {
    int i = 0;
    for(; i < count / 2; i ++) { // count以字节为单位，data以int16_t为单位
    //  Log("head_index: %d, Tail_index: %d, i: %d, count: %d\n", head_index, tail_index, i, count);
      Assert(tail_index != head_index, "Audio Buffer Underflow!");
      sdl_buf[i] = sbuf_headptr[tail_index];
      // if(sbuf_headptr[tail_index]) { Log("Read audio data: %d from sbuf index: %d\n", sbuf_headptr[tail_index], tail_index);}
      tail_index = (tail_index + 1U) % (CONFIG_SB_SIZE / 2U);
      if(tail_index == 0) tail_index = 2; // 跳过头尾指针位置
    }
    while(i < len / 2) {
      sdl_buf[i++] = 0; // 填充静音数据
    }
    sbuf_headptr[1] = tail_index;
    // Log("Tail_index after callback: %d\n", tail_index);
  }
  // 更新count寄存器
  update_count();
  // Log("Audio callback: %d bytes\n", len);
}

static void audio_io_handler(uint32_t offset, int len, bool is_write) {
  if(is_write && offset == 16) { // 写入init寄存器时
    s.format = AUDIO_S16SYS;  // 假设系统中音频数据的格式总是使用16位有符号数来表示
    s.userdata = NULL;        // 不使用
    s.freq = audio_base[reg_freq];
    s.channels = audio_base[reg_channels];
    s.samples = audio_base[reg_samples];
    s.callback = audio_callback;
 
    SDL_InitSubSystem(SDL_INIT_AUDIO);
    Assert(SDL_OpenAudio(&s, NULL) == 0, "SDL_InitSubSystem failed");
    SDL_PauseAudio(0);
  }

  // 更新count寄存器
  update_count();
}

void init_audio() {
  uint32_t space_size = sizeof(uint32_t) * nr_reg;
  audio_base = (uint32_t *)new_space(space_size);
#ifdef CONFIG_HAS_PORT_IO
  add_pio_map ("audio", CONFIG_AUDIO_CTL_PORT, audio_base, space_size, audio_io_handler);
#else
  add_mmio_map("audio", CONFIG_AUDIO_CTL_MMIO, audio_base, space_size, audio_io_handler);
#endif

  sbuf = (uint8_t *)new_space(CONFIG_SB_SIZE); // 计算空间基于1字节为单位，数据是int16_t类型（2字节）
  add_mmio_map("audio-sbuf", CONFIG_SB_ADDR, sbuf, CONFIG_SB_SIZE, NULL);

  uint16_t *sbuf_headptr = (uint16_t *)sbuf;
  sbuf_headptr[0] = sbuf_headptr[1] = 2; // 初始化头尾指针（占用前两个索引，使用uint16_t类型）

  audio_base[reg_sbuf_size] = CONFIG_SB_SIZE;
  audio_base[reg_present] = 1; // 寄存器存在标志位
  audio_base[reg_count] = 0;
  printf("Audio Init\n");
}

static void update_count() {
  // 更新count寄存器
  uint16_t *sbuf_headptr = (uint16_t *)sbuf;
  uint16_t head = sbuf_headptr[0];
  uint16_t tail = sbuf_headptr[1];
  if(head >= tail) {
    audio_base[reg_count] = (head - tail) * 2;
  } else { // 头指针在尾指针之后（环形缓冲区回绕）
    audio_base[reg_count] = ((CONFIG_SB_SIZE / 2) - (tail - head) - 2) * 2;
  }
}
