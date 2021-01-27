
#include "Polygon2D.h"

namespace swr {

using namespace std;

Polygon2D::Polygon2D () {
}

void Polygon2D::AddEdge (float x1, float y1, float x2, float y2) {
    edges.push_back(Edge2D(Vertex2D(x1, y1), Vertex2D(x2, y2)));
}

Polygon2D::Edge2D *Polygon2D::ConnectEdge (vector<Polygon2D::Vertex2D *> &verts, Polygon2D::Edge2D *in) {

    for (Edge2D &e : edges) {

        if (e != *in) {
            switch (in->canConnect(e)) {
                case Edge2D::Edge2D_LeftLeft:
                    /* TODO: Branches */
                    if (in->v1conn || e.v1conn) {
                        continue;
                    }
                    in->v1conn = true;
                    e.v1conn = true;
                    verts.push_back(&in->v1);
                break;
                case Edge2D::Edge2D_LeftRight:
                    if (in->v1conn || e.v2conn) {
                        continue;
                    }
                    in->v1conn = true;
                    e.v2conn = true;
                    verts.push_back(&in->v1);
                break;
                case Edge2D::Edge2D_RightLeft:
                    if (in->v2conn || e.v1conn) {
                        continue;
                    }
                    in->v2conn = true;
                    e.v1conn = true;
                    verts.push_back(&in->v2);
                break;
                case Edge2D::Edge2D_RightRight:
                    if (in->v2conn || e.v2conn) {
                        continue;
                    }
                    in->v2conn = true;
                    e.v2conn = true;
                    verts.push_back(&in->v2);
                break;
                default:
                    continue;
                break;
            }
            //std::cout << "\nConnected : " << in->canConnect(e) << "\n";
            //in->Print();
            //e.Print();
            //std::cout << "~~~~~~\n";
            return &e;
        }
    }
    return nullptr;
}

void Polygon2D::ConnectEdges (vector<Polygon2D::Vertex2D *> &verts) {

    int i;
    Edge2D *ep;

    for (Edge2D &e : edges) {
        if (e.Connected()) {
            continue;
        }
        ep = &e;
        while (ep) {
            ep = ConnectEdge(verts, ep);
        }
        verts.push_back(nullptr);
    }
}

void Polygon2D::BuildSubPolys (std::vector<SubPoly> &out, std::vector<Polygon2D::Vertex2D *> &vertsList) {
    out.resize(out.size() + 1);
    int i = 0;

    for (Vertex2D *v : vertsList) {
        if (v) {
            out[i].AddVert(v);
        } else {
            i++;
            out.resize(out.size() + i);
        }
    }
    if (!out[i].NumVerts()) {
        out[i].AddVert(nullptr);
    }
}

void Polygon2D::PrintEdges (void) {
    std::cout << "PrintEdges:\n";
    for (Edge2D &e : edges) {
        e.Print();
    }
    std::cout << "==========\n\n";
}

template <typename T>
static float _Cross (T *v1, T *v2, T *v3) {
    float dx1 = v2->x - v1->x;
    float dy1 = v2->y - v1->y;
    float dx2 = v3->x - v2->x;
    float dy2 = v3->y - v2->y;

    return dx1 * dy2 - dy1 * dx2;
}

void SubPoly::Triangulate () {
    Vertex2dNode *node, *prev, *next;

    std::cout << "Triangulate:" << verts.size() << "\n";

    LinkVerts();

    if (verts.size() < 2) {
        return;
    }
    for (Vertex2dNode &v : verts) {
        //v.Print();
    }
    node = &verts[0];
    node = node->next;
    prev = node->prev;
    next = node->next;

    while (node && next) {
        if (_Cross(prev, node, next) > 0) {
            subPoly.push_back(SubPoly(prev, node, next));
            prev->next = next;
            next->prev = prev;
            node = next;
            next = next->next;
        } else {
            prev = node;
            node = next;
            next = next->next;
        }
    }
}


void SubPoly::LinkVerts () {
    Vertex2dNode *node = &verts[0];

    if (verts.size() < 2) {
        return;
    }

    for (Vertex2dNode &v : verts) {
        if (node == &v) {
            continue;
        }
        node->next = &v;
        v.prev = node;
        node = &v;
    }
}


} /* namespace swr */

