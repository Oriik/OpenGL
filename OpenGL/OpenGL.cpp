// OpenGL.cpp : définit le point d'entrée pour l'application console.
//

#include "glad/include/glad/glad.h"
#include <GLFW/glfw3.h>
#include <glm/vec3.hpp>
#include "stl.h"
#include <vector>
#include <iostream>
#include <random>
#include <sstream>
#include <fstream>
#include <string>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#define TINYPLY_IMPLEMENTATION
#include "tinyply.h"

static void error_callback(int /*error*/, const char* description)
{
	std::cerr << "Error: " << description << std::endl;
}

static void key_callback(GLFWwindow* window, int key, int /*scancode*/, int action, int /*mods*/)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GLFW_TRUE);
}

/* PARTICULES */
struct Particule {
	glm::vec3 position;
	glm::vec3 color;
	glm::vec3 speed;
};

std::vector<Particule> MakeParticules(const int n)
{
	std::default_random_engine generator;
	std::uniform_real_distribution<float> distribution01(0, 1);
	std::uniform_real_distribution<float> distributionWorld(-1, 1);

	std::vector<Particule> p;
	p.reserve(n);

	for (int i = 0; i < n; i++)
	{
		p.push_back(Particule{
			{
				distributionWorld(generator),
				distributionWorld(generator),
				distributionWorld(generator)
			},
					 {
						 distribution01(generator),
						 distribution01(generator),
						 distribution01(generator)
					 },
			{ 0.f, 0.f, 0.f }
			});
	}

	return p;
}

GLuint MakeShader(GLuint t, std::string path)
{
	std::cout << path << std::endl;
	std::ifstream file(path.c_str(), std::ios::in);
	std::ostringstream contents;
	contents << file.rdbuf();
	file.close();

	const auto content = contents.str();
	std::cout << content << std::endl;

	const auto s = glCreateShader(t);

	GLint sizes[] = { (GLint)content.size() };
	const auto data = content.data();

	glShaderSource(s, 1, &data, sizes);
	glCompileShader(s);

	GLint success;
	glGetShaderiv(s, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		GLchar infoLog[512];
		GLsizei l;
		glGetShaderInfoLog(s, 512, &l, infoLog);

		std::cout << infoLog << std::endl;
	}

	return s;
}

GLuint AttachAndLink(std::vector<GLuint> shaders)
{
	const auto prg = glCreateProgram();
	for (const auto s : shaders)
	{
		glAttachShader(prg, s);
	}

	glLinkProgram(prg);

	GLint success;
	glGetProgramiv(prg, GL_LINK_STATUS, &success);
	if (!success)
	{
		GLchar infoLog[512];
		GLsizei l;
		glGetProgramInfoLog(prg, 512, &l, infoLog);

		std::cout << infoLog << std::endl;
	}

	return prg;
}

int main(void)
{
	std::default_random_engine generator;
	std::uniform_real_distribution<float> distributionWorld(-0.01f, 0.01f);

	GLFWwindow* window;
	glfwSetErrorCallback(error_callback);

	if (!glfwInit())
		exit(EXIT_FAILURE);

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);

	window = glfwCreateWindow(640, 480, "Simple example", NULL, NULL);

	if (!window)
	{
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	glfwSetKeyCallback(window, key_callback);
	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);
	// NOTE: OpenGL error checks have been omitted for brevity

	if (!gladLoadGL()) {
		std::cerr << "Something went wrong!" << std::endl;
		exit(-1);
	}

	const size_t nParticules = 1000;
	auto particules = MakeParticules(nParticules);

	/*auto stlArray = ReadStl("logo.stl");
	auto nbTriangles = stlArray.size();*/

	//IMPORT DRAGON
	std::ifstream ss("dragon.ply", std::ios::binary);

	tinyply::PlyFile file;
	file.parse_header(ss);

	auto vertices = file.request_properties_from_element("vertex", { "x", "y", "z", "nx", "ny", "nz", "s", "t" });
	auto faces = file.request_properties_from_element("face", { "vertex_indices" }, 3);

	file.read(ss);

	// Shader particules
	const auto vertex = MakeShader(GL_VERTEX_SHADER, "shader.vert");
	const auto fragment = MakeShader(GL_FRAGMENT_SHADER, "shader.frag");

	// Shader triangles
	const auto vertexTriangle = MakeShader(GL_VERTEX_SHADER, "shaderTriangle.vert");
	const auto fragmentTriangle = MakeShader(GL_FRAGMENT_SHADER, "shaderTriangle.frag");

	const auto program = AttachAndLink({ vertex, fragment });
	const auto programTriangle = AttachAndLink({ vertexTriangle, fragmentTriangle });


	//PARTICULES
	
	glUseProgram(program);

	GLuint vbo, vao;
	glGenBuffers(1, &vbo);
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, nParticules * sizeof(Particule), particules.data(), GL_STATIC_DRAW); //PARTICULES
	
	// Bindings
	const auto indexPosition = glGetAttribLocation(program, "position");
	glVertexAttribPointer(indexPosition, 3, GL_FLOAT, GL_FALSE, sizeof(Particule), nullptr);
	glEnableVertexAttribArray(indexPosition);

	const auto indexColor = glGetAttribLocation(program, "colorParticule");
	glVertexAttribPointer(indexColor, 3, GL_FLOAT, GL_FALSE, sizeof(Particule), (const void*)sizeof(glm::vec3));
	glEnableVertexAttribArray(indexColor);

	glPointSize(0.5f);

	// TRIANGLES

	glUseProgram(programTriangle);

	GLuint vboTriangle, vaoTriangle, vbi;
	glGenBuffers(1, &vboTriangle);
	glGenBuffers(1, &vbi);
	glGenVertexArrays(1, &vaoTriangle);
	glBindVertexArray(vaoTriangle);
	glBindBuffer(GL_ARRAY_BUFFER, vboTriangle);
	glBufferData(GL_ARRAY_BUFFER,vertices->count * 32, vertices->buffer.get(), GL_STATIC_DRAW);  //TRIANGLES
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbi);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, faces->count * 3 * 4, faces->buffer.get(), GL_STATIC_DRAW);  //TRIANGLES

	// Bindings
	const auto indexPositionTriangle = glGetAttribLocation(programTriangle, "position");
	glVertexAttribPointer(indexPositionTriangle, 3, GL_FLOAT, GL_FALSE, 32, nullptr);
	glEnableVertexAttribArray(indexPositionTriangle);

	auto rotationFactor = 1.0f;
	auto scaleFactor = 0.01f;
	bool isGrowing = true;
	auto rotationVector = glm::vec3(1.0f, 1.0f, 1.0f);
	glm::mat4 trans = glm::mat4(1.0f);
	trans = glm::rotate(trans, glm::radians(rotationFactor), rotationVector);
	trans = glm::scale(trans, glm::vec3(scaleFactor, scaleFactor, scaleFactor));
	GLint uniTransfoTris = glGetUniformLocation(programTriangle, "trans");
	glProgramUniformMatrix4fv(programTriangle, uniTransfoTris, 1, GL_FALSE, glm::value_ptr(trans));

	glPointSize(2.f);


	while (!glfwWindowShouldClose(window))
	{
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);

		glViewport(0, 0, width, height);

		glClear(GL_COLOR_BUFFER_BIT);
		//glClearColor(1.0f, 0.0f, 0.0f, 1.0f);

		glUseProgram(program);
		glBindVertexArray(vao);
		glDrawArrays(GL_POINTS, 0, nParticules);

		glUseProgram(programTriangle);
		glBindVertexArray(vaoTriangle);
		glDrawElements(GL_TRIANGLES, faces->count * 3, GL_UNSIGNED_INT, 0);

		glfwSwapBuffers(window);
		glfwPollEvents();

		for ( int i = 0; i < nParticules; i++) {
			particules.at(i).position.z += distributionWorld(generator);
			particules.at(i).position.y += distributionWorld(generator);
			particules.at(i).position.x += distributionWorld(generator);
			if (particules.at(i).position.y < -1) particules.at(i).position.y = -particules.at(i).position.y;
			else if (particules.at(i).position.y > 1) particules.at(i).position.y = -particules.at(i).position.y;
			if (particules.at(i).position.x < -1) particules.at(i).position.x = -particules.at(i).position.x;
			else if (particules.at(i).position.x > 1) particules.at(i).position.x = -particules.at(i).position.x;
		}

		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferSubData(GL_ARRAY_BUFFER,0, nParticules * sizeof(Particule), particules.data());

		
		rotationFactor += 1.0f;
		if (isGrowing) scaleFactor+=0.0001f;
		else scaleFactor-=0.0001f;
		glm::mat4 trans = glm::mat4(1.0f);
		trans = glm::rotate(trans, glm::radians(rotationFactor), rotationVector);
		trans = glm::scale(trans, glm::vec3(0.01f, scaleFactor, scaleFactor));
		GLint uniTransfoTris = glGetUniformLocation(programTriangle, "trans");
		glProgramUniformMatrix4fv(programTriangle, uniTransfoTris, 1, GL_FALSE, glm::value_ptr(trans));
		
		if (scaleFactor > 0.03f) isGrowing = false;
		if (scaleFactor < 0.01f) isGrowing = true;
		
	}
	glfwDestroyWindow(window);
	glfwTerminate();
	exit(EXIT_SUCCESS);
}