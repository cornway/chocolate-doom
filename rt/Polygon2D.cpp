
#include "Polygon2D.h"

namespace swr {

using namespace std;

Polygon2D::Polygon2D () {
}

void Polygon2D::AddEdge (float x1, float y1, float x2, float y2) {
    edges.push_back(Edge2D(Vertex2D(x1, y1), Vertex2D(x2, y2)));
}

Polygon2D::Edge2D *Polygon2D::ConnectEdge (Polygon2D::Edge2D &in) {
    Edge2D::Edge2DConnection connection;

    for (Edge2D &e : edges) {

        connection = in.canConnect(e);
        if (e != in && connection != Edge2D::Edge2D_NoConnection) {
            switch (connection) {
                case Edge2D::Edge2D_LeftLeft:
                    if (in.left == &e) {
                        continue;
                    }
                    in.left = &e;
                    e.left = &in;
                break;
                case Edge2D::Edge2D_LeftRight:
                    if (in.left == &e) {
                        continue;
                    }
                    in.left = &e;
                    e.right = &in;
                break;
                case Edge2D::Edge2D_RightLeft:
                    if (in.right == &e) {
                        continue;
                    }
                    in.right = &e;
                    e.left = &in;
                break;
                case Edge2D::Edge2D_RightRight:
                    if (in.right == &e) {
                        continue;
                    }
                    in.right = &e;
                    e.right = &in;
                break;
                default:
                break;
            }
            return &e;
        }
    }
    return nullptr;
}

void Polygon2D::ConnectEdges (void) {

    int i;
    Edge2D *e = &edges[0];

    while (e) {
        e = ConnectEdge(*e);
    }
}

void Polygon2D::AddEdge
    (vector<Polygon2D::Vertex2D *> *out, Polygon2D::Edge2D *prev, Polygon2D::Edge2D *cur, Polygon2D::Edge2D *next) {

    if (!prev && !next) {
        return;
    }

    if (!next) {
        if (cur->v1 == prev->v1 || cur->v1 == prev->v2) {
            out->push_back(&cur->v2);
        } else if (cur->v2 == prev->v1 || cur->v2 == prev->v2) {
            out->push_back(&cur->v1);
        }
    } else if (cur->v1 == next->v1 || cur->v1 == next->v2) {
        out->push_back(&cur->v1);
    } else if (cur->v2 == next->v1 || cur->v2 == next->v2) {
        out->push_back(&cur->v2);
    }
}

void Polygon2D::BuildVerts (vector<Vertex2D *> &verts) {
    Edge2D *cur, *right, *first;
    int i = 0;

    for (Edge2D &e : edges) {

        cur = &e;
        if (!cur->right) {
            continue;
        }
        first = cur;
        do {
            right = cur->right;
            AddEdge(&verts, cur->left, cur, cur->right);
            cur->right = nullptr;
            cur = right;
        } while (cur && cur != first);

        verts.push_back(nullptr);
    }
}

void Polygon2D::BuildSubPolys (std::vector<SubPoly> &out, std::vector<Polygon2D::Vertex2D *> &vertsList) {
    SubPoly poly;
    for (Vertex2D *v : vertsList) {
        if (v == nullptr && poly.NumVerts()) {
            out.push_back(poly);
            poly.Print();
            poly.Clear();
            continue;
        }
        poly.AddVert(v);
    }
    if (poly.NumVerts()) {
        out.push_back(poly);
    }
}


void Polygon2D::PrintEdges (void) {
    std::cout << "PrintEdges:\n";
    for (Edge2D &e : edges) {
        e.Print();
    }
    std::cout << "==========\n\n";
}

} /* namespace swr */

