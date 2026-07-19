#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <GLFW/glfw3.h>

class Camera
{
private:
	unsigned int shaderProgram;

	glm::mat4 model;
	glm::mat4 view;
	glm::mat4 projection; 

	glm::vec2 moveDirection = glm::vec2(0.0f);
	float moveSpeed;

	void readInput(GLFWwindow* window);
public: 
	Camera(unsigned int shaderProgram, float moveSpeed, float windowWidth, float windowHeight);
	void update(GLFWwindow* window);
	glm::mat4 getViewMatrix();
	glm::mat4 getProjectionMatrix();
	glm::mat4 getMVPInverse();
};