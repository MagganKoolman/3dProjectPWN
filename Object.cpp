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
void Object::setModelTranslation(glm::vec3 pos) {
	modelMatrix = glm::translate(modelMatrix, pos);
}
void Object::setModelScale(glm::vec3 scale) {
	modelMatrix = glm::scale(modelMatrix, scale);
}
glm::mat4x4 Object::getModelMatrix() {
	return modelMatrix;
}