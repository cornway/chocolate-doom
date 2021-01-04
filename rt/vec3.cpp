
#include "doom/r_defs.h"
#include <vec3.h>


Vec3::Vec3 (float x, float y, float z) : v{x, y, z} {}
Vec3::Vec3 () : Vec3(0, 0, 0) {}
Vec3::Vec3 (Vec3 &v) : Vec3(v.v[0], v.v[1], v.v[2]) {}
Vec3::Vec3 (Vec3 &&v) : Vec3(v.v[0], v.v[1], v.v[2]) {}
Vec3::Vec3 (vertex3_t *v) : Vec3(v->x, v->y, v->z) {}
void  Vec3::Init (float x, float y, float z) {
    v[0] = x;
    v[1] = y;
    v[2] = z;
}

Vec3 Vec3::operator - (Vec3 &vec) {
  Vec3 d;

  d.v[0] = v[0] - vec.v[0];
  d.v[0] = v[1] - vec.v[1];
  d.v[0] = v[2] - vec.v[2];
  return d;
}

Vec3 Vec3::operator * (float d) {
  v[0] *= d;
  v[1] *= d;
  v[2] *= d;
  return *this;
}

float Vec3::operator * (Vec3 &vec) {
  return v[0] * vec.v[0] + v[1] * vec.v[1] + v[2] * vec.v[2];
}

Vec3 &Vec3::Cross (Vec3 &d, Vec3 &vec) {
  d.v[0] = v[1] * vec.v[2] - v[2] * vec.v[1];
  d.v[1] = v[2] * vec.v[0] - v[0] * vec.v[2];
  d.v[2] = v[0] * vec.v[1] - v[1] * vec.v[0];
  return d;
}

void Vec3::operator = (Vec3 &&vec) {Init(vec.v[0], vec.v[1], vec.v[2]);}

#define sqr(x) ((x) * (x))

float Vec3::Normalize () {
    return sqrt(sqr(v[0]) + sqr(v[1]) + sqr(v[2]));
}


