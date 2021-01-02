#ifndef _RT_H_
#define _RT_H_

struct rt_ray_s;

typedef unsigned int rt_pix_t;
typedef fixed_t rt_type_t;

#define RT_EPSILON (FRACUNIT >> FRACBITS)
#define RT_ONE (FRACUNIT)
#define RT_INF (0xffffffff)

#define rt_mul(a, b) FixedMul((fixed_t)a, (fixed_t)b)
#define rt_div(a, b) FixedDiv((fixed_t)a, (fixed_t)b)
#define rt_sin(a) finesine[(a)>>ANGLETOFINESHIFT]
#define rt_cos(a) finecosine[(a)>>ANGLETOFINESHIFT]
#define rt_tan(a) finetangent[(a)>>ANGLETOFINESHIFT]

/* Ray tracing  core parameters */
typedef struct rt_core_s {
    int w, h;
    int height;
    angle_t aw, ah;
    view_t *view;
    struct rt_ray_s *rays;
} rt_core_t;

typedef struct rt_ray_s {
    int screen_x, screen_y;
    angle_t ax, az;
    vertex3_t orig;
    vertex3_t dir;
    rt_pix_t pixel;
} rt_ray_t;

void RT_SetupCore (rt_core_t *core, int w, int h, int height, angle_t aw, angle_t ah);
void RT_Generate (rt_core_t *core, view_t *view);
void RT_PreTrace (rt_core_t *core, seg_vis_t *segs, int seg_count, rt_pix_t (*mapper) (seg_vis_t *seg, poly3_t *poly, int u, int v));

void RT_Render (rt_core_t *core, pix_t *buf, int w, int h);

extern rt_core_t rt_core;

#endif /* _RT_H_ */

