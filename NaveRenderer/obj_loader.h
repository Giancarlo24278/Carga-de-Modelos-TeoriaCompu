#pragma once
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <numeric>
#include <map>
#include <limits>
#include "geometry.h"

// Los indices de OBJ son globales: NO se reinician al encontrar otro objeto.
inline int objIndex(const std::string& token, int vertexCount) {
    // "12/3/5" o "12//5" -> usamos solo el 12 (posicion del vertice).
    std::string value=token.substr(0,token.find('/'));
    std::size_t consumed=0;
    int raw=std::stoi(value,&consumed);
    if(consumed!=value.size() || raw==0) throw std::runtime_error("Indice OBJ invalido");
    int index=raw>0?raw-1:vertexCount+raw;
    if(index<0 || index>=vertexCount) throw std::runtime_error("Indice fuera del arreglo");
    return index;
}

inline Mesh loadOBJ(const std::string& path) {
    std::ifstream file(path);
    if(!file) throw std::runtime_error("No se pudo abrir: "+path);
    Mesh mesh; std::string line,object="default"; int faceId=0,lineNumber=0;
    while(std::getline(file,line)) {
        ++lineNumber; std::istringstream input(line); std::string type; input>>type;
        if(type.empty() || type[0]=='#') continue;
        try {
            if(type=="v") {
                Vec3 v;
                if(!(input>>v.x>>v.y>>v.z) || !std::isfinite(v.x) ||
                   !std::isfinite(v.y) || !std::isfinite(v.z))
                    throw std::runtime_error("Vertice invalido");
                mesh.vertices.push_back(v);
            } else if(type=="o") {
                input>>object;
            } else if(type=="f") {
                ++faceId; std::vector<int> face; std::string token;
                while(input>>token) {
                    if(token[0]=='#') break;
                    face.push_back(objIndex(token,int(mesh.vertices.size())));
                }
                if(face.size()<3) throw std::runtime_error("Cara con menos de tres vertices");
                // Tu OBJ ya viene triangulado. Esto tambien admite quads convexos.
                // Para poligonos concavos, triangula primero desde Blender.
                for(std::size_t i=1;i+1<face.size();++i) {
                    Triangle t;
                    t.indices[0]=face[0];t.indices[1]=face[i];t.indices[2]=face[i+1];
                    t.object=object;t.faceId=faceId;mesh.triangles.push_back(t);
                }
            }
            // vt, vn, mtllib, usemtl: no son necesarios para estos colores planos.
        } catch(const std::exception& e) {
            throw std::runtime_error("OBJ linea "+std::to_string(lineNumber)+": "+e.what());
        }
    }
    if(mesh.triangles.empty()) throw std::runtime_error("El OBJ no tiene caras");
    return mesh;
}

// Encuentra piezas conectadas, porque Cylinder contiene varias piezas juntas.
// Esto NO cambia el archivo OBJ ni sus coordenadas.
inline void assignParts(Mesh& mesh) {
    std::vector<int> parent(mesh.vertices.size());
    std::iota(parent.begin(),parent.end(),0);
    auto root=[&](int a) {
        while(parent[a]!=a) {parent[a]=parent[parent[a]];a=parent[a];}
        return a;
    };
    for(const auto& t:mesh.triangles) {
        parent[root(t.indices[1])]=root(t.indices[0]);
        parent[root(t.indices[2])]=root(t.indices[0]);
    }
    struct Bounds {
        Vec3 lo{1e30f,1e30f,1e30f},hi{-1e30f,-1e30f,-1e30f};
        void add(Vec3 p) {
            lo={std::min(lo.x,p.x),std::min(lo.y,p.y),std::min(lo.z,p.z)};
            hi={std::max(hi.x,p.x),std::max(hi.y,p.y),std::max(hi.z,p.z)};
        }
    };
    std::map<int,Bounds> components;
    for(const auto& t:mesh.triangles)
        for(int i:t.indices) components[root(i)].add(mesh.vertices[i]);

    for(auto& t:mesh.triangles) {
        Vec3 a=mesh.vertices[t.indices[0]],b=mesh.vertices[t.indices[1]],
             c=mesh.vertices[t.indices[2]];
        const auto& bounds=components[root(t.indices[0])];
        if(t.object=="Cylinder" || t.object=="Cylinder.001") {
            Vec3 n=normalized(cross(b-a,c-a));
            if(bounds.hi.y>-1.0f) t.part=Part::UpperCylinders;
            else if(bounds.lo.z<-1.5f) t.part=Part::LeftTurbine;
            else if(bounds.hi.z>1.5f) t.part=Part::RightTurbine;
            else t.part=Part::LowerCylinders;
            if(t.part==Part::LeftTurbine || t.part==Part::RightTurbine) {
                // Distancia al eje de la turbina (plano XZ), relativa a su radio.
                float cx=(bounds.lo.x+bounds.hi.x)*0.5f,cz=(bounds.lo.z+bounds.hi.z)*0.5f;
                float radius=(bounds.hi.x-bounds.lo.x)*0.5f,far=0.0f;
                for(Vec3 p:{a,b,c}) far=std::max(far,std::hypot(p.x-cx,p.z-cz));
                if(far<0.8f*radius) t.part=Part::TurbineCore;          // Disco interior.
                else if(std::abs(n.y)<0.3f) t.part=Part::TurbineRim;   // Pared exterior.
            } else if(n.x>0.3f) {
                // Cono y tapa que apuntan al frente (X positivo): toberas.
                t.part=Part::Nozzle;
            }
        } else {
            t.part=Part::Body;
            Vec3 n=normalized(cross(b-a,c-a));
            float minX=std::min({a.x,b.x,c.x}),maxX=std::max({a.x,b.x,c.x});
            // CAMBIA ESTA REGLA si quieres otra superficie como ventana.
            // El frente (X positivo) es un cubo biselado: visto de frente es
            // una cuadricula 3x3. Como la nave se dibuja volteada, la fila de
            // abajo del OBJ (n.y<0) queda arriba en pantalla. Ventana = esa
            // fila (1 2 3) + la fila del medio (4 5 6); se excluye la otra.
            if(minX>1.20f && n.x>0.2f && n.y<0.2f)
                t.part=Part::Window;
            // Salidita debajo del cuerpo en el OBJ (arriba ya volteada).
            // Del frente hacia atras: punta teal, anillo gris, caja salmon.
            else if(std::min({a.y,b.y,c.y})<-1.85f) {
                if(minX<-1.0f) t.part=Part::RidgeBack;
                else if(maxX<-0.6f) t.part=Part::RidgeMiddle;
                else t.part=Part::RidgeFront;
            }
            // Techo: caras casi horizontales situadas arriba del cuerpo.
            else if(std::min({a.y,b.y,c.y})>-0.60f && n.y>0.8f)
                t.part=Part::Roof;
        }
    }
}
