#ifndef _VEC3_H_
#define _VEC3_H_

#include <stdint.h>
#include <math.h>

#define RTF_EPSILON (0.00000001f)

typedef unsigned int Fixed_t;
#define _FRACBITS 16
#define _FRACUNIT (1 << _FRACBITS)

class Fixed {
    public :

        static Fixed_t ToFixed (float f) {
            return (Fixed_t)((float)_FRACUNIT * f);
        }
        static float ToFloat (Fixed_t f) {
            float m, e;
            e = (float)(f >> _FRACBITS);
            m = (float)((float)(f & (_FRACUNIT - 1)) / (float)_FRACUNIT);
            return e + m;
        }
};

class Vec3 {
    private:
        float v[3];
    public:
      Vec3 ();
      Vec3 (float x, float y, float z);
      Vec3 (Vec3 &vec);
      Vec3 (Vec3 &&vec);
      Vec3 (vertex3_t *v);

      void Init (float x, float y, float z);

      Vec3 operator - (Vec3 &v);
      Vec3 operator * (float d);
      float operator * (Vec3 &v);
      void operator = (Vec3 &&v);
      Vec3 &Cross (Vec3 &d, Vec3 &v);
      float Normalize ();
};

#endif /* _VEC3_H_ */

