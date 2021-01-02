
#include "doom/r_local.h"
#include "doom/r_draw.h"
#include "z_zone.h"
#include "rt.h"

rt_core_t rt_core;

void RT_SetupCore (rt_core_t *core, int w, int h, int height, angle_t aw, angle_t ah)
{
    rt_ray_t *rays = Z_Malloc(w * h * sizeof(rt_ray_t), PU_STATIC, NULL);

    core->rays = rays;
    core->w = w;
    core->h = h;
    core->ah = ah;
    core->aw = aw;

    printf("\n%s() : w=%d, h=%d, aw=%d, ah=%d, memory=%d bytes\n",
        __func__, w, h, aw, ah, w * h * sizeof(rt_ray_t));
}

void RT_Generate (rt_core_t *core, view_t *view)
{
    angle_t aw_step = core->aw / core->w, ah_step = core->ah / core->h;
    rt_ray_t *rays  =core->rays;
    rt_ray_t *ray;
    int x, y;
    angle_t aw_start = view->ax - core->aw / 2;
    angle_t ah_start = view->az - core->ah / 2;

    core->view = view;

    printf("%s() ++++\n", __func__);

    for (x = 0; x < core->w; x++) {
        ah_start = view->az - core->ah / 2;
        for (y = 0; y < core->h; y++) {
            ray = &rays[x + y * core->w];
            ray->screen_x = x;
            ray->screen_y = y;
            ray->ax = aw_start;
            ray->az = ah_start;
            ray->orig.x = view->x;
            ray->orig.y = view->y;
            ray->orig.z = view->z;

            ray->dir.x = rt_mul(RT_ONE, rt_sin(ray->ax));
            ray->dir.y = rt_mul(RT_ONE, rt_cos(ray->ax));
            ray->dir.z = rt_mul(RT_ONE, rt_cos(ray->az));
            ray->pixel = 0;
            ah_start += ah_step;
        }
        aw_start += aw_step;
    }

    printf("%s() ----\n", __func__);
}


static int _IntersectW (rt_ray_t *ray, seg_vis_t *seg)
{
    int x1_is_on_left = seg->ax1 < ray->ax;
    int x2_is_on_left = seg->ax2 < ray->ax;

    return x1_is_on_left ^ x2_is_on_left;
}

static int _IntersectH (rt_ray_t *ray, seg_vis_t *seg)
{
    int x1_is_on_top = seg->ay1 < ray->az;
    int x2_is_on_top = seg->ay2 < ray->az;

    return 1 || (x1_is_on_top ^ x2_is_on_top);
}

static void __cross (vertex3_t *dest, vertex3_t *v1, vertex3_t *v2)
{
    dest->x = rt_mul(v1->y, v2->z) - rt_mul(v1->z,  v2->y);
    dest->y = rt_mul(v1->z, v2->x) - rt_mul(v1->x, v2->z);
    dest->z = rt_mul(v1->x, v2->y) - rt_mul(v1->y, v2->x);
}

static rt_type_t __dot (vertex3_t *v1, vertex3_t *v2)
{
    return rt_mul(v1->x, v2->x) + rt_mul(v1->y, v2->y) + rt_mul(v1->z, v2->z);
}

static void __sub (vertex3_t *dest, vertex3_t *v1, vertex3_t *v2)
{
    dest->x = v1->x - v2->x;
    dest->y = v1->y - v2->y;
    dest->z = v1->z - v2->z;
}

static boolean _IntersectPoly (vertex3_t *orig, vertex3_t *dir, poly3_t *poly, rt_type_t *t, rt_type_t *u, rt_type_t *v)
{
    vertex3_t edge1, edge2, tvec, pvec, qvec;
    rt_type_t det, inv_det;

    __sub(&edge1, poly->v2, poly->v1);
    __sub(&edge2, poly->v3, poly->v1);

    __cross(&pvec, dir, &edge2);

    det = __dot(&edge1, &pvec);
    if (det < RT_EPSILON) {
        return false;
    }

    __sub(&tvec, orig, poly->v1);

    *u = __dot(&tvec, &pvec);

    if (*u < 0 || *u > det) {
        return false;
    }

    __cross(&qvec, &tvec, &edge1);

    *v = __dot(dir, &qvec);
    if (*v < 0 || *v + *u > det) {
        return false;
    }

    *t = __dot(&edge2, &qvec);
    inv_det = rt_div(RT_ONE, det);
    *t = rt_mul(*t, inv_det);
    *u = rt_mul(*u, inv_det);
    *v = rt_mul(*v, inv_det);
    return true;
}

static void __RT_PreTrace (rt_core_t *core, rt_ray_t *ray, seg_vis_t *segs, int seg_count,
        rt_pix_t (*mapper) (seg_vis_t *seg, poly3_t *poly, int u, int v))
{
    seg_vis_t *near_seg = NULL, *seg;
    poly3_t *poly = NULL;
    rt_type_t dist = 0, nu, nv;
    int count;

    for (count = 0; count < seg_count; count++) {
        seg = &segs[count];

        if (seg->poly_cnt == 0) {
            continue;
        }

        if (_IntersectW(ray, seg) && _IntersectH(ray, seg)) {
            rt_type_t t, u, v;

            printf("%s() : v1.x=%d, v1.y=%d, v2.x=%d, v2.y=%d\n"
                    "ray: orig %d %d %d, dir %d %d %d\n",
                __func__, seg->seg->v1->x>>FRACBITS, seg->seg->v1->y>>FRACBITS, seg->seg->v2->x>>FRACBITS, seg->seg->v2->y>>FRACBITS,
                ray->orig.x>>FRACBITS, ray->orig.y>>FRACBITS, ray->orig.z>>FRACBITS, ray->dir.x>>FRACBITS, ray->dir.x>>FRACBITS, ray->dir.y>>FRACBITS);

            printf("poly: v1 %d %d %d, v2 %d %d %d, v3 %d %d %d\n",
                seg->poly[0].v1->x>>FRACBITS, seg->poly[0].v1->y>>FRACBITS, seg->poly[0].v1->z>>FRACBITS,
                seg->poly[0].v2->x>>FRACBITS, seg->poly[0].v2->y>>FRACBITS, seg->poly[0].v2->z>>FRACBITS,
                seg->poly[0].v3->x>>FRACBITS, seg->poly[0].v3->y>>FRACBITS, seg->poly[0].v3->z>>FRACBITS);

            int i;

            for (i = 0; i < seg->poly_cnt; i++) {
                if (_IntersectPoly(&ray->orig, &ray->dir, &seg->poly[i], &t, &u, &v)) {

                    printf("%s() : t=%d, u=%d, v=%d, dist=%d\n", __func__, t, u, v, dist);
                    if (!dist || t < dist) {
                        dist = t;
                        nu = u;
                        nv = v;
                        near_seg = seg;
                        poly = &seg->poly[i];
                    }
                }
            }
        }
    }
    if (near_seg && poly) {
        ray->pixel = mapper(near_seg, poly, nu, nv);
    }
}


void RT_PreTrace (rt_core_t *core, seg_vis_t *segs, int seg_count,
        rt_pix_t (*mapper) (seg_vis_t *seg, poly3_t *poly, int u, int v))
{
    int x, y;
    rt_ray_t *ray;

    printf("%s() ++++\n", __func__);

    for (x = 0; x < core->w; x++) {
        for (y = 0; y < core->h; y++) {
            ray = &core->rays[x + y *core->w];
            __RT_PreTrace(core, ray, segs, seg_count, mapper);
        }
    }

    printf("%s() ----\n", __func__);
}

void RT_Render (rt_core_t *core, pix_t *buf, int w, int h)
{
    int x, y;
    rt_ray_t *ray;

    printf("%s() ++++\n", __func__);

    if (w != core->w) {
        printf("%s() : w(%d) != core->w(%d)\n", __func__, w, core->w);
        return;
    }

    for (x = 0; x < core->w; x++) {
        for (y = 0; y < core->h; y++) {
            ray = &core->rays[x + y *core->w];
            buf[ray->screen_x + ray->screen_y * w] = ray->pixel;
        }
    }

    printf("%s() ----\n", __func__);
}

