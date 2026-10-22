#pragma once
#include <algorithm>
#include <cmath>
#include <vector>
#include <string>
#include "colors.h"

struct Vec3 { float x=0, y=0, z=0; };
inline Vec3 operator+(Vec3 a, Vec3 b) {return {a.x+b.x,a.y+b.y,a.z+b.z};}
inline Vec3 operator-(Vec3 a, Vec3 b) {return {a.x-b.x,a.y-b.y,a.z-b.z};}
inline Vec3 operator*(Vec3 a, float s) {return {a.x*s,a.y*s,a.z*s};}
inline float dot(Vec3 a, Vec3 b) {return a.x*b.x+a.y*b.y+a.z*b.z;}
inline Vec3 cross(Vec3 a, Vec3 b) {
    return {a.y*b.z-a.z*b.y,a.z*b.x-a.x*b.z,a.x*b.y-a.y*b.x};
}
inline Vec3 normalized(Vec3 a) {
    float len=std::sqrt(dot(a,a));return len>1e-12f?a*(1.0f/len):Vec3{};
}
struct Triangle {
    int indices[3];
    std::string object;
    int faceId=0;
    Part part=Part::Body;
};
struct Mesh {std::vector<Vec3> vertices; std::vector<Triangle> triangles;};
