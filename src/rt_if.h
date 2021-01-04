#ifndef _RT_IF_H_
#define _RT_IF_H_

#include "rt/rt.h"

void RT_SetupCore (rt_core_t *core, int w, int h, int d, angle_t fov, void *(*_malloc) (unsigned));
void RT_Generate (rt_core_t *core, view_t *view);
void RT_PreTrace (rt_core_t *core, seg_vis_t *segs, int seg_count, rt_pix_t (*mapper) (seg_vis_t *seg, poly3_t *poly, int u, int v));
void RT_Render (rt_core_t *core, pix_t *buf, int w, int h);

extern rt_core_t rt_core;

#endif /* _RT_IF_H_ */

