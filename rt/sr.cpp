
#include "sr.h"
#include "Renderer.h"
#include "vector_math.h"

typedef vmath::vec2<float> vec2f;
typedef vmath::vec3<float> vec3f;
typedef vmath::vec4<float> vec4f;
typedef vmath::mat4<float> mat4f;

using namespace swr;

SR_Mapper_t g_mapper = nullptr;

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

        g_mapper(NULL, tx, ty, p.x, p.y);

        //Uint32 *texBuffer = (Uint32*)((Uint8 *)texture->pixels + (size_t)ty * (size_t)texture->pitch + (size_t)tx * 4);
        //Uint32 *screenBuffer = (Uint32*)((Uint8 *)surface->pixels + (size_t)p.y * (size_t)surface->pitch + (size_t)p.x * 4);

        //*screenBuffer = *texBuffer;
    }
};

class VertexShader : public VertexShaderBase<VertexShader> {
public:
    static const int AttribCount = 1;
    static const int AVarCount = 0;
    static const int PVarCount = 2;

    static mat4f modelViewProjectionMatrix;

    static void processVertex(VertexShaderInput in, VertexShaderOutput *out)
    {
        const VertexArrayData *data = (const VertexArrayData*)(in[0]);
        vec3f vertex(data->vertex.x, data->vertex.y, data->vertex.z);
        vec2f texcoord(data->texvert.x, data->texvert.x);

        vec4f position = modelViewProjectionMatrix * vec4f(vertex, 1.0f);

        out->x = position.x;
        out->y = position.y;
        out->z = position.z;
        out->w = position.w;
        out->pvar[0] = texcoord.x;
        out->pvar[1] = texcoord.y;
    }
};


struct Core {
    Rasterizer r;
    VertexProcessor *v;
    Core (SR_Mapper_t mapper, int w, int h) : mapper(mapper) {
        mat4f lookAtMatrix = vmath::lookat_matrix(vec3f(3.0f, 2.0f, 5.0f), vec3f(0.0f), vec3f(0.0f, 1.0f, 0.0f));
        mat4f perspectiveMatrix = vmath::perspective_matrix(60.0f, 4.0f / 3.0f, 0.1f, 10.0f);

        v = new VertexProcessor(&r);

        r.setRasterMode(RasterMode::Span);
        r.setScissorRect(0, 0, w, h);
        r.setPixelShader<PixelShader>();

        VertexShader::modelViewProjectionMatrix = perspectiveMatrix * lookAtMatrix;
    }

    ~Core () {
        delete v;
    }

    Core &LoadVert (Poly3_t *poly, int poly_cnt) {
        int i;

        for (i = 0; i < poly_cnt; i++) {
            polys.push_back(poly);
            poly++;
        }
        return *this;
    }

    void ToVertexArray () {
        VertexArrayData vd;
        int i = 0;

        idata.clear();
        vdata.clear();

        for (Poly3_t *poly : polys) {
            vd.vertex = poly->v1;
            vd.texvert = poly->t1;
            vdata.push_back(vd);

            vd.vertex = poly->v2;
            vd.texvert = poly->t2;
            vdata.push_back(vd);

            vd.vertex = poly->v3;
            vd.texvert = poly->t3;
            vdata.push_back(vd);

            idata.push_back(i);
            idata.push_back(i+1);
            idata.push_back(i+2);

            i += 3;
        }
    }
    void Render () {
        v->setVertexAttribPointer(0, sizeof(VertexArrayData), &vdata[0]);
        v->drawElements(DrawMode::Triangle, idata.size(), &idata[0]);
    }

private:
    std::vector<Poly3_t *> polys;
    std::vector<VertexArrayData> vdata;
    std::vector<int> idata;
    SR_Mapper_t mapper;
}*core;


void SR_SetupCore (SR_Mapper_t mapper, int w, int h)
{
    core = new Core(mapper, w, h);
    g_mapper = mapper;
}

void SR_DestroyCore (void)
{
    delete core;
}

void SR_SetupCamera (void)
{

}

void SR_LoadVert (Poly3_t *poly, int poly_cnt)
{
    core->LoadVert(poly, poly_cnt);
    core->ToVertexArray();
}

void SR_Render (void)
{
    core->Render();
}


