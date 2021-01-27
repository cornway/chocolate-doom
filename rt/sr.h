#ifndef _SR_H_
#define _SR_H_

#ifdef __cplusplus
    extern "C" {
#endif

typedef int Fixed_t;

typedef struct pixelShader {
    int texture;
    unsigned int (*draw)(const struct pixelShader *shader, int x, int y, int tx, int ty);
} pixelShader_t;

typedef struct
{
    Fixed_t	x;
    Fixed_t	y;
    Fixed_t z;
} vertex3_t;

typedef struct
{
    Fixed_t	x;
    Fixed_t	y;
} vertex2_t;

typedef struct {
    vertex3_t v1, v2, v3;
    vertex2_t tv1, tv2, tv3;
    pixelShader_t shader;
} poly3_t;

typedef struct {
    float x, y, z;
} Vertex3f_t;

typedef struct {
    float x, y;
} Vertex2f_t;

typedef struct Poly3f_s {
    Vertex3f_t v1, v2, v3;
    Vertex2f_t t1, t2, t3;
    pixelShader_t shader;
} Poly3f_t;

void SR_SetupCore (int w, int h);
void SR_DestroyCore (void);
void SR_SetupCamera (Vertex3f_t *orig, Vertex3f_t *dir);
void SR_LoadVert (Poly3f_t *poly, int poly_cnt);
void SR_Render (void);
int SR_SplitPolygon2D (Poly3f_t *out, Vertex3f_t *in, unsigned int size);


#ifdef __cplusplus
}
#endif

#endif /* _SR_H_ */

