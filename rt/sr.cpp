#include <iostream>

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

        std::cout << __func__ << "(): " << p.x << " "  << p.y << " " << tx << " "  << ty << " "  << "\n";

        g_mapper(p.texture, tx, ty, p.x, p.y);

        //Uint32 *texBuffer = (Uint32*)((Uint8 *)texture->pixels + (size_t)ty * (size_t)texture->pitch + (size_t)tx * 4);
        //Uint32 *screenBuffer = (Uint32*)((Uint8 *)surface->pixels + (size_t)p.y * (size_t)surface->pitch + (size_t)p.x * 4);

        //*screenBuffer = *texBuffer;
    }
};

static mat4f modelViewProjectionMatrix;
static mat4f perspectiveMatrix;

class VertexShader : public VertexShaderBase<VertexShader> {
public:
    static const int AttribCount = 1;
    static const int AVarCount = 0;
    static const int PVarCount = 2;

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

        std::cout << __func__ << "() :" << out->x << ", " << out->y << ", " << out->z << ", " << out->w << "\n";
    }
};


struct Core {
    Rasterizer r;
    VertexProcessor *v;
    Core (SR_Mapper_t mapper, int w, int h) : mapper(mapper) {

        v = new VertexProcessor(&r);

        r.setRasterMode(RasterMode::Span);
        r.setScissorRect(0, 0, w, h);
        r.setPixelShader<PixelShader>();

        v->setViewport(0, 0, w, h);
        v->setCullMode(CullMode::CW);
        v->setVertexShader<VertexShader>();
    }

    ~Core () {
        delete v;
    }

    void LoadVert (Poly3_t *poly, int poly_cnt) {
        VertexArrayData vd;
        int pcnt = 0, i;

        std::cout << __func__ << "+++\n";

        idata.clear();
        vdata.clear();

        for (i = 0, pcnt = 0; pcnt < poly_cnt; i += 3, pcnt++) {
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

            textures.push_back(poly->data);

            std::cout << __func__ << "():" << i << "; " << poly->v1.x << " " << poly->v1.y << " " << poly->v1.z << "\n";
            std::cout << __func__ << "():" << i+1 << "; " << poly->v2.x << " " << poly->v2.y << " " << poly->v2.z << "\n";
            std::cout << __func__ << "():" << i+2 << "; " << poly->v3.x << " " << poly->v3.y << " " << poly->v3.z << "\n";
            poly++;
        }
    }
    void Render () {
        v->setVertexAttribPointer(0, sizeof(VertexArrayData), &vdata[0]);
        v->drawElements(DrawMode::Triangle, idata.size(), &idata[0], &textures[0]);
    }

    void SetView (Vertex3f_t *orig, Vertex3f_t *dir) {
        mat4f lookAtMatrix;

        lookAtMatrix= vmath::lookat_matrix(vec3f(orig->x, orig->y, orig->z), vec3f(dir->x, dir->y, dir->z), vec3f(0.0f, 1.0f, 0.0f));
        perspectiveMatrix = vmath::perspective_matrix(60.0f, 4.0f / 3.0f, 0.1f, 10.0f);

        modelViewProjectionMatrix = perspectiveMatrix * lookAtMatrix;

        std::cout << __func__ << "(): orig=" << orig->x << ", " << orig->y << ", " << orig->z << "\n";
        std::cout << __func__ << "(): dir=" << dir->x << ", " << dir->y << ", "  << dir->z << "\n";
    }

private:
    std::vector<VertexArrayData> vdata;
    std::vector<int> idata;
    std::vector<void *> textures;
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

void SR_SetupCamera (Vertex3f_t *orig, Vertex3f_t *dir)
{
    core->SetView(orig, dir);
}

void SR_LoadVert (Poly3_t *poly, int poly_cnt)
{
    core->LoadVert(poly, poly_cnt);
}

void SR_Render (void)
{
    core->Render();
}


