#include "Camera.hpp"
#include <iostream>

Camera::Camera(unsigned int shaderProgram, float moveSpeed){
	

	this->shaderProgram = shaderProgram;
	this->moveSpeed = moveSpeed;

	this->model = glm::mat4(1.0f);
}

void Camera::update(GLFWwindow* window){
	moveDirection = glm::vec2(0.0f);

	readInput(window);
	model = glm::translate(this->model, glm::vec3(moveDirection.x, moveDirection.y, 0.0f));
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "u_MVP"), 1, GL_FALSE, glm::value_ptr(model));
}

void Camera::readInput(GLFWwindow* window){
	if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
        moveDirection.x = moveSpeed;
    }
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
         moveDirection.x = -moveSpeed;
    }
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
        moveDirection.y = -moveSpeed;
    }
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
        moveDirection.y = moveSpeed;
    }
}