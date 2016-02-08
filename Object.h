#pragma once
#include "datatypes.h"
#include <vector>


class Object {
	//private:
	public:
		std::vector<face> faces;
		GLuint texture;
		GLuint VAOid;
		GLuint BUFFid;
		material* mat;
	public:
		Object();
		~Object();
		void setTexture(GLuint text);
		void addFace(face f);
		bool isEmpty();
};