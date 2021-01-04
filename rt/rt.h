#ifndef _RT_H_
#define _RT_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "r_view.h"

typedef unsigned int rt_pix_t;

/* Ray tracing  core parameters */
typedef struct rt_core_s {
    int w, h;
    int d;
    float fov;
    view_t *view;
    void *rays;
} rt_core_t;

extern void _RT_SetupCore (rt_core_t *core, int w, int h, int d, angle_t fov, void *(*_malloc) (unsigned));
extern void _RT_Generate (rt_core_t *core, view_t *view);
extern void _RT_PreTrace (rt_core_t *core, seg_vis_t *segs, int seg_count, rt_pix_t (*mapper) (seg_vis_t *seg, poly3_t *poly, int u, int v));
extern void _RT_Render (rt_core_t *core, pix_t *buf, int w, int h);

#ifdef __cplusplus
}
#endif

#endif /* _RT_H_ */

