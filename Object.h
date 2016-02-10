#pragma once
#include "datatypes.h"
#include "glm\gtx\transform.hpp"
#include "glm\gtc\matrix_transform.hpp"
#include <vector>


class Object {
	//private:
	public:
		std::vector<face> faces;
		GLuint texture;
		GLuint VAOid;
		GLuint BUFFid;
		material* mat;
		glm::mat4x4 modelMatrix;
	public:
		Object();
		~Object();
		void setTexture(GLuint text);
		void addFace(face f);
		bool isEmpty();
		void setModelTranslation(glm::vec3 pos);
		void setModelScale(glm::vec3 scale);
		glm::mat4x4 getModelMatrix();
};