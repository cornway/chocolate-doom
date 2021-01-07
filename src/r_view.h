#ifndef _R_VIEW_H_
#define _R_VIEW_H_

#include "m_fixed.h"
#include "tables.h"
#include "rt_if.h"


struct player_s;

typedef struct view_s {
    vertex3_t orig;
    Vertex3f_t origf;
    angle_t ax, az;
    float axsinf, axcosf;
    fixed_t axsin, axcos;
    struct player_s *player;
} view_t;

extern view_t view;

#endif /* _R_VIEW_H_ */

