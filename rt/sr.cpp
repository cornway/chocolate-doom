/*
MIT License

Copyright (c) 2017-2020 Markus Trenkwalder

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include <iostream>
#include <string>
#include <sstream>
#include <iostream>
#include <map>

#include "sr.h"
#include "Renderer.h"
#include "vector_math.h"
#include "Polygon2D.h"


typedef vmath::vec4<float> vec4f;
typedef vmath::mat4<float> mat4f;

using namespace swr;

class vec3f : public vmath::vec3<float> {
    public:
    vec3f () : vmath::vec3<float>(0) {}
    vec3f (float x) : vmath::vec3<float>(x) {}
    vec3f (float x, float y, float z) : vmath::vec3<float>(x, y, z) {}
    vec3f (vmath::vec3<float> &v) : vmath::vec3<float>(v) {}
    vec3f (vmath::vec3<float> v) : vmath::vec3<float>(v) {}
    vec3f (Vertex3f_t *v) : vmath::vec3<float>(v->x, v->y, v->z) {}

    vec3f &operator = (Vertex3f_t v) {
        x = v.x;
        y = v.y;
        z = v.z;
        return *this;
    }
};

class vec2f : public vmath::vec2<float> {
    public:
    vec2f () : vmath::vec2<float>(0) {}
    vec2f (float x) : vmath::vec2<float>(x) {}
    vec2f (vmath::vec2<float> &v) : vmath::vec2<float>(v) {}
    vec2f (vmath::vec2<float> v) : vmath::vec2<float>(v) {}

    vec2f &operator = (Vertex2f_t &v) {
        x = v.x;
        y = v.y;
        return *this;
    }
};

struct VertexArrayData {
    vec3f vertex;
    vec2f texcoord;
    pixelShader_t shader;
};

struct VertexRef {
    unsigned vertexIndex;
    unsigned normalIndex;
    unsigned texcoordIndex;
};

struct Core {
    static mat4f modelViewProjectionMatrix;
    static mat4f perspectiveMatrix;

    static float *zbuffer;
    static int w, h;

    Rasterizer r;
    VertexProcessor *v;
    class PixelShader : public PixelShaderBase<PixelShader> {
    public:
        static const bool InterpolateZ = false;
        static const bool InterpolateW = false;
        static const int AVarCount = 0;
        static const int PVarCount = 2;

        static void drawPixel(const PixelData &p)
        {
            int tx = p.pvar[0];
            int ty = p.pvar[1];

            assert(p.shader);
            if (Core::zbuffer[p.x + p.y * Core::w] > p.w) {
                if (p.shader->draw(p.shader, p.x, p.y, tx, ty)) {
                    Core::zbuffer[p.x + p.y * Core::w] = p.w;
                }
            }
        }
    };
    class VertexShader : public VertexShaderBase<VertexShader> {
    public:
        static const int AttribCount = 1;
        static const int AVarCount = 0;
        static const int PVarCount = 2;
    
        static void processVertex(VertexShaderInput in, VertexShaderOutput *out)
        {
            const VertexArrayData *data = (const VertexArrayData*)(in[0]);
            vec3f vertex(data->vertex);
            vec2f texcoord(data->texcoord);
    
            vec4f position = modelViewProjectionMatrix * vec4f(vertex, 1.0f);
    
            out->x = position.x;
            out->y = position.y;
            out->z = position.z;
            out->w = position.w;
            out->pvar[0] = texcoord.x;
            out->pvar[1] = texcoord.y;
            out->shader = data->shader;
        }
    };
    Core (int w, int h) : farplane(100000.0f) {

        v = new VertexProcessor(&r);

        r.setRasterMode(RasterMode::Span);
        r.setScissorRect(0, 0, w, h);
        r.setPixelShader<PixelShader>();

        v->setViewport(0, 0, w, h);
        v->setCullMode(CullMode::CW);
        v->setVertexShader<VertexShader>();

        perspectiveMatrix = vmath::perspective_matrix(60.0f, 4.0f / 3.0f, 0.1f, farplane);

        zbuffer = new float[w*h];
        zbuffClear();
        Core::w = w;
        Core::h = h;
    }

    ~Core () {
        delete v;
        delete zbuffer;
    }

    void LoadVert (Poly3f_t *poly, int poly_cnt) {
        VertexArrayData vd;
        int pcnt = 0, i;

        idata.clear();
        vdata.clear();

        for (i = 0, pcnt = 0; pcnt < poly_cnt; i += 3, pcnt++) {
            assert(poly->shader.draw);
            assert(poly->shader.texture);

            vd.vertex = poly->v1;
            vd.texcoord = poly->t1;
            vd.shader = poly->shader;
            vdata.push_back(vd);

            vd.vertex = poly->v2;
            vd.texcoord = poly->t2;
            vd.shader = poly->shader;
            vdata.push_back(vd);

            vd.vertex = poly->v3;
            vd.texcoord = poly->t3;
            vd.shader = poly->shader;
            vdata.push_back(vd);

            idata.push_back(i);
            idata.push_back(i+1);
            idata.push_back(i+2);

            poly++;
        }

    }
    void Render () {
        zbuffClear();
        v->setVertexAttribPointer(0, sizeof(VertexArrayData), &vdata[0]);
        v->drawElements(DrawMode::Triangle, idata.size(), &idata[0]);
    }

    void SetView (Vertex3f_t *pos, Vertex3f_t *dirf) {
        mat4f lookAtMatrix;
        vec3f up(0.0f, 1.0f, 0.0f);
        vec3f dir(dirf);
        vec3f orig(pos);
        float speed = 1.0f;

        lookAtMatrix= vmath::lookat_matrix(orig, dir, up, speed);

        modelViewProjectionMatrix = perspectiveMatrix * lookAtMatrix;

        std::cout << __func__ << "(): orig=" << orig.x << ", " << orig.y << ", " << orig.z << "\n";
        std::cout << __func__ << "(): dir=" << dir.x << ", " << dir.y << ", "  << dir.z << "\n";
    }

private:
    std::vector<VertexArrayData> vdata;
    std::vector<int> idata;
    float farplane;

    void zbuffClear () {
        for (int i = 0; i < w*h; i++)
            zbuffer[i] = farplane;
    }
}*core;

mat4f Core::modelViewProjectionMatrix;
mat4f Core::perspectiveMatrix;
int Core::w;
int Core::h;
float *Core::zbuffer;

void SR_SetupCore (int w, int h)
{
    core = new Core(w, h);
}

void SR_DestroyCore (void)
{
    delete core;
}

void SR_SetupCamera (Vertex3f_t *orig, Vertex3f_t *dir)
{
    core->SetView(orig, dir);
}

void SR_LoadVert (Poly3f_t *poly, int poly_cnt)
{
    core->LoadVert(poly, poly_cnt);
}

void SR_Render (void)
{
    core->Render();
}

static inline void _CopyVert (Vertex3f_t *out, Polygon2D::Vertex2D *in)
{
    out->x = in->x;
    out->y = in->y;
    out->z = 0;
}

int SR_SplitPolygon2D (Poly3f_t *out, Vertex3f_t *in, unsigned int size)
{
    Polygon2D poly;
    std::vector<SubPoly> subPolys;
    std::vector<Polygon2D::Vertex2D *> verts;
    int i;

    for (i = 0; i < size; i += 2) {
        poly.AddEdge(in[i].x, in[i].y, in[i+1].x, in[i+1].y);
    }
    poly.PrintEdges();
    poly.ConnectEdges(verts);

    std::cout << "built verts:\n";
    for (Polygon2D::Vertex2D *v : verts) {
        if (v) {
            v->Print();
        } else {
            std::cout << "null vert\n";
        }
    }
    std::cout << "=======\n\n";
    poly.BuildSubPolys(subPolys, verts);

    std::cout << "SubPolys = " << subPolys.size() << "\n";

    for (SubPoly &p : subPolys) {
        //p.Print();
    }

    i = 0;
    for (SubPoly &p : subPolys) {
        p.Triangulate();

        std::cout << "SubTriangles = " << p.subPolys().size() << "\n";
        for (SubPoly &p : p.subPolys()) {
            p.Print();

            _CopyVert(&out->v1, p[0]);
            _CopyVert(&out->v2, p[1]);
            _CopyVert(&out->v3, p[2]);
            out++;
            i++;
        }
    }

    std::cout << "SR_SplitPolygon2D---\n\n";
    return i;
}


