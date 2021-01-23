#pragma once

#include <vector>
#include <iostream>

namespace swr {

class SubPoly;

class Polygon2D {
    public :
        struct Vertex2D {
            float x, y;
            Vertex2D (float x, float y) : x(x), y(y) {}
            bool operator == (Vertex2D &v) {
                return (x == v.x) && (y == v.y);
            }
            void Print () {
                std::cout << "Vertex2D (x,y)=" << x << ", " << y << ";\n";
            }
        };
        struct Edge2D {
            typedef enum {
                Edge2D_NoConnection,
                Edge2D_LeftRight,
                Edge2D_LeftLeft,
                Edge2D_RightLeft,
                Edge2D_RightRight,
            } Edge2DConnection;
            Vertex2D v1, v2;
            Edge2D *left, *right;
            Edge2D (float x1, float y1, float x2, float y2) : v1(x1, y1), v2(x2, y2), left(nullptr), right(nullptr) {}
            Edge2D (Vertex2D &&v1, Vertex2D &&v2) : Edge2D(v1.x, v1.y, v2.x, v2.y) {}
            bool operator == (Edge2D &e) {
                return (v1 == e.v1) && (v2 == e.v2);
            }
            bool operator != (Edge2D &e) {
                return !(*this == e);
            }
            Edge2DConnection canConnect (Edge2D &e) {
                if (v1 == e.v1) {
                    return Edge2D_LeftLeft;
                }
                if (v1 == e.v2) {
                    return Edge2D_LeftRight;
                }
                if (v2 == e.v1) {
                    return Edge2D_RightLeft;
                }
                if (v2 == e.v2) {
                    return Edge2D_RightRight;
                }
                return Edge2D_NoConnection;
            }
            void Print () {
                std::cout << "Edge2D\n";
                v1.Print();
                v2.Print();
            }
        };
        Polygon2D ();

        void AddEdge (float x1, float y1, float x2, float y2);
        void BuildVerts (std::vector<Vertex2D *> &vertsList);
        void BuildSubPolys (std::vector<SubPoly> &out, std::vector<Vertex2D *> &vertsList);
        void PrintEdges ();
        void ConnectEdges (void);

    private :
        std::vector<Edge2D> edges;

        Edge2D *ConnectEdge (Edge2D &in);
        void AddEdge (std::vector<Vertex2D *> *out, Edge2D *prev, Edge2D *cur, Edge2D *next);
};

class SubPoly {
    public :
        void AddVert (Polygon2D::Vertex2D *v) {
            verts.push_back(v);
        }
        unsigned int NumVerts () {
            return verts.size();
        }
        void Clear () {
            verts.clear();
        }
        void Print () {
            std::cout << "SubPly:\n";
            for (Polygon2D::Vertex2D *v : verts) {
                v->Print();
            }
            std::cout << "==========\n\n";
        }

    private :
        std::vector<Polygon2D::Vertex2D *> verts;
};

} /* namespace swr */

