#include <lcom/lcf.h>
#include <minix/sysutil.h>
#include <machine/int86.h>
#include "video_gr.h"
#include "vbe.h"
#include "sprite.h"

static void *video_mem;		/* Process (virtual) address to which VRAM is mapped */
static void *video_buffer;
static unsigned h_res;	        /* Horizontal resolution in pixels */
static unsigned v_res;	        /* Vertical resolution in pixels */
static unsigned bits_per_pixel; /* Number of VRAM bits per pixel */
static unsigned bytes_per_pixel;
static enum xpm_image_type img_type;
static unsigned phys_base_ptr;

void * (vg_init)(uint16_t mode){
  // Fill info
  vbe_mode_info_t vmi_p;
  vbe_mode_get_info(mode, &vmi_p);
  switch(mode){
    case MODE_1024x768_INDEX:img_type=XPM_INDEXED; break;
    case MODE_640x480_DIRECT:img_type=XPM_1_5_5_5; break;
    case MODE_800x600_DIRECT:img_type=XPM_8_8_8; break;
    case MODE_1280x1024_DIRECT:img_type=XPM_5_6_5; break;
    case MODE_1152x864_DIRECT:img_type=XPM_8_8_8_8; break;
    default: img_type=INVALID_XPM;
  }
  h_res = vmi_p.XResolution;
  v_res = vmi_p.YResolution;
  bits_per_pixel = vmi_p.BitsPerPixel;
  bytes_per_pixel = bits_per_pixel != 15 ? bits_per_pixel / 8 : 2;
  phys_base_ptr = vmi_p.PhysBasePtr;
  printf("h_res: %d   v_res: %d   bits_per_pixel: %d\n", h_res, v_res, bits_per_pixel);

  unsigned int vram_size = h_res * v_res * bytes_per_pixel;
  video_buffer = (uint8_t *) malloc(vram_size);
  memset(video_buffer, 0, vram_size);

  video_mem = vbe_map_vram(phys_base_ptr, vram_size);
  if(video_mem == NULL){
    printf("Failed to map vram\n");
    return NULL;
  }

  if(vbe_set_mode(mode)){
    printf("vbe_set_mode failed\n");
    return NULL;
  }
  
  return video_mem;
}

int (vg_draw_hline)(uint16_t x, uint16_t y, uint16_t len, uint32_t color){
  uint8_t *ptr;
  ptr = video_buffer;
  ptr += (y*h_res + x) * bytes_per_pixel;

  for(int i = 0; i < len; ++i){
    memcpy(ptr, &color, bytes_per_pixel);
    ptr += bytes_per_pixel;
  }
  return 0;
}


int (vg_draw_char)(){
  struct reg86 r;
  memset(&r, 0, sizeof(r));
  r.intno = BIOS_VID_CARD;
  r.ax = 0x1130;
  r.bh = 0x06;
  if( (sys_int86)(&r) != OK ) {
    printf("\tsys_int86() failed \n");
    return 1;
  }
  uint32_t phys_addr = r.es * 0x10 + r.bp;
  uint16_t table_size = 256*r.cx;

  uint8_t* font_bit_buf = (uint8_t *) malloc(table_size);
  sys_readbios(phys_addr, font_bit_buf, table_size);
  void* font_pixel_buf = (void *) malloc(table_size * bytes_per_pixel);

  uint8_t background_color = 0x0;
  uint8_t foreground_color = 0x11;

  uint8_t * ptr = (uint8_t *) font_pixel_buf;
  // iterate chars
  for(int i = 0; i < 256; ++i){
    // iterate rows
    for(int j = 0; j < 16; ++j){
      uint8_t byte = *font_bit_buf;
      // iterate bits
      for(int k = 7; k >= 0; --k){
        *ptr = (byte & BIT(k)) ? foreground_color : background_color;
        ptr += bytes_per_pixel;
      }
      font_bit_buf++;
    }
  }
  free(font_bit_buf);

  uint16_t x = 50, y = 50;
  ptr = font_pixel_buf;
  ptr += 'a'*16*8*bytes_per_pixel;
  uint8_t * base_buffer = video_buffer;
  for(int i = 0; i < 16; ++i){
    uint8_t * buffer = base_buffer + (y+i)*h_res + x;
    memcpy(buffer, ptr, bytes_per_pixel*8);
    ptr += bytes_per_pixel*8;
  }

  return 0;
}

void vg_show_buffer(){
  memcpy(video_mem, video_buffer, h_res*v_res*bytes_per_pixel);
}

void vg_clear_buffer(){
  memset(video_buffer, 0, h_res*v_res*bytes_per_pixel);
}

int (vg_draw_rectangle)(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint32_t color){
  for(int i = 0; i < height; ++i){
    vg_draw_hline(x, y+i, width, color);
  }
  return 0;
}

uint8_t * vg_read_xpm(xpm_map_t xpm, xpm_image_t *img){
  uint8_t *map;
  map = xpm_load(xpm, img_type, img);
  if(map == NULL){
    printf("Couldn't read xpm\n");
    return NULL;
  }
  return map;
}

int vg_draw_xpm(xpm_map_t xpm, uint16_t x, uint16_t y){
  xpm_image_t img;
  uint8_t *map = vg_read_xpm(xpm, &img);
  if(map == NULL){
    return 1;
  }

  uint8_t *line_ptr = video_buffer;
  line_ptr += y*bytes_per_pixel*h_res; 
  for(int i = 0; i < img.height; ++i){
    uint8_t *buffer_ptr = line_ptr + x;
    memcpy(buffer_ptr, map, bytes_per_pixel*img.width);
    map += bytes_per_pixel*img.width;
    line_ptr += bytes_per_pixel*h_res;
  }
  return 0;
}

int vg_draw_sprite(Sprite * sprite){
  uint8_t *map = sprite->map;
  uint8_t *line_ptr = video_buffer;
  line_ptr += sprite->y*bytes_per_pixel*h_res; 
  for(int i = 0; i < sprite->height; ++i){
    uint8_t *buffer_ptr = line_ptr + sprite->x;
    memcpy(buffer_ptr, map, bytes_per_pixel*sprite->width);
    map += bytes_per_pixel*sprite->width;
    line_ptr += bytes_per_pixel*h_res;
  }
  return 0;
}

int vg_clear_sprite(Sprite * sprite){
  return vg_draw_rectangle(sprite->x,sprite->y,sprite->width,sprite->height,0);
}

int (vg_draw_pattern)(uint16_t mode, uint8_t no_rectangles, uint32_t first, uint8_t step) {
  unsigned int width = h_res / no_rectangles, height = v_res / no_rectangles;
  uint32_t color;
  for(unsigned int col = 0; col < no_rectangles; col++) {
    for(unsigned int row = 0; row < no_rectangles; row++) {
      if (mode == MODE_1024x768_INDEX) { // Indexed Mode
        color = (first + (row * no_rectangles + col) * step) % (1 << bits_per_pixel);
      }
      else { // Direct Mode
        vbe_mode_info_t vmi_p;
        vbe_get_mode_info(mode, &vmi_p);
        uint32_t R = (((first & ((1 << vmi_p.RedMaskSize) - 1) << vmi_p.RedFieldPosition) >> vmi_p.RedFieldPosition) + col * step) % (1 << vmi_p.RedMaskSize);
        uint32_t G = (((first & ((1 << vmi_p.GreenMaskSize) - 1) << vmi_p.GreenFieldPosition) >> vmi_p.GreenFieldPosition) + row * step) % (1 << vmi_p.GreenMaskSize);
        uint32_t B = (((first & ((1 << vmi_p.BlueMaskSize) - 1) << vmi_p.BlueFieldPosition) >> vmi_p.BlueFieldPosition) + (col + row) * step) % ( 1 << vmi_p.BlueMaskSize);

        color = (R << vmi_p.RedFieldPosition) | (G << vmi_p.GreenFieldPosition) | B;
        printf("R:0x%x G:0x%x B:0x%x      color: 0x%x\n", R, G, B, color);
      }
      vg_draw_rectangle(col * width, row * height, width, height, color);
    }
  }
  return 0;
}

int vg_change_pixel(uint16_t x, uint16_t y, uint32_t color){
  uint8_t * mem = video_buffer;
  mem += (y * h_res + x) * bytes_per_pixel;
  memcpy(mem, &color, bytes_per_pixel);
  return 0;
}
