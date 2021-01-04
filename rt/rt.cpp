
#include "doom/r_defs.h"
#include "r_view.h"
#include "rt_if.h"
#include "poly.h"

int g_use_rt = 1;

rt_core_t rt_core;

typedef struct rt_ray_s {
    int x, y;
    Vec3 dir;
    vertex3_t *orig;
    rt_pix_t pixel;
} rt_ray_t;

void _RT_SetupCore (rt_core_t *core, int w, int h, int d, angle_t fov, void *(*_malloc) (unsigned))
{
    rt_ray_t *rays;
    if (!g_use_rt) {
        return;
    }

    rays = (rt_ray_t *)malloc(w * h * sizeof(rt_ray_t));

    core->rays = rays;
    core->w = w;
    core->h = h;
    core->d = d;

    printf("\n%s() : w=%d, h=%d, fov=%d, memory=%d bytes\n",
        __func__, w, h, fov * h * sizeof(rt_ray_t));
}

void _RT_Generate (rt_core_t *core, view_t *view)
{
    rt_ray_t *rays  = (rt_ray_t *)core->rays;
    rt_ray_t *ray;
    int x, y;
    float scale, ratio, xx, yy, zz;

    if (!g_use_rt) {
        return;
    }

    core->view = view;
    scale = 1.0f;
    ratio = (float)core->w / (float)core->h;

    printf("%s() ++++\n", __func__);

    for (x = 0; x < core->w; x++) {
        for (y = 0; y < core->h; y++) {
            ray = &rays[x + y * core->w];
            ray->x = x;
            ray->y = y;
            ray->orig = &view->orig;

            xx = (2 * (x + 0.5) / (float)core->w - 1) * ratio * scale;
            yy = (1 - 2 * (y + 0.5) / (float)core->h) * scale;
            zz= 1.0f;

            ray->dir.Init(xx, yy, zz);
            ray->dir.Normalize();

            ray->pixel = 0;
        }
    }

    printf("%s() ----\n", __func__);
}

static void __RT_PreTrace (rt_core_t *core, rt_ray_t *ray, seg_vis_t *segs, int seg_count,
        rt_pix_t (*mapper) (seg_vis_t *seg, poly3_t *poly, int u, int v))
{
    seg_vis_t *near_seg = NULL, *seg;
    poly3_t *poly = NULL;
    float dist = 0, nu, nv;
    int count, i;
    float t, u, v;

    for (count = 0; count < seg_count; count++) {
        seg = &segs[count];

        if (seg->poly_cnt == 0) {
            continue;
        }

        for (i = 0; i < seg->poly_cnt; i++) {
            poly = &seg->poly[i];
            Poly3 poly3(poly->v1, poly->v2, poly->v3);
            Vec3 orig(ray->orig);
            if (poly3.Intersect(orig, ray->dir, &u, &v, &t)) {

                if (!dist || t < dist) {
                    dist = t;
                    nu = u;
                    nv = v;
                    near_seg = seg;
                }
            }
        }
    }
    if (near_seg && poly) {
        ray->pixel = mapper(near_seg, poly, nu, nv);
    }
}


void _RT_PreTrace (rt_core_t *core, seg_vis_t *segs, int seg_count,
        rt_pix_t (*mapper) (seg_vis_t *seg, poly3_t *poly, int u, int v))
{
    int x, y;
    rt_ray_t *ray;
    rt_ray_t *rays = (rt_ray_t *)core->rays;

    if (!g_use_rt) {
        return;
    }

    printf("%s() ++++\n", __func__);

    for (x = 0; x < core->w; x++) {
        for (y = 0; y < core->h; y++) {
            ray = &rays[x + y *core->w];
            __RT_PreTrace(core, ray, segs, seg_count, mapper);
        }
    }

    printf("%s() ----\n", __func__);
}

void _RT_Render (rt_core_t *core, pix_t *buf, int w, int h)
{
    int x, y;
    rt_ray_t *ray;
    rt_ray_t *rays = (rt_ray_t *)core->rays;

    if (!g_use_rt) {
        return;
    }

    printf("%s() ++++\n", __func__);

    if (w != core->w) {
        printf("%s() : w(%d) != core->w(%d)\n", __func__, w, core->w);
        return;
    }

    for (x = 0; x < core->w; x++) {
        for (y = 0; y < core->h; y++) {
            ray = &rays[x + y *core->w];
            buf[ray->x + ray->y * w] = ray->pixel;
        }
    }

    printf("%s() ----\n", __func__);
}

