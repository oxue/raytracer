#include "A2.hpp"
#include "cs488-framework/GlErrorCheck.hpp"

#include <iostream>
using namespace std;

#include <imgui/imgui.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/io.hpp>
using namespace glm;

//----------------------------------------------------------------------------------------
// Constructor
VertexData::VertexData()
	: numVertices(0),
	  index(0)
{
	positions.resize(kMaxVertices);
	colours.resize(kMaxVertices);
}


//----------------------------------------------------------------------------------------
// Constructor
A2::A2()
	: m_currentLineColour(vec3(0.0f)),
	cube_gnomon_origin(vec4(0.0f,0.0f,0.0f,1.0f)),
	cube_gnomon_x(vec4(0.1f,0.0f,0.0f,1.0f)),
	cube_gnomon_y(vec4(0.0f,0.1f,0.0f,1.0f)),
	cube_gnomon_z(vec4(0.0f,0.0f,0.1f,1.0f))
{
	projectionMatrix = getPerspective(fov, 1.0f, near, far);
	
	cubeTranslation = translate(glm::mat4(1.0f), glm::vec3(1.0f,1.5f,1.0f));
	cubeScale = glm::mat4(1.0f);
	cubeRotation = glm::mat4(1.0f);

	eyeTranslation = translate(glm::mat4(1.0f), glm::vec3(4,3,3));
	eyeRotation = rotate(glm::mat4(1.0f), -30.0f, glm::vec3(1.0f, 0.0f, 0.0f));
	eyeRotation = rotate(eyeRotation, 55.0f, glm::vec3(0.0f, 1.0f, 0.0f));
}

//----------------------------------------------------------------------------------------
// Destructor
A2::~A2()
{

}

glm::mat4 A2::translate(glm::mat4 m, glm::vec3 offset)
{
	float data[16] = {
		1,0,0,offset.x,
		0,1,0,offset.y,
		0,0,1,offset.z,
		0,0,0,1
	};
	return glm::transpose(glm::make_mat4(data)) * m;
}

glm::mat4 A2::scale(glm::mat4 m, glm::vec3 factors)
{
	float data[16] = {
		factors.x,0,        0,        0,
		0,        factors.y,0,        0,
		0,        0,        factors.z,0,
		0,        0,        0,        1
	};
	return glm::transpose(glm::make_mat4(data)) * m;
}

glm::mat4 A2::rotate(glm::mat4 m, float angle, glm::vec3 axis)
{
	float theta = glm::radians(angle);
	float c = glm::cos(theta);
	float s = glm::sin(theta);

	axis = glm::normalize(axis);

	float ux = axis.x;
	float uy = axis.y;
	float uz = axis.z;

	float data[16] = {
		c+ux*ux*(1-c),    ux*uy*(1-c)-uz*s, ux*uz*(1-c)+uy*s, 0,
		uy*ux*(1-c)+uz*s, c+uy*uy*(1-c),    uy*uz*(1-c)-ux*s, 0,
		uz*ux*(1-c)-uy*s, uz*uy*(1-c)+ux*s, c+uz*uz*(1-c),    0,
		0,                0,                0,                1
	};
	return glm::transpose(glm::make_mat4(data)) * m;
}

glm::mat4 A2::getPerspective(float fov, float ratio, float n, float f)
{
	float theta = glm::radians(fov);
	float data[16] = {
		1/(ratio * glm::tan(theta/2)), 0,                   0,            0,
		0,                             1/glm::tan(theta/2), 0,            0,
		0,                             0,                   -(f+n)/(f-n), -2*f*n/(f-n),
		0,                             0,                   -1,           0
	};
	return glm::transpose(glm::make_mat4(data));
}



//----------------------------------------------------------------------------------------
/*
 * Called once, at program start.
 */
void A2::init()
{
	// Set the background colour.
	glClearColor(0.3, 0.5, 0.7, 1.0);

	createShaderProgram();

	glGenVertexArrays(1, &m_vao);

	enableVertexAttribIndices();

	generateVertexBuffers();

	mapVboDataToVertexAttributeLocation();
}

//----------------------------------------------------------------------------------------
void A2::createShaderProgram()
{
	m_shader.generateProgramObject();
	m_shader.attachVertexShader( getAssetFilePath("VertexShader.vs").c_str() );
	m_shader.attachFragmentShader( getAssetFilePath("FragmentShader.fs").c_str() );
	m_shader.link();
}

//----------------------------------------------------------------------------------------
void A2::enableVertexAttribIndices()
{
	glBindVertexArray(m_vao);

	// Enable the attribute index location for "position" when rendering.
	GLint positionAttribLocation = m_shader.getAttribLocation( "position" );
	glEnableVertexAttribArray(positionAttribLocation);

	// Enable the attribute index location for "colour" when rendering.
	GLint colourAttribLocation = m_shader.getAttribLocation( "colour" );
	glEnableVertexAttribArray(colourAttribLocation);

	// Restore defaults
	glBindVertexArray(0);

	CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
void A2::generateVertexBuffers()
{
	// Generate a vertex buffer to store line vertex positions
	{
		glGenBuffers(1, &m_vbo_positions);

		glBindBuffer(GL_ARRAY_BUFFER, m_vbo_positions);

		// Set to GL_DYNAMIC_DRAW because the data store will be modified frequently.
		glBufferData(GL_ARRAY_BUFFER, sizeof(vec2) * kMaxVertices, nullptr,
				GL_DYNAMIC_DRAW);


		// Unbind the target GL_ARRAY_BUFFER, now that we are finished using it.
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		CHECK_GL_ERRORS;
	}

	// Generate a vertex buffer to store line colors
	{
		glGenBuffers(1, &m_vbo_colours);

		glBindBuffer(GL_ARRAY_BUFFER, m_vbo_colours);

		// Set to GL_DYNAMIC_DRAW because the data store will be modified frequently.
		glBufferData(GL_ARRAY_BUFFER, sizeof(vec3) * kMaxVertices, nullptr,
				GL_DYNAMIC_DRAW);


		// Unbind the target GL_ARRAY_BUFFER, now that we are finished using it.
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		CHECK_GL_ERRORS;
	}
}

//----------------------------------------------------------------------------------------
void A2::mapVboDataToVertexAttributeLocation()
{
	// Bind VAO in order to record the data mapping.
	glBindVertexArray(m_vao);

	// Tell GL how to map data from the vertex buffer "m_vbo_positions" into the
	// "position" vertex attribute index for any bound shader program.
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo_positions);
	GLint positionAttribLocation = m_shader.getAttribLocation( "position" );
	glVertexAttribPointer(positionAttribLocation, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

	// Tell GL how to map data from the vertex buffer "m_vbo_colours" into the
	// "colour" vertex attribute index for any bound shader program.
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo_colours);
	GLint colorAttribLocation = m_shader.getAttribLocation( "colour" );
	glVertexAttribPointer(colorAttribLocation, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

	//-- Unbind target, and restore default values:
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	CHECK_GL_ERRORS;
}

//---------------------------------------------------------------------------------------
void A2::initLineData()
{
	m_vertexData.numVertices = 0;
	m_vertexData.index = 0;
}

//---------------------------------------------------------------------------------------
void A2::setLineColour (
		const glm::vec3 & colour
) {
	m_currentLineColour = colour;
}

//---------------------------------------------------------------------------------------
void A2::drawLine(
		const glm::vec2 & v0,   // Line Start (NDC coordinate)
		const glm::vec2 & v1    // Line End (NDC coordinate)
) {

	m_vertexData.positions[m_vertexData.index] = v0;
	m_vertexData.colours[m_vertexData.index] = m_currentLineColour;
	++m_vertexData.index;
	m_vertexData.positions[m_vertexData.index] = v1;
	m_vertexData.colours[m_vertexData.index] = m_currentLineColour;
	++m_vertexData.index;

	m_vertexData.numVertices += 2;
}

void A2::pipeLine(
		const glm::vec4 & v0,
		const glm::vec4 & v1,
		glm::mat4 modelMatrix)
{
	drawLine(pipelineTransform(v0, modelMatrix), pipelineTransform(v1, modelMatrix));
}

glm::vec2 A2::pipelineTransform(const glm::vec4 & v, glm::mat4 modelMatrix) {
	glm::mat4 viewMatrix = glm::inverse(tempEyeTranslation * tempEyeRotation);
	glm::vec4 result = projectionMatrix * viewMatrix * modelMatrix * v;
	return glm::vec2(result.x/result.w, result.y/result.w);
}

void A2::upFunc0(glm::vec4 axis){
	float diff = (mouseX - recordedX) / 768.0f;
	glm::vec4 hand = eyeRotation * axis;
	glm::vec4 modHand = cubeRotation * axis;
	switch(currentMode) {
		case modeRV:
			eyeRotation = rotate(eyeRotation, diff * 180.0f, glm::vec3(hand.x, hand.y, hand.z));
		break;
		case modeTV:
			eyeTranslation = translate(eyeTranslation, diff * 2 * glm::vec3(hand.x, hand.y, hand.z));
		break;
		case modeP:
			if(axis.x == 1.0f){
				float newfov = fov + 200.0f * diff;
				if(newfov > 160.0f) newfov = 160.0f;
				if(newfov < 5.0f) newfov = 5.0f;
				fov = newfov;
			}
			if(axis.y == 1.0f){
				near = near + diff * 10;
			}
			if(axis.z == 1.0f){
				far = far + diff * 10;
			}
		break;
		case modeRM:
			cubeRotation = rotate(cubeRotation, diff * 180.0f, glm::vec3(modHand.x, modHand.y, modHand.z));
		break;
		case modeTM:
			cubeTranslation = translate(cubeTranslation, diff * 2 * glm::vec3(modHand.x, modHand.y, modHand.z));
		break;
		case modeSM:
			cubeScale = scale(cubeScale, (diff * glm::vec3(axis.x, axis.y, axis.z)) + glm::vec3(1.0f));
		break;
		case modeVP:

		break;
	}
}

void A2::updateFunc0(glm::vec4 axis) {
	float diff = (mouseX - recordedX) / 768.0f;
	glm::vec4 hand = eyeRotation * axis;
	glm::vec4 modHand = cubeRotation * axis;
	switch(currentMode) {
		case modeRV:
			tempEyeRotation = rotate(eyeRotation, diff * 180.0f, glm::vec3(hand.x, hand.y, hand.z));
		break;
		case modeTV:
			tempEyeTranslation = translate(eyeTranslation, diff * 2 * glm::vec3(hand.x, hand.y, hand.z));
		break;
		case modeP:
			if(axis.x == 1.0f) {
				float newfov = fov + 200.0f * diff;
				if(newfov > 160.0f) newfov = 160.0f;
				if(newfov < 5.0f) newfov = 5.0f;
				projectionMatrix = getPerspective(newfov, 1.0f, near, far);
			}
			if(axis.y == 1.0f) {
				projectionMatrix = getPerspective(fov, 1.0f, near + diff * 10, far);
			}
			if(axis.z == 1.0f) {
				projectionMatrix = getPerspective(fov, 1.0f, near, far + diff * 10);
			}
		break;
		case modeRM:
			tempCubeRotation = rotate(cubeRotation, diff * 180.0f, glm::vec3(modHand.x, modHand.y, modHand.z));
		break;
		case modeTM:
			tempCubeTranslation = translate(cubeTranslation, diff * 2 * glm::vec3(modHand.x, modHand.y, modHand.z));
		break;
		case modeSM:
			tempCubeScale = scale(cubeScale, (diff * glm::vec3(axis.x, axis.y, axis.z)) + glm::vec3(1.0f));
		break;
	}
}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, before guiLogic().
 */
void A2::appLogic()
{
	// Place per frame, application logic here ...

	// Call at the beginning of frame, before drawing lines:
	initLineData();

	// // Draw outer square:
	// setLineColour(vec3(1.0f, 0.7f, 0.8f));
	// drawLine(vec2(-0.5f, -0.5f), vec2(0.5f, -0.5f));
	// drawLine(vec2(0.5f, -0.5f), vec2(0.5f, 0.5f));
	// drawLine(vec2(0.5f, 0.5f), vec2(-0.5f, 0.5f));
	// drawLine(vec2(-0.5f, 0.5f), vec2(-0.5f, -0.5f));

	tempEyeTranslation = eyeTranslation;
	tempEyeRotation = eyeRotation;

	tempCubeTranslation = cubeTranslation;
	tempCubeRotation = cubeRotation;
	tempCubeScale = cubeScale;

	if(leftMouseDown) {
		updateFunc0(glm::vec4(1.0f, 0.0f, 0.0f, 1.0));
	}

	if(rightMouseDown) {
		updateFunc0(glm::vec4(0.0f, 1.0f, 0.0f, 1.0));
	}

	if(middleMouseDown) {
		updateFunc0(glm::vec4(0.0f, 0.0f, 1.0f, 1.0));
	}

	// // Draw inner square:
	// setLineColour(vec3(0.2f, 1.0f, 1.0f));
	// drawLine(vec2(-0.25f, -0.25f), vec2(0.25f, -0.25f));
	// drawLine(vec2(0.25f, -0.25f), vec2(0.25f, 0.25f));
	// drawLine(vec2(0.25f, 0.25f), vec2(-0.25f, 0.25f));
	// drawLine(vec2(-0.25f, 0.25f), vec2(-0.25f, -0.25f));
	setLineColour(vec3(0.2f, 0.2f, 1.0f));
	pipeLine(glm::vec4(0,0,0,1), glm::vec4(.1f, 0.0f, 0.0f,1));
	setLineColour(vec3(1.0f, 0.2f, 0.2f));
	pipeLine(glm::vec4(0,0,0,1), glm::vec4(0.0f, .1f, 0.0f,1));
	setLineColour(vec3(0.2f, 1.0f, 0.2f));
	pipeLine(glm::vec4(0,0,0,1), glm::vec4(0.0f, 0.0f, .1f,1));

	glm::mat4 cubeGnomonModelMatrix = tempCubeTranslation * tempCubeRotation;
	setLineColour(vec3(0.2f, 1.0f, 1.0f));
	pipeLine(cube_gnomon_origin, cube_gnomon_x, cubeGnomonModelMatrix);
	setLineColour(vec3(1.0f, 0.2f, 1.0f));
	pipeLine(cube_gnomon_origin, cube_gnomon_y, cubeGnomonModelMatrix);
	setLineColour(vec3(1.0f, 1.0f, 0.2f));
	pipeLine(cube_gnomon_origin, cube_gnomon_z, cubeGnomonModelMatrix);

	//draw cube
	glm::mat4 cubeModelMatrix = tempCubeTranslation * tempCubeRotation * tempCubeScale;
	setLineColour(vec3(1.0f, 1.0f, 1.0f));
	pipeLine(vec4(1.0f, 1.0f, 1.0f, 1.0f),vec4(-1.0f, 1.0f, 1.0f, 1.0f), cubeModelMatrix);
	pipeLine(vec4(1.0f, 1.0f, 1.0f, 1.0f),vec4(1.0f, 1.0f, -1.0f, 1.0f), cubeModelMatrix);
	pipeLine(vec4(-1.0f, 1.0f, 1.0f, 1.0f),vec4(-1.0f, 1.0f, -1.0f, 1.0f), cubeModelMatrix);
	pipeLine(vec4(-1.0f, 1.0f, -1.0f, 1.0f),vec4(1.0f, 1.0f, -1.0f, 1.0f), cubeModelMatrix);

	pipeLine(vec4(1.0f, -1.0f, 1.0f, 1.0f),vec4(-1.0f, -1.0f, 1.0f, 1.0f), cubeModelMatrix);
	pipeLine(vec4(1.0f, -1.0f, 1.0f, 1.0f),vec4(1.0f, -1.0f, -1.0f, 1.0f), cubeModelMatrix);
	pipeLine(vec4(-1.0f, -1.0f, 1.0f, 1.0f),vec4(-1.0f, -1.0f, -1.0f, 1.0f), cubeModelMatrix);
	pipeLine(vec4(-1.0f, -1.0f, -1.0f, 1.0f),vec4(1.0f, -1.0f, -1.0f, 1.0f), cubeModelMatrix);

	pipeLine(vec4(1.0f, 1.0f, 1.0f, 1.0f),vec4(1.0f, -1.0f, 1.0f, 1.0f), cubeModelMatrix);
	pipeLine(vec4(1.0f, 1.0f, -1.0f, 1.0f),vec4(1.0f, -1.0f, -1.0f, 1.0f), cubeModelMatrix);
	pipeLine(vec4(-1.0f, 1.0f, -1.0f, 1.0f),vec4(-1.0f, -1.0f, -1.0f, 1.0f), cubeModelMatrix);
	pipeLine(vec4(-1.0f, 1.0f, 1.0f, 1.0f),vec4(-1.0f, -1.0f, 1.0f, 1.0f), cubeModelMatrix);
}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, after appLogic(), but before the draw() method.
 */
void A2::guiLogic()
{
	static bool firstRun(true);
	if (firstRun) {
		ImGui::SetNextWindowPos(ImVec2(50, 50));
		firstRun = false;
	}

	static bool showDebugWindow(true);
	ImGuiWindowFlags windowFlags(ImGuiWindowFlags_AlwaysAutoResize);
	float opacity(0.5f);

	ImGui::Begin("Properties", &showDebugWindow, ImVec2(100,100), opacity,
			windowFlags);

		ImGui::PushID( 0 );
		if( ImGui::RadioButton( "RV", &currentMode, 0 ) ) {
			
		}
		ImGui::PopID();	
		ImGui::PushID( 0 );
		if( ImGui::RadioButton( "TV", &currentMode, 1 ) ) {
			
		}
		ImGui::PopID();	
		ImGui::PushID( 0 );
		if( ImGui::RadioButton( "P", &currentMode, 2 ) ) {
			
		}
		ImGui::PopID();	
		ImGui::PushID( 0 );
		if( ImGui::RadioButton( "RM", &currentMode, 3 ) ) {
			
		}
		ImGui::PopID();	
		ImGui::PushID( 0 );
		if( ImGui::RadioButton( "TM", &currentMode, 4 ) ) {
			
		}
		ImGui::PopID();	
		ImGui::PushID( 0 );
		if( ImGui::RadioButton( "SM", &currentMode, 5 ) ) {
			
		}
		ImGui::PopID();	
		ImGui::PushID( 0 );
		if( ImGui::RadioButton( "VP", &currentMode, 6 ) ) {
			
		}
		ImGui::PopID();	


		ImGui::Text("near: %f", near);
		ImGui::Text("far: %f", far);


		// Add more gui elements here here ...
		if( ImGui::Button( "Reset" ) ) {
			reset();
		}

		// Create Button, and check if it was clicked:
		if( ImGui::Button( "Quit Application" ) ) {
			glfwSetWindowShouldClose(m_window, GL_TRUE);
		}

		ImGui::Text( "Framerate: %.1f FPS", ImGui::GetIO().Framerate );

	ImGui::End();
}

//----------------------------------------------------------------------------------------
void A2::uploadVertexDataToVbos() {

	//-- Copy vertex position data into VBO, m_vbo_positions:
	{
		glBindBuffer(GL_ARRAY_BUFFER, m_vbo_positions);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vec2) * m_vertexData.numVertices,
				m_vertexData.positions.data());
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		CHECK_GL_ERRORS;
	}

	//-- Copy vertex colour data into VBO, m_vbo_colours:
	{
		glBindBuffer(GL_ARRAY_BUFFER, m_vbo_colours);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vec3) * m_vertexData.numVertices,
				m_vertexData.colours.data());
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		CHECK_GL_ERRORS;
	}
}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, after guiLogic().
 */
void A2::draw()
{
	uploadVertexDataToVbos();

	glBindVertexArray(m_vao);

	m_shader.enable();
		glDrawArrays(GL_LINES, 0, m_vertexData.numVertices);
	m_shader.disable();

	// Restore defaults
	glBindVertexArray(0);

	CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
/*
 * Called once, after program is signaled to terminate.
 */
void A2::cleanup()
{

}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles cursor entering the window area events.
 */
bool A2::cursorEnterWindowEvent (
		int entered
) {
	bool eventHandled(false);

	// Fill in with event handling code...

	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles mouse cursor movement events.
 */
bool A2::mouseMoveEvent(double xPos, double yPos) 
{
	bool eventHandled(false);

	if (!ImGui::IsMouseHoveringAnyWindow()) {
		// Put some code here to handle rotations.  Probably need to
		// check whether we're *dragging*, not just moving the mouse.
		// Probably need some instance variables to track the current
		// rotation amount, and maybe the previous X position (so 
		// that you can rotate relative to the *change* in X.
		mouseX = xPos;
		mouseY = yPos;
	}

	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles mouse button events.
 */
bool A2::mouseButtonInputEvent (
		int button,
		int actions,
		int mods
) {
	bool eventHandled(false);

	if (!ImGui::IsMouseHoveringAnyWindow()) {
		if(actions == 1) {
			if(button == 0){
				leftMouseDown = true;
			}
			if(button == 1){
				rightMouseDown = true;
			}
			if(button == 2){
				middleMouseDown = true;
			}
			recordedX = mouseX;
			recordedY = mouseY;
		}
		if(actions == 0){
			if(button == 0){
				leftMouseDown = false;
				upFunc0(glm::vec4(1.0f, 0.0f,0.0f,1.0f));
			}
			if(button == 1){
				rightMouseDown = false;	
				upFunc0(glm::vec4(0.0f, 1.0f,0.0f,1.0f));
			}
			if(button == 2){
				middleMouseDown = false;
				upFunc0(glm::vec4(0.0f, 0.0f,1.0f,1.0f));
			}
		}
		cout<<button<<endl;
	}

	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles mouse scroll wheel events.
 */
bool A2::mouseScrollEvent (
		double xOffSet,
		double yOffSet
) {
	bool eventHandled(false);

	// Fill in with event handling code...

	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles window resize events.
 */
bool A2::windowResizeEvent (
		int width,
		int height
) {
	bool eventHandled(false);

	// Fill in with event handling code...

	return eventHandled;
}

void A2::reset() {
	fov = 45.0f;
	near = 0.1f;
	far = 5.0f;

	currentMode = 3;

	projectionMatrix = getPerspective(fov, 1.0f, near, far);
	
	cubeTranslation = translate(glm::mat4(1.0f), glm::vec3(1.0f,1.5f,1.0f));
	cubeScale = glm::mat4(1.0f);
	cubeRotation = glm::mat4(1.0f);

	eyeTranslation = translate(glm::mat4(1.0f), glm::vec3(4,3,3));
	eyeRotation = rotate(glm::mat4(1.0f), -30.0f, glm::vec3(1.0f, 0.0f, 0.0f));
	eyeRotation = rotate(eyeRotation, 55.0f, glm::vec3(0.0f, 1.0f, 0.0f));
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles key input events.
 */
bool A2::keyInputEvent (
		int key,
		int action,
		int mods
) {
	bool eventHandled(false);

	// Fill in with event handling code...

	// Fill in with event handling code...
	if( action == GLFW_PRESS ) {
		// Respond to some key events.
		if (key == GLFW_KEY_A) {
			reset();
		}
		if (key == GLFW_KEY_Q) {
			glfwSetWindowShouldClose(m_window, GL_TRUE);
		}

		if(key == GLFW_KEY_O) {
			cubeRotation = rotate(cubeRotation, -10.f, glm::vec3(0.f, 1.05f, 0.f));
		}
		if(key == GLFW_KEY_I) {
			cubeRotation = rotate(cubeRotation, 10.f, glm::vec3(0, 0.95, 0));
		}

		if(key == GLFW_KEY_LEFT_SHIFT) {
			recordedX = mouseX;
			middleMouseDown = true;
		}
	}

	if(action == GLFW_RELEASE) {
		if(key == GLFW_KEY_LEFT_SHIFT) {
			middleMouseDown = false;
			upFunc0(glm::vec4(0.0f, 0.0f,1.0f,1.0f));
		}
		
	}

	return eventHandled;
}





