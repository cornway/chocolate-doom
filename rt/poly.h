#ifndef _POLY_H_
#define _POLY_H_

#include "vec3.h"

class Poly3 {
    private:
        Vec3 v[3];
    public:
        Poly3 ();
        Poly3 (Vec3 &v0, Vec3 &v1, Vec3 &v2);
        Poly3 (Vec3 &&v0, Vec3 &&v1, Vec3 &&v2);
        Poly3 (vertex3_t *v0, vertex3_t *v1, vertex3_t *v2);

       bool Intersect (Vec3 &orig, Vec3 &dir, float *u, float *v, float *t);
};

#endif /* _POLY_H_ */

