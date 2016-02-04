#pragma once
#include "datatypes.h"
#include <gl/glew.h>
#include <vector>


class Object {
	//private:
	public:
		std::vector<face> faces;
		GLuint texture;
		material* mat;
	public:
		Object();
		void setTexture(GLuint text);
		void addFace(face f);
		bool isEmpty();
};