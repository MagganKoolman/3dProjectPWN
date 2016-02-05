#pragma once
#include <string>
#include <gl/glew.h>
struct vtx {
	float x, y, z;
};
struct nvtx {
	float x, y, z;
};
struct tex {
	float u, v;
};
struct face {
	int v[3], n[3], t[3];
};
struct material {
	vec3 Ka, Kd, Ks;
	std::string materialName;
	GLuint texid;
};
struct TriangleVertex
{
	float x, y, z, u, v, nx, ny, nz;
};
