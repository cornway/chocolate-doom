#ifndef _R_VIEW_H_
#define _R_VIEW_H_

#include "m_fixed.h"
#include "tables.h"

struct player_s;

typedef struct view_s {
    fixed_t x, y, z;
    angle_t ax, az;
    fixed_t axsin, axcos;
    struct player_s *player;
} view_t;

extern view_t view;

#endif /* _R_VIEW_H_ */

