#ifndef _SR_H_
#define _SR_H_

#ifdef __cplusplus
    extern "C" {
#endif

typedef struct {
    float x, y, z;
} Vertex3f_t;

typedef struct {
    float x, y;
} Vertex2f_t;

typedef unsigned int (*SR_Mapper_t) (void *, int, int, int, int);

typedef struct Poly3_s {
    Vertex3f_t v1, v2, v3;
    Vertex2f_t t1, t2, t3;
    void *data;
} Poly3_t;

typedef struct VertexArrayData_s {
    Vertex3f_t vertex;
    Vertex2f_t texvert;
} VertexArrayData;

void SR_SetupCore (SR_Mapper_t mapper, int w, int h);
void SR_DestroyCore (void);
void SR_SetupCamera (void);
void SR_LoadVert (Poly3_t *poly, int poly_cnt);
void SR_Render (void);

#ifdef __cplusplus
}
#endif

#endif /* _SR_H_ */

