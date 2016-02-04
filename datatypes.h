#pragma once
#include <string>
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
	float Ka[3], Kd[3], Ks[3];
	std::string materialName;
};