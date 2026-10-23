#pragma once
#include <fstream>
#include <limits>
#include <stdexcept>
#include "geometry.h"

inline constexpr int SCREEN_WIDTH=1000, SCREEN_HEIGHT=750;
struct ScreenVertex {float x,y,z;};

class Framebuffer {
public:
    std::vector<uint32_t> pixels;
    std::vector<float> depth;
    Color currentColor=ORANGE;
    Framebuffer():pixels(SCREEN_WIDTH*SCREEN_HEIGHT),depth(pixels.size()) {clear();}
    void setColor(Color c) {currentColor=c;}
    void clear() {
        std::fill(pixels.begin(),pixels.end(),pack(BACKGROUND));
        std::fill(depth.begin(),depth.end(),-std::numeric_limits<float>::infinity());
    }
    static uint32_t pack(Color c) {
        return 0xff000000u|(uint32_t(c.r)<<16)|(uint32_t(c.g)<<8)|c.b;
    }
    static float edge(ScreenVertex a,ScreenVertex b,float x,float y) {
        return (b.x-a.x)*(y-a.y)-(b.y-a.y)*(x-a.x);
    }
    // ESTA ES LA FUNCION QUE PINTA TRIANGULOS EN SOFTWARE.
    // SDL solo mostrara el arreglo final de pixeles.
    void triangle(ScreenVertex a,ScreenVertex b,ScreenVertex c) {
        float area=edge(a,b,c.x,c.y);
        if(std::abs(area)<1e-7f) return;
        int x0=std::max(0,int(std::floor(std::min({a.x,b.x,c.x}))));
        int x1=std::min(SCREEN_WIDTH-1,int(std::ceil(std::max({a.x,b.x,c.x}))));
        int y0=std::max(0,int(std::floor(std::min({a.y,b.y,c.y}))));
        int y1=std::min(SCREEN_HEIGHT-1,int(std::ceil(std::max({a.y,b.y,c.y}))));
        for(int y=y0;y<=y1;++y) for(int x=x0;x<=x1;++x) {
            float px=x+0.5f,py=y+0.5f;
            float w0=edge(b,c,px,py)/area, w1=edge(c,a,px,py)/area;
            float w2=edge(a,b,px,py)/area;
            if(w0<0 || w1<0 || w2<0) continue;
            float z=w0*a.z+w1*b.z+w2*c.z;
            int offset=y*SCREEN_WIDTH+x;
            // Z mayor = mas cerca. La superficie cercana tapa la lejana.
            if(z>depth[offset]) {depth[offset]=z;pixels[offset]=pack(currentColor);}
        }
    }
    // Exportacion sin librerias adicionales: PPM, convertible a PNG con Python.
    void savePPM(const std::string& path) const {
        std::ofstream out(path,std::ios::binary);
        if(!out) throw std::runtime_error("No se pudo guardar: "+path);
        out<<"P6\n"<<SCREEN_WIDTH<<" "<<SCREEN_HEIGHT<<"\n255\n";
        for(uint32_t pixel:pixels) {
            const char rgb[3]={char((pixel>>16)&255),char((pixel>>8)&255),char(pixel&255)};
            out.write(rgb,3);
        }
        if(!out) throw std::runtime_error("Error escribiendo: "+path);
    }
};

inline Vec3 rotate(Vec3 p,float yaw,float pitch) {
    float ca=std::cos(yaw),sa=std::sin(yaw),cb=std::cos(pitch),sb=std::sin(pitch);
    float x=ca*p.x+sa*p.z,z=-sa*p.x+ca*p.z;
    return {x,cb*p.y-sb*z,sb*p.y+cb*z};
}

inline void render(const Mesh& mesh,Framebuffer& fb,float yaw,float pitch) {
    // Algunos vertices de tu OBJ no pertenecen a ninguna cara.
    // Ajustamos el encuadre usando solo los vertices que SI se dibujan.
    Vec3 lo{1e30f,1e30f,1e30f},hi{-1e30f,-1e30f,-1e30f};
    for(const auto& t:mesh.triangles) for(int i:t.indices) {
        Vec3 p=mesh.vertices[i];
        lo={std::min(lo.x,p.x),std::min(lo.y,p.y),std::min(lo.z,p.z)};
        hi={std::max(hi.x,p.x),std::max(hi.y,p.y),std::max(hi.z,p.z)};
    }
    Vec3 center=(lo+hi)*0.5f;
    std::vector<Vec3> rotated;
    for(Vec3 p:mesh.vertices) rotated.push_back(rotate(p-center,yaw,pitch));
    float minX=1e30f,minY=1e30f,maxX=-1e30f,maxY=-1e30f;
    for(const auto& t:mesh.triangles) for(int i:t.indices) {
        Vec3 p=rotated[i];
        minX=std::min(minX,p.x);maxX=std::max(maxX,p.x);
        minY=std::min(minY,p.y);maxY=std::max(maxY,p.y);
    }
    // 0.82 = ocupa como maximo el 82% del ancho o alto de la pantalla.
    float scale=0.82f*std::min(SCREEN_WIDTH/std::max(maxX-minX,0.001f),
                             SCREEN_HEIGHT/std::max(maxY-minY,0.001f));
    auto project=[&](Vec3 p) -> ScreenVertex {
        return {(p.x-(minX+maxX)*0.5f)*scale+SCREEN_WIDTH*0.5f,
                SCREEN_HEIGHT*0.5f-(p.y-(minY+maxY)*0.5f)*scale,p.z};
    };
    Vec3 light=normalized({-0.3f,0.8f,0.6f});
    // RECORRIDO MANUAL: cara -> indices -> tres vertices -> triangulo.
    for(const Triangle& face:mesh.triangles) {
        Vec3 a=rotated[face.indices[0]];
        Vec3 b=rotated[face.indices[1]];
        Vec3 c=rotated[face.indices[2]];
        Color base=colorForPart(face.part);
        auto custom=FACE_COLORS.find(face.faceId);
        if(custom!=FACE_COLORS.end()) base=custom->second;
        Vec3 n=normalized(cross(b-a,c-a));
        float intensity=USE_LIGHTING?0.55f+0.45f*std::max(0.0f,dot(n,light)):1.0f;
        fb.setColor({uint8_t(base.r*intensity),uint8_t(base.g*intensity),uint8_t(base.b*intensity)});
        fb.triangle(project(a),project(b),project(c));
    }
}
