#ifndef PLAYER_H
#define PLAYER_H
#include "glm\glm.hpp" 
#include <windows.h>

const float PI = 3.1415f;
const float halfPI = PI / 2;

using namespace glm;
class Player {
	private:
		glm::mat3x3 rotY;
		glm::mat3x3 rotX;
		glm::vec3 position;
		glm::vec3 lookat;
		float angleHor;
		float angleVer;
		POINT cursor;
	public:
		Player();
		~Player();
		void update(float dt, const HWND &hDC);
		glm::vec3 getForward();
		glm::vec3 getPosition();
};

#endif