#include "Player.h"
#include <Windows.h>
Player::Player() {
	this->position = { 0,1,0 };
	this->lookat = {0,0,1};
	this->angleHor = 0.01f;
	this->angleVer = 0.01f;
}

Player::~Player() {

}

void Player::update(float dt, const HWND &window) {


	if (GetAsyncKeyState('W')) {
		position = position + vec3(lookat.x,0,lookat.z)*dt;
	}
	if (GetAsyncKeyState('S')) {
		position = position - vec3(lookat.x, 0, lookat.z)*dt;
	}
	if (GetAsyncKeyState('A')) {
		vec3 rightVec = cross(vec3(0,1,0),lookat);
		position = position + rightVec*dt;
	}
	if (GetAsyncKeyState('D')) {
		vec3 rightVec = cross(vec3(0, 1, 0), lookat);
		position = position - rightVec*dt;
	}
	RECT rect;
	GetWindowRect(window, &rect);
	POINT temp;
	if (GetCursorPos(&temp))
	{	
		float x = rect.left + 540 - temp.x;
		float y = rect.top + 360 - temp.y;
		angleHor = x * -0.001f;
		angleVer = y * -0.001f;
	}
	rotX = { glm::vec3(cosf(angleHor), 0, sinf(angleHor)), glm::vec3(0,1,0), glm::vec3(-sinf(angleHor), 0, cosf(angleHor)) };
	this->lookat = rotX*lookat;
	lookat.y -= angleVer;

	SetCursorPos(540 + rect.left, 360 + rect.top);
}

glm::vec3 Player::getForward() {
	return lookat;
}

glm::vec3 Player::getPosition() {
	return this->position;
}
