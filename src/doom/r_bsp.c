//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 2005-2014 Simon Howard
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// DESCRIPTION:
//	BSP traversal, handling of LineSegs for rendering.
//




#include "doomdef.h"

#include "m_bbox.h"

#include "i_system.h"

#include "r_defs.h"
#include "r_view.h"
#include "r_main.h"
#include "r_plane.h"
#include "r_things.h"
#include "r_draw.h"
#include "rt_if.h"
// State.
#include "doomstat.h"
#include "r_state.h"
//#include "r_local.h"

side_t*		sidedef;
line_t*		linedef;

drawseg_t	drawsegs[MAXDRAWSEGS];
drawseg_t*	ds_p;


void
R_StoreWallRange
( view_t *view,
  seg_vis_t *seg,
  int	start,
  int	stop );




//
// R_ClearDrawSegs
//
void R_ClearDrawSegs (void)
{
    ds_p = drawsegs;
}



//
// ClipWallSegment
// Clips the given range of columns
// and includes it in the new clip list.
//
typedef	struct
{
    int	first;
    int last;
    
} cliprange_t;

// We must expand MAXSEGS to the theoretical limit of the number of solidsegs
// that can be generated in a scene by the DOOM engine. This was determined by
// Lee Killough during BOOM development to be a function of the screensize.
// The simplest thing we can do, other than fix this bug, is to let the game
// render overage and then bomb out by detecting the overflow after the 
// fact. -haleyjd
//#define MAXSEGS 32
#define MAXSEGS (SCREENWIDTH / 2 + 1)

// newend is one past the last valid seg
cliprange_t*	newend;
cliprange_t	solidsegs[MAXSEGS];




//
// R_ClipSolidWallSegment
// Does handle solid walls,
//  e.g. single sided LineDefs (middle texture)
//  that entirely block the view.
// 
void
R_ClipSolidWallSegment
( view_t *view, seg_vis_t *seg )
{
    cliprange_t*	next;
    cliprange_t*	start;
    int first = seg->x1;
    int last = seg->x2;

    // Find the first range that touches the range
    //  (adjacent pixels are touching).
    start = solidsegs;
    while (start->last < first-1)
	start++;

    if (first < start->first)
    {
	if (last < start->first-1)
	{
	    // Post is entirely visible (above start),
	    //  so insert a new clippost.
	    R_StoreWallRange (view, seg, first, last);
	    next = newend;
	    newend++;
	    
	    while (next != start)
	    {
		*next = *(next-1);
		next--;
	    }
	    next->first = first;
	    next->last = last;
	    return;
	}
		
	// There is a fragment above *start.
	R_StoreWallRange (view, seg, first, start->first - 1);
	// Now adjust the clip size.
	start->first = first;	
    }

    // Bottom contained in start?
    if (last <= start->last)
	return;			
		
    next = start;
    while (last >= (next+1)->first-1)
    {
	// There is a fragment between two posts.
	R_StoreWallRange (view, seg, next->last + 1, (next+1)->first - 1);
	next++;
	
	if (last <= next->last)
	{
	    // Bottom is contained in next.
	    // Adjust the clip size.
	    start->last = next->last;	
	    goto crunch;
	}
    }
	
    // There is a fragment after *next.
    R_StoreWallRange (view, seg, next->last + 1, last);
    // Adjust the clip size.
    start->last = last;
	
    // Remove start+1 to next from the clip list,
    // because start now covers their area.
  crunch:
    if (next == start)
    {
	// Post just extended past the bottom of one post.
	return;
    }
    

    while (next++ != newend)
    {
	// Remove a post.
	*++start = *next;
    }

    newend = start+1;
}



//
// R_ClipPassWallSegment
// Clips the given range of columns,
//  but does not includes it in the clip list.
// Does handle windows,
//  e.g. LineDefs with upper and lower texture.
//
void
R_ClipPassWallSegment
( view_t *view, seg_vis_t *seg )
{
    cliprange_t*	start;
    int first = seg->x1;
    int last = seg->x2;

    // Find the first range that touches the range
    //  (adjacent pixels are touching).
    start = solidsegs;
    while (start->last < first-1)
	start++;

    if (first < start->first)
    {
	if (last < start->first-1)
	{
	    // Post is entirely visible (above start).
	    R_StoreWallRange (view, seg, first, last);
	    return;
	}
		
	// There is a fragment above *start.
	R_StoreWallRange (view, seg, first, start->first - 1);
    }

    // Bottom contained in start?
    if (last <= start->last)
	return;			
		
    while (last >= (start+1)->first-1)
    {
	// There is a fragment between two posts.
	R_StoreWallRange (view, seg, start->last + 1, (start+1)->first - 1);
	start++;
	
	if (last <= start->last)
	    return;
    }
	
    // There is a fragment after *next.
    R_StoreWallRange (view, seg, start->last + 1, last);
}



//
// R_ClearClipSegs
//
void R_ClearClipSegs (void)
{
    solidsegs[0].first = -0x7fffffff;
    solidsegs[0].last = -1;
    solidsegs[1].first = viewwidth;
    solidsegs[1].last = 0x7fffffff;
    newend = solidsegs+2;
}

//
// R_AddLine
// Clips the given segment
// and adds any visible pieces to the line list.
//
boolean R_AddLine (view_t *view, seg_vis_t *line)
{
    int     x1, x2;
    angle_t angle1, angle2;
    angle_t span, tspan;
    sector_t *frontsector, *backsector;

    // OPTIMIZE: quickly reject orthogonal back sides.
    angle1 = R_PointToAngle (view->orig.x, view->orig.y, line->seg->v1->x, line->seg->v1->y);
    angle2 = R_PointToAngle (view->orig.x, view->orig.y, line->seg->v2->x, line->seg->v2->y);
    
    // Clip to view edges.
    // OPTIMIZE: make constant out of 2*clipangle (FIELDOFVIEW).
    span = angle1 - angle2;
    
    // Back side? I.e. backface culling?
    if (span >= ANG180)
        return false;

    // Global angle needed by segcalc.
    line->ax1 = angle1;
    line->ax2 = angle2;
    line->ay1 = 0;
    line->ay2 = 0;
    angle1 -= view->ax;
    angle2 -= view->ax;

    tspan = angle1 + clipangle;
    if (tspan > 2*clipangle)
    {
        tspan -= 2*clipangle;

        // Totally off the left edge?
        if (tspan >= span)
            return false;

        angle1 = clipangle;
    }
    tspan = clipangle - angle2;
    if (tspan > 2*clipangle)
    {
        tspan -= 2*clipangle;

        // Totally off the left edge?
        if (tspan >= span)
            return false;
        angle2 = -clipangle;
    }
    
    // The seg is in the view range,
    // but not necessarily visible.
    angle1 = (angle1+ANG90)>>ANGLETOFINESHIFT;
    angle2 = (angle2+ANG90)>>ANGLETOFINESHIFT;
    x1 = viewangletox[angle1];
    x2 = viewangletox[angle2];

    // Does not cross a pixel?
    if (x1 == x2)
        return false;

    line->x1 = x1;
    line->x2 = x2-1;
    backsector = line->seg->backsector;
    frontsector = line->seg->frontsector;

    // Single sided line?
    if (!backsector)
        goto clipsolid;

    // Closed door.
    if (backsector->ceilingheight <= frontsector->floorheight || backsector->floorheight >= frontsector->ceilingheight)
        goto clipsolid;

    // Window.
    if (backsector->ceilingheight != frontsector->ceilingheight || backsector->floorheight != frontsector->floorheight)
        goto clippass;

    // Reject empty lines used for triggers
    //  and special events.
    // Identical floor and ceiling on both sides,
    // identical light levels on both sides,
    // and no middle texture.
    if (backsector->ceilingpic == frontsector->ceilingpic
    && backsector->floorpic == frontsector->floorpic
    && backsector->lightlevel == frontsector->lightlevel
    && line->seg->sidedef->midtexture == 0)
    {
        return false;
    }
    clippass:
    line->solid = false;
    return true;
    clipsolid:
    line->solid = true;
    return true;
}

void R_ProjectLine (view_t *view, seg_vis_t *seg)
{
    if (seg->solid) {
        R_ClipSolidWallSegment (view, seg);
     } else {
         R_ClipPassWallSegment (view, seg);
     }
}

//
// R_CheckBBox
// Checks BSP node/subtree bounding box.
// Returns true
//  if some part of the bbox might be visible.
//
int	checkcoord[12][4] =
{
    {3,0,2,1},
    {3,0,2,0},
    {3,1,2,0},
    {0},
    {2,0,2,1},
    {0,0,0,0},
    {3,1,3,0},
    {0},
    {2,0,3,1},
    {2,1,3,1},
    {2,1,3,0}
};


boolean R_CheckBBox (view_t *view, fixed_t*	bspcoord)
{
    int			boxx;
    int			boxy;
    int			boxpos;

    fixed_t		x1;
    fixed_t		y1;
    fixed_t		x2;
    fixed_t		y2;
    
    angle_t		angle1;
    angle_t		angle2;
    angle_t		span;
    angle_t		tspan;
    
    cliprange_t*	start;

    int			sx1;
    int			sx2;
    
    // Find the corners of the box
    // that define the edges from current viewpoint.
    if (view->orig.x <= bspcoord[BOXLEFT])
	boxx = 0;
    else if (view->orig.x < bspcoord[BOXRIGHT])
	boxx = 1;
    else
	boxx = 2;
		
    if (view->orig.y >= bspcoord[BOXTOP])
	boxy = 0;
    else if (view->orig.y > bspcoord[BOXBOTTOM])
	boxy = 1;
    else
	boxy = 2;
		
    boxpos = (boxy<<2)+boxx;
    if (boxpos == 5)
	return true;
	
    x1 = bspcoord[checkcoord[boxpos][0]];
    y1 = bspcoord[checkcoord[boxpos][1]];
    x2 = bspcoord[checkcoord[boxpos][2]];
    y2 = bspcoord[checkcoord[boxpos][3]];
    
    // check clip list for an open space
    angle1 = R_PointToAngle (view->orig.x, view->orig.y, x1, y1) - view->ax;
    angle2 = R_PointToAngle (view->orig.x, view->orig.y, x2, y2) - view->ax;
	
    span = angle1 - angle2;

    // Sitting on a line?
    if (span >= ANG180)
	return true;
    
    tspan = angle1 + clipangle;

    if (tspan > 2*clipangle)
    {
	tspan -= 2*clipangle;

	// Totally off the left edge?
	if (tspan >= span)
	    return false;	

	angle1 = clipangle;
    }
    tspan = clipangle - angle2;
    if (tspan > 2*clipangle)
    {
	tspan -= 2*clipangle;

	// Totally off the left edge?
	if (tspan >= span)
	    return false;
	
	angle2 = -clipangle;
    }


    // Find the first clippost
    //  that touches the source post
    //  (adjacent pixels are touching).
    angle1 = (angle1+ANG90)>>ANGLETOFINESHIFT;
    angle2 = (angle2+ANG90)>>ANGLETOFINESHIFT;
    sx1 = viewangletox[angle1];
    sx2 = viewangletox[angle2];

    // Does not cross a pixel.
    if (sx1 == sx2)
	return false;			
    sx2--;
	
    start = solidsegs;
    while (start->last < sx2)
	start++;
    
    if (sx1 >= start->first
	&& sx2 <= start->last)
    {
	// The clippost contains the new span.
	return false;
    }

    return true;
}



//
// R_Subsector
// Determine floor/ceiling planes.
// Add sprites of things in sector.
// Draw one or more line segments.
//
void R_Subsector (view_t *view, int num)
{
    int			count;
    seg_t*		line;
    seg_vis_t _vis;
    seg_vis_t *vis = &_vis;
    subsector_t*	sub;
    sector_t *frontsector;
    visplane_t *ceilingplane, *floorplane;

#ifdef RANGECHECK
    if (num>=numsubsectors)
        I_Error ("R_Subsector: ss %i with numss = %i", num, numsubsectors);
#endif
    sub = &subsectors[num];
    frontsector = sub->sector;
    count = sub->numlines;
    line = &segs[sub->firstline];

    if (frontsector->floorheight < view->orig.z)
    {
        floorplane = R_FindPlane (frontsector->floorheight,
        frontsector->floorpic,
        frontsector->lightlevel);
    }
    else
        floorplane = NULL;

    if (frontsector->ceilingheight > view->orig.z
        || frontsector->ceilingpic == skyflatnum)
    {
        ceilingplane = R_FindPlane (frontsector->ceilingheight, frontsector->ceilingpic, frontsector->lightlevel);
    }
    else
        ceilingplane = NULL;

    R_AddSprites (view, frontsector);

    while (count--)
    {
        vis->seg = line;
        if (R_AddLine (view, vis)) {
            vis->floorplane = floorplane;
            vis->ceilingplane = ceilingplane;
            segs_vis[sscount++] = *vis;
        }
        line++;
    }

    // check for solidsegs overflow - extremely unsatisfactory!
    if(newend > &solidsegs[32])
        I_Error("R_Subsector: solidsegs overflow (vanilla may crash here)\n");
}

static inline void R_PolyToTexCord (Poly3f_t *poly, const Vertex3f_t *v1, const Vertex3f_t *v2, const Vertex3f_t *v3)
{
    poly->t1.x = v1->x;
    poly->t1.y = v1->y;

    poly->t2.x = v2->x;
    poly->t2.y = v2->y;

    poly->t3.x = v3->x;
    poly->t3.y = v3->y;
}

static inline void R_FloorPolyToTexCord (Poly3f_t *poly)
{
    R_PolyToTexCord(poly, &poly->v1, &poly->v2, &poly->v3);
}

static int R_LineToPoly (seg_t *line, Vertex3f_t *v, Poly3f_t poly[2], float floor, float height, int texnum)
{
    float flength = line->linedef->length;
    float fheight = ToFloat(height);
    const Vertex3f_t vt[]= {{0, 0, 0}, {flength, 0, 0}, {0, fheight, 0}, {flength, fheight, 0}};
    const int cw_dir[] = {0, 1, 2, 2, 1, 3};
    const int *dir;

    if (!flength || !fheight) {
        return 0;
    }

    v[0].x = ToFloat(line->v1->x);
    v[0].y = ToFloat(line->v1->y);
    v[0].z = ToFloat(floor);

    v[1].x = ToFloat(line->v2->x);
    v[1].y = ToFloat(line->v2->y);
    v[1].z = ToFloat(floor);

    v[2].x = ToFloat(line->v1->x);
    v[2].y = ToFloat(line->v1->y);
    v[2].z = ToFloat(floor + height);

    v[3].x = ToFloat(line->v2->x);
    v[3].y = ToFloat(line->v2->y);
    v[3].z = ToFloat(floor + height);

    dir = cw_dir;

    poly[0].v1 = v[dir[0]];
    poly[0].v2 = v[dir[1]];
    poly[0].v3 = v[dir[2]];

    poly[1].v1 = v[dir[3]];
    poly[1].v2 = v[dir[4]];
    poly[1].v3 = v[dir[5]];

    R_PolyToTexCord(&poly[0], &vt[dir[0]], &vt[dir[1]], &vt[dir[2]]);
    R_PolyToTexCord(&poly[1], &vt[dir[3]], &vt[dir[4]], &vt[dir[5]]);

    poly[0].shader.texture = texnum;
    poly[0].shader.draw = R_DrawSolidPoly;

    poly[1].shader.texture = texnum;
    poly[1].shader.draw = R_DrawSolidPoly;

    return 2;
}

#define MAX_LINES (64)

typedef struct bbox_s {
    fixed_t x1, y1, x2, y2;
} bbox_t;

void bboxToVert (vertex3_t v[4], bbox_t *bbox)
{
    v[0].x = bbox->x1;
    v[0].y = bbox->y1;

    v[1].x = bbox->x2;
    v[1].y = bbox->y1;

    v[2].x = bbox->x2;
    v[2].y = bbox->y2;

    v[3].x = bbox->x1;
    v[3].y = bbox->y2;
}

static void linesCopy (Vertex3f_t *dest, line_t **src, int lines_count, bbox_t *bbox)
{
    int i;
    bbox->x1 = src[0]->v1->x;
    bbox->y1 = src[0]->v1->y;
    bbox->x2 = src[0]->v1->x;
    bbox->y2 = src[0]->v1->y;

    for (i = 0; i < lines_count; i++) {
        dest->x = ToFloat(src[i]->v1->x);
        dest->y = ToFloat(src[i]->v1->y);
        dest->z = 0;
        dest++;
        dest->x = ToFloat(src[i]->v2->x);
        dest->y = ToFloat(src[i]->v2->y);
        dest->z = 0;
        dest++;
        if (src[i]->v1->x < bbox->x1) {
            bbox->x1 = src[i]->v1->x;
        }
        if (src[i]->v1->y < bbox->y1) {
            bbox->y1 = src[i]->v1->y;
        }
        if (src[i]->v2->x < bbox->x1) {
            bbox->x1 = src[i]->v2->x;
        }
        if (src[i]->v2->y < bbox->y1) {
            bbox->y1 = src[i]->v2->y;
        }

        if (src[i]->v1->x > bbox->x2) {
            bbox->x2 = src[i]->v1->x;
        }
        if (src[i]->v1->y > bbox->y2) {
            bbox->y2 = src[i]->v1->y;
        }
        if (src[i]->v2->x > bbox->x2) {
            bbox->x2 = src[i]->v2->x;
        }
        if (src[i]->v2->y > bbox->y2) {
            bbox->y2 = src[i]->v2->y;
        }
    }
}


static inline void printVert3 (vertex3_t *v)
{
    printf("Vert= %d %d %d\n", v->x >> FRACBITS, v->y >> FRACBITS, v->z >> FRACBITS);
}

static void R_SetupPolys (Poly3f_t *polys, unsigned int poly_cnt, float height, const pixelShader_t *shader)
{
    int i;

    for (i = 0; i < poly_cnt; i++) {
        polys[i].shader = *shader;
        polys[i].v1.z = height;
        polys[i].v2.z = height;
        polys[i].v3.z = height;
        R_FloorPolyToTexCord(&polys[i]);
    }
}

static int R_GroupLines (Poly3f_t *polys, line_t **lines, int lines_count, float height, int picnum, boolean isFloor)
{
    const pixelShader_t shader = {picnum, R_DrawFloorCeilPoly};
    Vertex3f_t verts[MAX_LINES*2];
    unsigned int poly_cnt;
    bbox_t bbox;

    linesCopy(verts, lines, lines_count, &bbox);
    poly_cnt = SR_SplitPolygon2D(polys, verts, lines_count * 2);
    R_SetupPolys(polys, poly_cnt, height, &shader);
    return poly_cnt;
}

static int R_FloorToPoly (Poly3f_t *polys, int isfloor, seg_t *line)
{
    fixed_t fheight = isfloor ? line->frontsector->floorheight : line->frontsector->ceilingheight;
    float height = ToFloat(fheight);
    int picnum = isfloor ? line->frontsector->floorpic : line->frontsector->ceilingpic;
    visplane_t *floorplane = R_FindPlane (line->frontsector->floorheight,
            picnum,
            line->frontsector->lightlevel);

    picnum = floorplane->picnum;

    printf("Lines=%d\n", line->frontsector->linecount);

    if (line->frontsector->linecount > MAX_LINES) {
        printf("Lines > %d\n", MAX_LINES);
        return 0;
    }

    return R_GroupLines(polys, line->frontsector->lines, line->frontsector->linecount, height, picnum, isfloor);
}

int R_SegToPoly (Vertex3f_t *vert, Poly3f_t *poly, seg_vis_t *seg)
{
    seg_t *line = seg->seg;
    fixed_t floor, ceil;
    int vcount = 0, pcount = 0;

    /* Bottom wall */
    if (line->sidedef->bottomtexture && line->backsector) {
        floor = min(line->backsector->floorheight, line->frontsector->floorheight);
        ceil = max(line->backsector->floorheight, line->frontsector->floorheight);
        pcount += R_LineToPoly(line, &vert[vcount], &poly[pcount], floor, ceil - floor, line->sidedef->bottomtexture);
        vcount += pcount * 2;
    }
    /* Top wall */
    if (line->sidedef->toptexture && line->backsector) {
        floor = min(line->backsector->floorheight, line->frontsector->floorheight);
        ceil = max(line->backsector->floorheight, line->frontsector->floorheight);
        pcount += R_LineToPoly(line, &vert[vcount], &poly[pcount], floor, ceil - floor, line->sidedef->toptexture);
        vcount += pcount * 2;
    }
    /* Mid wall */
    if (line->sidedef->midtexture) {
        floor = line->frontsector->floorheight;
        ceil = line->frontsector->ceilingheight;
        pcount += R_LineToPoly(line, &vert[vcount], &poly[pcount], floor, ceil - floor, line->sidedef->midtexture);
        vcount += pcount * 2;
    }

    return pcount;
}

void R_TransformPoly (Poly3f_t *dest, Poly3f_t *src, int cnt)
{
    int i;
    for (i = 0; i < cnt; i++) {

        R_TranslateVert2Dto3D(&dest->v1, &src->v1);
        R_TranslateVert2Dto3D(&dest->v2, &src->v2);
        R_TranslateVert2Dto3D(&dest->v3, &src->v3);

        dest->t1.x = src->t1.x;
        dest->t1.y = src->t1.y;
        dest->t2.x = src->t2.x;
        dest->t2.y = src->t2.y;
        dest->t3.x = src->t3.x;
        dest->t3.y = src->t3.y;

        dest->shader = src->shader;
        dest++;
        src++;
    }
}

void R_RenderPrepare (void)
{
    int seg_count = sscount;
    seg_vis_t *seg = segs_vis;

    while (seg_count--) {
        seg->seg->frontsector->r_prepared = 0;
        seg++;
    }
}

void R_RenderWorld (view_t *view)
{
    int seg_count = sscount, in_poly_cnt = 0, out_poly_cnt = 0;
    seg_vis_t *seg = segs_vis;
    Vertex3f_t vert[8];
    Poly3f_t polys_in[128];
    Poly3f_t polys_out[2048];

    printf("%s() +++ %d\n", __func__, sscount);

    R_RenderPrepare();

    while (seg_count--) {
        in_poly_cnt = R_SegToPoly(vert, polys_in, seg);
        if (!seg->seg->frontsector->r_prepared) {
            in_poly_cnt += R_FloorToPoly(&polys_in[in_poly_cnt], true, seg->seg);
            in_poly_cnt += R_FloorToPoly(&polys_in[in_poly_cnt], false, seg->seg);
            printf("~~~~~~~~~~~~~~%d\n", in_poly_cnt);
            seg->seg->frontsector->r_prepared = 1;
        }
        printf("%s() : %d\n", __func__, in_poly_cnt);

        R_TransformPoly(&polys_out[out_poly_cnt], polys_in, in_poly_cnt);
        out_poly_cnt += in_poly_cnt;
        seg++;
    }

    SR_LoadVert(polys_out, out_poly_cnt);
    printf("%s() ---\n", __func__);
}

void R_ProjectBSP (view_t *view, void (*h) (view_t *view, seg_vis_t *seg) )
{
    int count = sscount;
    seg_vis_t *seg = segs_vis;

    while (count--) {
        h(view, seg);
        seg++;
    }
}

//
// RenderBSPNode
// Renders all subsectors below a given node,
//  traversing subtree recursively.
// Just call with BSP root.
void R_RenderBSPNode (int bspnum, view_t *view)
{
    node_t*bsp;
    int side;

    // Found a subsector?
    if (bspnum & NF_SUBSECTOR)
    {
        if (bspnum == -1)
            R_Subsector (view, 0);
        else
            R_Subsector (view, bspnum&(~NF_SUBSECTOR));
        return;
    }

    bsp = &nodes[bspnum];
    
    // Decide which side the view point is on.
    side = R_PointOnSide (view->orig.x, view->orig.y, bsp);

    // Recursively divide front space.
    R_RenderBSPNode (bsp->children[side], view);

    // Possibly divide back space.
    if (R_CheckBBox (view, bsp->bbox[side^1]))
        R_RenderBSPNode (bsp->children[side^1], view);
}


