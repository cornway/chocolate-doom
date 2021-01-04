
#include "doom/r_defs.h"
#include "rt_if.h"

void RT_SetupCore (rt_core_t *core, int w, int h, int d, angle_t fov, void *(*_malloc) (unsigned))
{
    _RT_SetupCore(core, w, h, d, fov, _malloc);
}

void RT_Render (rt_core_t *core, pix_t *buf, int w, int h)
{
    _RT_Render(core, buf, w, h);
}

void RT_PreTrace (rt_core_t *core, seg_vis_t *segs, int seg_count, rt_pix_t (*mapper) (seg_vis_t *seg, poly3_t *poly, int u, int v))
{
    _RT_PreTrace (core, segs, seg_count, mapper);
}

void RT_Generate (rt_core_t *core, view_t *view)
{
    _RT_Generate(core, view);
}

