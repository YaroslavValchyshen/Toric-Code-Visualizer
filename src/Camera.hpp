#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <GLFW/glfw3.h>

class Camera
{
private:
	unsigned int shaderProgram;

	glm::mat4 model;

	glm::vec2 moveDirection = glm::vec2(0.0f);
	float moveSpeed;

	void readInput(GLFWwindow* window);
public: 
	Camera(unsigned int shaderProgram, float moveSpeed);
	void update(GLFWwindow* window);
};