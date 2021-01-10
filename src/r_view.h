#ifndef _R_VIEW_H_
#define _R_VIEW_H_

#include "m_fixed.h"
#include "tables.h"
#include "rt_if.h"


struct player_s;

typedef struct view_s {
    vertex3_t orig;
    vertex3_t direction;
    angle_t ax, az;

    struct player_s *player;
} view_t;

static inline void VertTranslate2Dto3D (Vertex3f_t *out, Vertex3f_t *in)
{
    out->x = in->y;
    out->y = in->z;
    out->z = in->x;
}

extern view_t view;

#endif /* _R_VIEW_H_ */

