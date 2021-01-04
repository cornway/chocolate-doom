#ifndef _VEC3_H_
#define _VEC3_H_

#include <stdint.h>
#include <math.h>

#define RTF_EPSILON (0.00000001f)

typedef int Fixed_t;
#define _FRACBITS 16
#define _FRACUNIT (1 << _FRACBITS)

class Fixed {
    public :

        static inline Fixed_t ToFixed (float f) {
            return (Fixed_t)((float)_FRACUNIT * f);
        }
        static inline float ToFloat (Fixed_t f) {
            return (float)((float)f / (float)_FRACUNIT);
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
      float operator [] (unsigned i) {return v[i];}
      Vec3 &Cross (Vec3 &d, Vec3 &v);
      void Normalize ();

      void Print ();
};

#endif /* _VEC3_H_ */

