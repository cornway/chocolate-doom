#pragma once

#include <vector>
#include <iostream>

#include "vector_math.h"

namespace swr {

template <typename T>
class Node {
    public :
        Node () : next(nullptr), prev(nullptr) {}
        T *next, *prev;
};

class SubPoly;

class Polygon2D {
    public :
        struct Vertex2D :    public vmath::vec2<float> {
            Vertex2D (float x, float y) : vmath::vec2<float>(x, y) {}
            Vertex2D (Vertex2D *v) : Vertex2D(v->x, v->y) {}
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
            bool v1conn, v2conn;

            Edge2D (float x1, float y1, float x2, float y2) : v1(x1, y1), v2(x2, y2), v1conn(false), v2conn(false) {}
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
            bool Connected () {
                return v1conn && v2conn;
            }
            void Print () {
                std::cout << "Edge2D\n";
                v1.Print();
                v2.Print();
            }
        };
        Polygon2D ();

        void AddEdge (float x1, float y1, float x2, float y2);
        void BuildSubPolys (std::vector<SubPoly> &out, std::vector<Vertex2D *> &vertsList);
        void PrintEdges ();
        void ConnectEdges (std::vector<Vertex2D *> &verts);

    private :
        std::vector<Edge2D> edges;

        Edge2D *ConnectEdge (std::vector<Vertex2D *> &verts, Edge2D *in);
        void AddEdge (std::vector<Vertex2D *> *out, Edge2D *prev, Edge2D *cur, Edge2D *next);
};

class SubPoly {
    public :
        class Vertex2dNode : public Node<Vertex2dNode>,
                                      public Polygon2D::Vertex2D {
            public:
                Vertex2dNode (Polygon2D::Vertex2D *v) : Polygon2D::Vertex2D(v) {}
                void Print () {
                    std::cout << "Vertex2DNode:\n";
                    Polygon2D::Vertex2D::Print();
                    std::cout << "prev=" << prev << ", next=" << next << ";\n\n";
                }
        };
        SubPoly (Polygon2D::Vertex2D *v1, Polygon2D::Vertex2D *v2, Polygon2D::Vertex2D *v3) : head(nullptr) {
            verts.clear();
            AddVert(v1);
            AddVert(v2);
            AddVert(v3);
        }
        SubPoly () : head(nullptr) {verts.clear();}

        void AddVert (Polygon2D::Vertex2D *v) {
            Vertex2dNode *node;

            if (v == nullptr) {
                return;
            }
            verts.push_back(Vertex2dNode(v));
        }
        Polygon2D::Vertex2D *operator [] (int i) {
            return &verts[i];
        }
        unsigned int NumVerts () {
            return verts.size();
        }
        void Clear () {
            verts.clear();
        }
        void LinkVerts ();
        void Triangulate ();
        std::vector<SubPoly> &subPolys () {return subPoly;}
        void Print () {
            std::cout << "SubPoly:\n";
            for (Vertex2dNode v : verts) {
                v.Print();
            }
            std::cout << "==========\n\n";
        }

    private :
        Vertex2dNode *head;
        std::vector<Vertex2dNode> verts;
        std::vector<SubPoly> subPoly;
};

} /* namespace swr */

