#include "Object.h"

Object::Object() {
	this->texture = 0;
}
Object::~Object() {

}
void Object::setTexture(GLuint text) {
	this->texture = text;
}
void Object::addFace(face f) {
	this->faces.push_back(f);
}
bool Object::isEmpty() {
	return (this->faces.size() == 0);
}