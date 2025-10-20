#include <am.h>
#include <nemu.h>
#include <stdint.h>

#define SYNC_ADDR (VGACTL_ADDR + 4)

static int screen_w = 0, screen_h = 0;
void __am_gpu_init() {
  uint32_t screen_size = inl(VGACTL_ADDR);
  screen_w = screen_size >> 16;
  screen_h = screen_size & 0xFFFF; 
}

void __am_gpu_config(AM_GPU_CONFIG_T *cfg) {
  uint32_t screen_size = inl(VGACTL_ADDR);
  *cfg = (AM_GPU_CONFIG_T) {
    .present = true, .has_accel = false,
    .width = screen_size >> 16, .height = screen_size & 0xFFFF,
    .vmemsz = (screen_size >> 16) * (screen_size & 0xFFFF) * sizeof(uint32_t)
  };
}

void __am_gpu_fbdraw(AM_GPU_FBDRAW_T *ctl) {
  if(screen_w == 0 || screen_h == 0) {
    uint32_t screen_size = inl(VGACTL_ADDR);
    screen_w = screen_size >> 16;
    screen_h = screen_size & 0xFFFF; 
  }

  uint32_t *fb = (uint32_t *)(uintptr_t)FB_ADDR;
  int begin_x = ctl->x, begin_y = ctl->y;
  int draw_w = ctl->w, draw_h = ctl->h;
  for(int i = begin_y; i < begin_y + draw_h; i++) {
    for(int j = begin_x; j < begin_x + draw_w; j++) {
      if(i >= 0 && i < screen_h && j >= 0 && j < screen_w) {
        fb[i * screen_w + j] = *((uint32_t *)ctl->pixels + (i - begin_y) * draw_w + (j - begin_x));
      }
    }
  }
  if (ctl->sync) {
    outl(SYNC_ADDR, 1);
  }
}

void __am_gpu_status(AM_GPU_STATUS_T *status) {
  status->ready = true;
}
