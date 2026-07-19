#include "Camera.hpp"
#include <iostream>

Camera::Camera(unsigned int shaderProgram, float moveSpeed, float windowWidth, float windowHeight){
	

	this->shaderProgram = shaderProgram;
	this->moveSpeed = moveSpeed;

	this->model = glm::mat4(1.0f);
    this->view = glm::mat4(1.0f);
    glm::mat4 projection = glm::ortho(0.0f, (float)windowWidth, 0.0f, (float)windowHeight, -1.0f, 1.0f);
}

void Camera::update(GLFWwindow* window){
	moveDirection = glm::vec2(0.0f);

	readInput(window);

	view = glm::translate(this->view, glm::vec3(moveDirection.x, moveDirection.y, 0.0f));
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "u_MVP"), 1, GL_FALSE, glm::value_ptr(view * model));
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

glm::mat4 Camera::getViewMatrix(){
    return view;
}
glm::mat4 Camera::getProjectionMatrix(){
    return projection;
}

glm::mat4 Camera::getMVPInverse(){
    return glm::inverse(view * model);
}
