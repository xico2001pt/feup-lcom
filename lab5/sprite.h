#ifndef _SPRITE_H_
#define _SPRITE_H_
#include <lcom/lcf.h>

typedef struct {
  int x, y; // current position
  int width, height;  // dimensions
  int xspeed, yspeed; // current speed
  uint8_t *map;          // the pixmap
  } Sprite;

Sprite * create_sprite(xpm_map_t map, int x, int y, int xspeed, int yspeed);
int animate_sprite(Sprite *sp);
void destroy_sprite(Sprite *sp);
#endif //_SPRITE_H_
