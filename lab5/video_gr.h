#ifndef __VIDEO_GR__
#define __VIDEO_GR__
#include "utils.h"
#include "sprite.h"
#include <machine/int86.h>

int vg_draw_xpm(xpm_map_t xpm, uint16_t x, uint16_t y);

uint8_t * vg_read_xpm(xpm_map_t xpm, xpm_image_t *img);

int vg_change_pixel(uint16_t x, uint16_t y, uint32_t color);

int (vg_draw_pattern)(uint16_t mode, uint8_t no_rectangles, uint32_t first, uint8_t step);

int vg_draw_sprite(Sprite * sprite);

int vg_clear_sprite(Sprite * sprite);

void vg_show_buffer();

int (vg_draw_char)();

void vg_clear_buffer();

#endif //__VBE__
