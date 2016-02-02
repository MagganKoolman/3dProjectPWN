#pragma once
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