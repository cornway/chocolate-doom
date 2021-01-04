
#include "doom/r_defs.h"
#include "poly.h"

Poly3::Poly3 (Vec3 &&v0, Vec3 &&v1, Vec3 &&v2) : v{v0, v1, v2} {}
Poly3::Poly3 () : v{Vec3(), Vec3(), Vec3()} {}

Poly3::Poly3 (vertex3_t *v0, vertex3_t *v1, vertex3_t *v2) : v{Vec3(v0), Vec3(v1), Vec3(v2)} {};


bool Poly3::Intersect (Vec3 &orig, Vec3 &dir, float *u, float *vv, float *t) {
    Vec3 edge1, edge2, tvec, pvec, qvec;
    float det, inv_det;

    edge1 = v[1] - v[0];
    edge1 = v[2] - v[0];

    dir.Cross(pvec, edge2);

    det = edge1 * pvec;

    if (det < RTF_EPSILON) {
        return false;
    }

    tvec = orig - v[0];

    *u = tvec * pvec;

    if (*u < 0 || *u > det) {
        return false;
    }

    tvec.Cross(qvec, edge1);

    *vv = dir * qvec;

    if (*vv < 0 || *vv + *u > det) {
        return false;
    }

    *t = edge2 * qvec;
    inv_det = 1.0f / det;

    *t *= inv_det;
    *u *= inv_det;
    *vv *= inv_det;
    return true;
}


