#include <am.h>
#include <nemu.h>


#define AUDIO_FREQ_ADDR      (AUDIO_ADDR + 0x00)
#define AUDIO_CHANNELS_ADDR  (AUDIO_ADDR + 0x04)
#define AUDIO_SAMPLES_ADDR   (AUDIO_ADDR + 0x08)
#define AUDIO_SBUF_SIZE_ADDR (AUDIO_ADDR + 0x0c)
#define AUDIO_INIT_ADDR      (AUDIO_ADDR + 0x10)
#define AUDIO_COUNT_ADDR     (AUDIO_ADDR + 0x14)
#define ADUIO_PRESENT_ADDR   (AUDIO_ADDR + 0x18)

#define BUF_HEAD_ADDR        (AUDIO_SBUF_ADDR + 0x00)
#define BUF_TAIL_ADDR        (AUDIO_SBUF_ADDR + 0x02)

void __am_audio_init() {
}

void __am_audio_config(AM_AUDIO_CONFIG_T *cfg) {
  cfg->present = inl(ADUIO_PRESENT_ADDR);
  cfg->bufsize = inl(AUDIO_SBUF_SIZE_ADDR);
}

void __am_audio_ctrl(AM_AUDIO_CTRL_T *ctrl) {
  outl(AUDIO_CHANNELS_ADDR, ctrl->channels);
  outl(AUDIO_SAMPLES_ADDR, ctrl->samples);
  outl(AUDIO_FREQ_ADDR, ctrl->freq);
  outl(AUDIO_INIT_ADDR, 1);
}

void __am_audio_status(AM_AUDIO_STATUS_T *stat) {
  stat->count = inl(AUDIO_COUNT_ADDR);
}

void __am_audio_play(AM_AUDIO_PLAY_T *ctl) {
  int use_size = inl(AUDIO_COUNT_ADDR);
  int bf_size = inl(AUDIO_SBUF_SIZE_ADDR);
  int free_size = bf_size - use_size;
  int data_size = (uint8_t *)(ctl->buf.end) - (uint8_t *)(ctl->buf.start);
  while(free_size < data_size) {
    use_size =  inl(AUDIO_COUNT_ADDR);
    free_size = bf_size - use_size;
  }
  uint16_t buf_ptr = inw(BUF_HEAD_ADDR);
  uint32_t buf_size = inl(AUDIO_SBUF_SIZE_ADDR);

  int16_t *data_ptr = (int16_t *)(ctl->buf.start);
  int16_t *data_end = (int16_t *)(ctl->buf.end); // data不包括data_end指向的内容
  while (data_ptr < data_end) {
    outw(AUDIO_SBUF_ADDR + buf_ptr * 2, *data_ptr);
    data_ptr++;

    buf_ptr = (buf_ptr + 1) % (buf_size / 2); // 跳过头尾指针位置（但注意指针为16位 -> 最大索引为0xFFFF）
    if(buf_ptr == 0) buf_ptr = 2;
  }
  outw(BUF_HEAD_ADDR, buf_ptr);
  
  return;
}
