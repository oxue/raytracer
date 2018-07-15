#pragma once

#include "cs488-framework/CS488Window.hpp"
#include "cs488-framework/OpenGLImport.hpp"
#include "cs488-framework/ShaderProgram.hpp"

#include <glm/glm.hpp>

#include <vector>

// Set a global maximum number of vertices in order to pre-allocate VBO data
// in one shot, rather than reallocating each frame.
const GLsizei kMaxVertices = 1000;


// Convenience class for storing vertex data in CPU memory.
// Data should be copied over to GPU memory via VBO storage before rendering.
class VertexData {
public:
	VertexData();

	std::vector<glm::vec2> positions;
	std::vector<glm::vec3> colours;
	GLuint index;
	GLsizei numVertices;
};


class A2 : public CS488Window {
public:
	A2();
	virtual ~A2();

protected:
	virtual void init() override;
	virtual void appLogic() override;
	virtual void guiLogic() override;
	virtual void draw() override;
	virtual void cleanup() override;

	virtual bool cursorEnterWindowEvent(int entered) override;
	virtual bool mouseMoveEvent(double xPos, double yPos) override;
	virtual bool mouseButtonInputEvent(int button, int actions, int mods) override;
	virtual bool mouseScrollEvent(double xOffSet, double yOffSet) override;
	virtual bool windowResizeEvent(int width, int height) override;
	virtual bool keyInputEvent(int key, int action, int mods) override;

	void createShaderProgram();
	void enableVertexAttribIndices();
	void generateVertexBuffers();
	void mapVboDataToVertexAttributeLocation();
	void uploadVertexDataToVbos();

	void initLineData();

	void setLineColour(const glm::vec3 & colour);

	void drawLine (
			const glm::vec2 & v0,
			const glm::vec2 & v1
	);

	void pipeLine (
			const glm::vec4 & v0,
			const glm::vec4 & v1,
			glm::mat4 modelMatrix = glm::mat4(1.0f)
	);

	void reset();
	void updateFunc0(glm::vec4 axis);
	void updateFunc1();
	void updateFunc2();

	void upFunc0(glm::vec4 axis);
	void upFunc1();
	void upFunc2();

	glm::mat4 rotate(glm::mat4 m, float a, glm::vec3 axis);
	glm::mat4 translate(glm::mat4 m, glm::vec3 offset);
	glm::mat4 scale(glm::mat4 m, glm::vec3 factors);

	glm::mat4 getPerspective(float fov, float ratio, float near, float far);

	glm::vec2 pipelineTransform (const glm::vec4 & v, glm::mat4 modelMatrix);

	ShaderProgram m_shader;

	GLuint m_vao;            // Vertex Array Object
	GLuint m_vbo_positions;  // Vertex Buffer Object
	GLuint m_vbo_colours;    // Vertex Buffer Object

	VertexData m_vertexData;

	glm::vec3 m_currentLineColour;

	glm::mat4 projectionMatrix;

	glm::mat4 eyeTranslation;
	glm::mat4 eyeRotation;
	glm::mat4 tempEyeTranslation;
	glm::mat4 tempEyeRotation;

	glm::mat4 cubeTranslation;
	glm::mat4 cubeRotation;
	glm::mat4 cubeScale;

	glm::mat4 tempCubeTranslation;
	glm::mat4 tempCubeRotation;
	glm::mat4 tempCubeScale;

	glm::vec4 cube_gnomon_origin;
	glm::vec4 cube_gnomon_x;
	glm::vec4 cube_gnomon_y;
	glm::vec4 cube_gnomon_z;

	bool leftMouseDown = false;
	bool rightMouseDown = false;
	bool middleMouseDown = false;

	int currentMode = 3;

	static constexpr int modeRV = 0;
	static constexpr int modeTV = 1;
	static constexpr int modeP = 2;
	static constexpr int modeRM = 3;
	static constexpr int modeTM = 4;
	static constexpr int modeSM = 5;
	static constexpr int modeVP = 6;

	double mouseX = 0.0;
	double mouseY = 0.0;
	double recordedX = 0.0;
	double recordedY = 0.0;

	float fov = 45.0f;
	float near = 0.1f;
	float far = 5.0f;

	float viewTop = 768.0f;
	float viewRight = 768.0f;
	float viewLeft = 0.0f;
	float viewBottom = 0.0f;
};
