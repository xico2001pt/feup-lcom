#include "sprite.h"
#include <video_gr.h>


Sprite * create_sprite(xpm_map_t map, int x, int y, int xspeed, int yspeed) {
  Sprite *sp = (Sprite*) malloc( sizeof(Sprite));
  xpm_image_t img;
  if( sp == NULL )
    return NULL;
  // read the sprite pixmap
  sp->map = xpm_load(map, XPM_INDEXED, &img);
  if( sp->map == NULL ) {
    free(sp);
    return NULL;
  }
  sp->width = img.width;
  sp->height = img.height;
  sp->x = x;
  sp->y = y;
  sp->xspeed = xspeed;
  sp->yspeed = yspeed;
  vg_draw_sprite(sp);
  vg_show_buffer();
  return sp;
}

int animate_sprite(Sprite *sp) {
  vg_clear_sprite(sp);
  sp->x += sp->xspeed;
  sp->y += sp->yspeed;
  vg_draw_sprite(sp);
  return 0;
}

void destroy_sprite(Sprite *sp) {
  if( sp == NULL )
    return;
  if( sp ->map )
    free(sp->map);
  free(sp);
  sp = NULL;
}

