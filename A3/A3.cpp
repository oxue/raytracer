#include "A3.hpp"
#include "scene_lua.hpp"
using namespace std;

#include "cs488-framework/GlErrorCheck.hpp"
#include "cs488-framework/MathUtils.hpp"
#include "GeometryNode.hpp"
#include "JointNode.hpp"

#include <imgui/imgui.h>

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/io.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <chrono>
#include <string>

using namespace std::chrono;

using namespace glm;

static bool show_gui = true;

const size_t CIRCLE_PTS = 48;

//----------------------------------------------------------------------------------------
// Constructor
A3::A3(const std::string & luaSceneFile)
	: m_luaSceneFile(luaSceneFile),
	  m_positionAttribLocation(0),
	  m_normalAttribLocation(0),
	  m_vao_meshData(0),
	  m_vbo_vertexPositions(0),
	  m_vbo_vertexNormals(0),
	  m_vao_arcCircle(0),
	  m_vbo_arcCircle(0)
{
	puppetTranslation = glm::mat4(1.0);
	puppetRotation = glm::mat4(1.0);
}

std::vector<SceneNode*> A3::getAllSelected()
{
	std::vector<SceneNode*> ret;
	for (auto it = A3::nodeMap.begin(); it!= A3::nodeMap.end(); it++)
	{
		if(it->second->isSelected){
			ret.push_back(it->second);
		}
	}
	return ret;
}

//----------------------------------------------------------------------------------------
// Destructor
A3::~A3()
{

}

std::map<unsigned int, SceneNode*> A3::nodeMap = std::map<unsigned int, SceneNode*>();
std::map<std::string, SceneNode*> A3::nameMap = std::map<std::string, SceneNode*>();

//----------------------------------------------------------------------------------------
/*
 * Called once, at program start.
 */
void A3::init()
{
	// Set the background colour.
	glClearColor(0.0, 0.0, 0.0, 1.0);

	createShaderProgram();

	glGenVertexArrays(1, &m_vao_arcCircle);
	glGenVertexArrays(1, &m_vao_meshData);
	glGenVertexArrays(1, &m_vao_picking);
	enableVertexShaderInputSlots();

	processLuaSceneFile(m_luaSceneFile);

	// Load and decode all .obj files at once here.  You may add additional .obj files to
	// this list in order to support rendering additional mesh types.  All vertex
	// positions, and normals will be extracted and stored within the MeshConsolidator
	// class.
	unique_ptr<MeshConsolidator> meshConsolidator (new MeshConsolidator{
			getAssetFilePath("cube.obj"),
			getAssetFilePath("sphere.obj"),
			getAssetFilePath("suzanne.obj")
	});


	// Acquire the BatchInfoMap from the MeshConsolidator.
	meshConsolidator->getBatchInfoMap(m_batchInfoMap);

	// Take all vertex data within the MeshConsolidator and upload it to VBOs on the GPU.
	uploadVertexDataToVbos(*meshConsolidator);

	mapVboDataToVertexShaderInputLocations();

	initPerspectiveMatrix();

	initViewMatrix();

	initLightSources();

	initPicking();	

	puppetTranslation = m_rootNode->trans;

	initialTranslation = puppetTranslation;
	initialRotation = puppetRotation;

	for (auto it = nodeMap.begin(); it != nodeMap.end(); it++){
		it->second->initialTrans = it->second->trans;
	}


	// Exiting the current scope calls delete automatically on meshConsolidator freeing
	// all vertex data resources.  This is fine since we already copied this data to
	// VBOs on the GPU.  We have no use for storing vertex data on the CPU side beyond
	// this point.
}

unsigned int A3::getObjectIdAt(int x, int y)
{
	glBindFramebuffer(GL_READ_FRAMEBUFFER, m_fbo);
	glReadBuffer(GL_COLOR_ATTACHMENT0);

	float pixelData[3];
	glReadPixels(x, y, 1, 1, GL_RGB, GL_FLOAT, pixelData);

	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);

	return (unsigned int)(pixelData[0] * 255);
}

bool A3::initPicking() {
	glGenFramebuffers(1, &m_fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

	glGenTextures(1, &m_pickingTexture);
	glBindTexture(GL_TEXTURE_2D, m_pickingTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, m_windowWidth, m_windowHeight, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_pickingTexture, 0);

	glGenTextures(1, &m_depthTexture);
	glBindTexture(GL_TEXTURE_2D, m_depthTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, 1024, 768, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_depthTexture, 0); 

	//glReadBuffer(GL_NONE);

	glDrawBuffer(GL_COLOR_ATTACHMENT0);
	GLenum Status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (Status != GL_FRAMEBUFFER_COMPLETE) {
		printf("FB error, status: 0x%x\n", Status);
		return false;
	}
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	CHECK_GL_ERRORS;
	return true;
}

//----------------------------------------------------------------------------------------
void A3::processLuaSceneFile(const std::string & filename) {
	// This version of the code treats the Lua file as an Asset,
	// so that you'd launch the program with just the filename
	// of a puppet in the Assets/ directory.
	// std::string assetFilePath = getAssetFilePath(filename.c_str());
	// m_rootNode = std::shared_ptr<SceneNode>(import_lua(assetFilePath));

	// This version of the code treats the main program argument
	// as a straightforward pathname.
	m_rootNode = std::shared_ptr<SceneNode>(import_lua(filename));
	if (!m_rootNode) {
		std::cerr << "Could not open " << filename << std::endl;
	}
}

//----------------------------------------------------------------------------------------
void A3::createShaderProgram()
{
	m_shader.generateProgramObject();
	m_shader.attachVertexShader( getAssetFilePath("VertexShader.vs").c_str() );
	m_shader.attachFragmentShader( getAssetFilePath("FragmentShader.fs").c_str() );
	m_shader.link();

	m_shader_arcCircle.generateProgramObject();
	m_shader_arcCircle.attachVertexShader( getAssetFilePath("arc_VertexShader.vs").c_str() );
	m_shader_arcCircle.attachFragmentShader( getAssetFilePath("arc_FragmentShader.fs").c_str() );
	m_shader_arcCircle.link();

	m_shader_picking.generateProgramObject();
	m_shader_picking.attachVertexShader( getAssetFilePath("PickerVertexShader.vs").c_str() );
	m_shader_picking.attachFragmentShader( getAssetFilePath("PickerFragmentShader.fs").c_str() );
	m_shader_picking.link();
}

//----------------------------------------------------------------------------------------
void A3::enableVertexShaderInputSlots()
{
	//-- Enable input slots for m_vao_meshData:
	{
		glBindVertexArray(m_vao_meshData);

		// Enable the vertex shader attribute location for "position" when rendering.
		m_positionAttribLocation = m_shader.getAttribLocation("position");
		glEnableVertexAttribArray(m_positionAttribLocation);

		// Enable the vertex shader attribute location for "normal" when rendering.
		m_normalAttribLocation = m_shader.getAttribLocation("normal");
		glEnableVertexAttribArray(m_normalAttribLocation);

		CHECK_GL_ERRORS;
	}


	//-- Enable input slots for m_vao_arcCircle:
	{
		glBindVertexArray(m_vao_arcCircle);

		// Enable the vertex shader attribute location for "position" when rendering.
		m_arc_positionAttribLocation = m_shader_arcCircle.getAttribLocation("position");
		glEnableVertexAttribArray(m_arc_positionAttribLocation);

		CHECK_GL_ERRORS;
	}

	{
		glBindVertexArray(m_vao_picking);
		m_pickng_positionAttribLocation = m_shader_picking.getAttribLocation("position");
		glEnableVertexAttribArray(m_pickng_positionAttribLocation);

		CHECK_GL_ERRORS;
	}

	// Restore defaults
	glBindVertexArray(0);
}

//----------------------------------------------------------------------------------------
void A3::uploadVertexDataToVbos (
	const MeshConsolidator & meshConsolidator
) {
	// Generate VBO to store all vertex position data
	{
		glGenBuffers(1, &m_vbo_vertexPositions);

		glBindBuffer(GL_ARRAY_BUFFER, m_vbo_vertexPositions);

		glBufferData(GL_ARRAY_BUFFER, meshConsolidator.getNumVertexPositionBytes(),
				meshConsolidator.getVertexPositionDataPtr(), GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		CHECK_GL_ERRORS;
	}

	// Generate VBO to store all vertex normal data
	{
		glGenBuffers(1, &m_vbo_vertexNormals);

		glBindBuffer(GL_ARRAY_BUFFER, m_vbo_vertexNormals);

		glBufferData(GL_ARRAY_BUFFER, meshConsolidator.getNumVertexNormalBytes(),
				meshConsolidator.getVertexNormalDataPtr(), GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		CHECK_GL_ERRORS;
	}

	// Generate VBO to store the trackball circle.
	{
		glGenBuffers( 1, &m_vbo_arcCircle );
		glBindBuffer( GL_ARRAY_BUFFER, m_vbo_arcCircle );

		float *pts = new float[ 2 * CIRCLE_PTS ];
		for( size_t idx = 0; idx < CIRCLE_PTS; ++idx ) {
			float ang = 2.0 * M_PI * float(idx) / CIRCLE_PTS;
			pts[2*idx] = cos( ang );
			pts[2*idx+1] = sin( ang );
		}

		glBufferData(GL_ARRAY_BUFFER, 2*CIRCLE_PTS*sizeof(float), pts, GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		CHECK_GL_ERRORS;
	}
}

//----------------------------------------------------------------------------------------
void A3::mapVboDataToVertexShaderInputLocations()
{
	// Bind VAO in order to record the data mapping.
	glBindVertexArray(m_vao_meshData);

	// Tell GL how to map data from the vertex buffer "m_vbo_vertexPositions" into the
	// "position" vertex attribute location for any bound vertex shader program.
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo_vertexPositions);
	glVertexAttribPointer(m_positionAttribLocation, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

	// Tell GL how to map data from the vertex buffer "m_vbo_vertexNormals" into the
	// "normal" vertex attribute location for any bound vertex shader program.
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo_vertexNormals);
	glVertexAttribPointer(m_normalAttribLocation, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

	//-- Unbind target, and restore default values:
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	CHECK_GL_ERRORS;

	glBindVertexArray(m_vao_picking);
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo_vertexPositions);
	glVertexAttribPointer(m_pickng_positionAttribLocation, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	CHECK_GL_ERRORS;

	// Bind VAO in order to record the data mapping.
	glBindVertexArray(m_vao_arcCircle);

	// Tell GL how to map data from the vertex buffer "m_vbo_arcCircle" into the
	// "position" vertex attribute location for any bound vertex shader program.
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo_arcCircle);
	glVertexAttribPointer(m_arc_positionAttribLocation, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

	//-- Unbind target, and restore default values:
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
void A3::initPerspectiveMatrix()
{
	float aspect = ((float)m_windowWidth) / m_windowHeight;
	m_perpsective = glm::perspective(degreesToRadians(60.0f), aspect, 0.1f, 100.0f);
}


//----------------------------------------------------------------------------------------
void A3::initViewMatrix() {
	m_view = glm::lookAt(vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, -1.0f),
			vec3(0.0f, 1.0f, 0.0f));
}

//----------------------------------------------------------------------------------------
void A3::initLightSources() {
	// World-space position
	m_light.position = vec3(-2.0f, 5.0f, 0.5f);
	m_light.rgbIntensity = vec3(0.8f); // White light
}

//----------------------------------------------------------------------------------------
void A3::uploadCommonSceneUniforms() {
	m_shader.enable();
	{
		//-- Set Perpsective matrix uniform for the scene:
		GLint location = m_shader.getUniformLocation("Perspective");
		glUniformMatrix4fv(location, 1, GL_FALSE, value_ptr(m_perpsective));
		CHECK_GL_ERRORS;

		//-- Set LightSource uniform for the scene:
		{
			location = m_shader.getUniformLocation("light.position");
			glUniform3fv(location, 1, value_ptr(m_light.position));
			location = m_shader.getUniformLocation("light.rgbIntensity");
			glUniform3fv(location, 1, value_ptr(m_light.rgbIntensity));
			CHECK_GL_ERRORS;
		}

		//-- Set background light ambient intensity
		{
			location = m_shader.getUniformLocation("ambientIntensity");
			vec3 ambientIntensity(0.05f);
			glUniform3fv(location, 1, value_ptr(ambientIntensity));
			CHECK_GL_ERRORS;
		}
	}
	m_shader.disable();

	m_shader_picking.enable();
	{
		GLint location = m_shader_picking.getUniformLocation("Perspective");
		glUniformMatrix4fv(location, 1, GL_FALSE, value_ptr(m_perpsective));
		CHECK_GL_ERRORS;
	}
	m_shader_picking.disable();
}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, before guiLogic().
 */
void A3::appLogic()
{
	// Place per frame, application logic here ...

	uploadCommonSceneUniforms();
}

void A3::resetPosition()
{
	puppetTranslation = initialTranslation;
}

void A3::resetRotation()
{
	puppetRotation = initialRotation;
}

void A3::resetJoints()
{
	for (auto it = nodeMap.begin(); it != nodeMap.end(); it++){
		it->second->trans = it->second->initialTrans;
	}
	commandStack.clear();
	redoStack.clear();
}

void A3::resetAll()
{
	resetPosition();
	resetRotation();
	resetJoints();
}

void A3::undo()
{
	if(commandStack.size() == 0)
		return;
	vector<Command> state = commandStack.back();
	vector<Command> redoState;
	commandStack.pop_back();
	int i = state.size();
	while(i--){
		Command c = state[i];
		Command r = c;
		JointNode* joint = dynamic_cast<JointNode*>(A3::nodeMap[c.objectId]);
		r.originalTrans = joint->trans;
		r.oxr = joint->xr;
		r.oyr = joint->yr;
		joint->trans = c.originalTrans;
		joint->xr = c.oxr;
		joint->yr = c.oyr;
		redoState.push_back(r);
	}
	redoStack.push_back(redoState);
}

void A3::redo()
{
	if(redoStack.size() == 0)
		return;
	vector<Command> state = redoStack.back();
	vector<Command> undoState;
	redoStack.pop_back();
	int i = state.size();
	while(i--){
		Command c = state[i];
		Command u = c;
		JointNode* joint = dynamic_cast<JointNode*>(A3::nodeMap[c.objectId]);
		u.originalTrans = joint->trans;
		u.oxr = joint->xr;
		u.oyr = joint->yr;
		joint->trans = c.originalTrans;
		joint->xr = c.oxr;
		joint->yr = c.oyr;
		undoState.push_back(u);
	}
	commandStack.push_back(undoState);
}
//----------------------------------------------------------------------------------------
/*
 * Called once per frame, after appLogic(), but before the draw() method.
 */
void A3::guiLogic()
{
	if( !show_gui ) {
		return;
	}

	static bool firstRun(true);
	if (firstRun) {
		ImGui::SetNextWindowPos(ImVec2(50, 50));
		firstRun = false;
	}

	static bool showDebugWindow(true);
	ImGuiWindowFlags windowFlags(ImGuiWindowFlags_AlwaysAutoResize);
	float opacity(0.5f);

	if (ImGui::BeginMainMenuBar()) {
		if (ImGui::BeginMenu("Application")) {
		    if (ImGui::MenuItem("Reset Position", "I")) {
				resetPosition();
			}
			if (ImGui::MenuItem("Reset Orientation", "O")) {
				resetRotation();
			}
			if (ImGui::MenuItem("Reset Joints", "N")) {
				resetJoints();
			}
			if (ImGui::MenuItem("Reset All", "A")) {
				resetAll();
			}
			if (ImGui::MenuItem("Quit", "Q")) {
				glfwSetWindowShouldClose(m_window, GL_TRUE);
			}
			ImGui::EndMenu();
		}
	
		if (ImGui::BeginMenu("Edit")) {
			if (ImGui::MenuItem("Undo", "U")) {
				undo();
			}
			if (ImGui::MenuItem("Redo", "R")) {
				redo();
			}
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Options")) {
			ImGui::MenuItem("Circle", "C", &circleEnabled, true);
			ImGui::MenuItem("Z-buffer", "Z", &depthTesting, true);
			ImGui::MenuItem("Backface culling", "B", &cullBack, true);
			ImGui::MenuItem("Frontface culling", "F", &cullFront, true);
			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}

	bool selected;
	if (ImGui::RadioButton("Position/Orientation (P)", &mode, PMode)) {
		std::vector<SceneNode*> selectedNodes = getAllSelected();
		int i = selectedNodes.size();
		while(i--){
			selectedNodes[i]->isSelected = false;
		}
	}
	
	if (ImGui::RadioButton("Joints (J)", &mode, JMode)) {
	}

	ImGui::Begin("Properties", &showDebugWindow, ImVec2(100,100), opacity,
			windowFlags);


		// Add more gui elements here here ...


		// Create Button, and check if it was clicked:
		if( ImGui::Button( "Quit Application" ) ) {
			glfwSetWindowShouldClose(m_window, GL_TRUE);
		}

		ImGui::Text( "Framerate: %.1f FPS", ImGui::GetIO().Framerate );

	ImGui::End();
}

void A3::updateShaderUniformsPicking(
	const ShaderProgram & shader,
	const GeometryNode & node, 
	const glm::mat4 & viewMatrix
) {
	shader.enable();
	{
		GLint location = shader.getUniformLocation("ModelView");
		mat4 modelView = viewMatrix * node.trans;
		glUniformMatrix4fv(location, 1, GL_FALSE, value_ptr(modelView));
		CHECK_GL_ERRORS;

		location = shader.getUniformLocation("objectID");
		glUniform1ui(location, node.m_nodeId);
		CHECK_GL_ERRORS;
	}
	shader.disable();
}

//----------------------------------------------------------------------------------------
// Update mesh specific shader uniforms:
void A3::updateShaderUniforms(
		const ShaderProgram & shader,
		const GeometryNode & node,
		const glm::mat4 & viewMatrix
) {

	shader.enable();
	{
		//-- Set ModelView matrix:
		GLint location = shader.getUniformLocation("ModelView");
		mat4 modelView = viewMatrix * node.trans;
		glUniformMatrix4fv(location, 1, GL_FALSE, value_ptr(modelView));
		CHECK_GL_ERRORS;

		//-- Set NormMatrix:
		location = shader.getUniformLocation("NormalMatrix");
		mat3 normalMatrix = glm::transpose(glm::inverse(mat3(modelView)));
		glUniformMatrix3fv(location, 1, GL_FALSE, value_ptr(normalMatrix));
		CHECK_GL_ERRORS;

		location = shader.getUniformLocation("isSelected");
		glUniform1i(location, node.isSelected);
		CHECK_GL_ERRORS;

		milliseconds ms = duration_cast< milliseconds >(
			system_clock::now().time_since_epoch()
		);

		location = shader.getUniformLocation("time");
		float timeparam = (ms.count()%200) / 200.f;
		glUniform1f(location, timeparam);
		
		CHECK_GL_ERRORS;

		//-- Set Material values:
		location = shader.getUniformLocation("material.kd");
		vec3 kd = node.material.kd;
		glUniform3fv(location, 1, value_ptr(kd));
		CHECK_GL_ERRORS;
		location = shader.getUniformLocation("material.ks");
		vec3 ks = node.material.ks;
		glUniform3fv(location, 1, value_ptr(ks));
		CHECK_GL_ERRORS;
		location = shader.getUniformLocation("material.shininess");
		glUniform1f(location, node.material.shininess);
		CHECK_GL_ERRORS;

	}
	shader.disable();

}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, after guiLogic().
 */
void A3::draw() {

	glEnable( GL_DEPTH_TEST );
	glEnable( GL_CULL_FACE );
	
	if(cullFront) 
		glCullFace(GL_FRONT);
	if(cullBack)
		glCullFace(GL_BACK);
	if(cullFront && cullBack)
		glCullFace(GL_FRONT_AND_BACK);
	if(!cullFront && !cullBack)
		glDisable( GL_CULL_FACE );

	m_rootNode->trans = puppetTranslation * puppetRotation;

	renderPicking(*m_rootNode);
	renderSceneGraph(*m_rootNode);

	glDisable( GL_DEPTH_TEST );
	if(circleEnabled){
		renderArcCircle();	
	}
}

void A3::renderPicking(const SceneNode & root) {
	glEnable( GL_DEPTH_TEST );	
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbo);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glBindVertexArray(m_vao_picking);

	matStack = std::stack<glm::mat4>();
	matStack.push(m_view);
	root.render(&matStack, &m_shader_picking, &m_batchInfoMap, true);

	glBindVertexArray(0);
	CHECK_GL_ERRORS;

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glDisable( GL_DEPTH_TEST );
}

//----------------------------------------------------------------------------------------
void A3::renderSceneGraph(const SceneNode & root) {
	if(depthTesting){
		glEnable( GL_DEPTH_TEST );	
	}else
	{
		glDisable(GL_DEPTH_TEST);
	}	
	// Bind the VAO once here, and reuse for all GeometryNode rendering below.
	glBindVertexArray(m_vao_meshData);
	
	matStack = std::stack<glm::mat4>();
	matStack.push(m_view);
	root.render(&matStack, &m_shader, &m_batchInfoMap);

	glBindVertexArray(0);
	CHECK_GL_ERRORS;

	glDisable(GL_DEPTH_TEST);

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, m_fbo);
	glBlitFramebuffer(0,0,1000,700,0,0,1000,700,GL_COLOR_BUFFER_BIT,GL_NEAREST);
}

//----------------------------------------------------------------------------------------
// Draw the trackball circle.
void A3::renderArcCircle() {
	glBindVertexArray(m_vao_arcCircle);

	m_shader_arcCircle.enable();
		GLint m_location = m_shader_arcCircle.getUniformLocation( "M" );
		float aspect = float(m_framebufferWidth)/float(m_framebufferHeight);
		glm::mat4 M;
		if( aspect > 1.0 ) {
			M = glm::scale( glm::mat4(), glm::vec3( 0.5/aspect, 0.5, 1.0 ) );
		} else {
			M = glm::scale( glm::mat4(), glm::vec3( 0.5, 0.5*aspect, 1.0 ) );
		}
		glUniformMatrix4fv( m_location, 1, GL_FALSE, value_ptr( M ) );
		glDrawArrays( GL_LINE_LOOP, 0, CIRCLE_PTS );
	m_shader_arcCircle.disable();

	glBindVertexArray(0);
	CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
/*
 * Called once, after program is signaled to terminate.
 */
void A3::cleanup()
{

}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles cursor entering the window area events.
 */
bool A3::cursorEnterWindowEvent (
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
bool A3::mouseMoveEvent (
		double xPos,
		double yPos
) {
	bool eventHandled(false);

	// Fill in with event handling code...
	mouseX = xPos;
	mouseY = yPos;
	
	double dX = mouseX - oldMouseX; 
	double dY = mouseY - oldMouseY; 

	if(isMovingPuppet)
	{
		puppetTranslation = glm::translate(puppetTranslation, vec3(dX / 100.0, -dY / 100.0, 0));
	}
	if(isMovingPuppetZ)
	{
		puppetTranslation = glm::translate(puppetTranslation, vec3(0, 0, dY / 100.0));
	}
	if(isRotatingPuppet)
	{
		vec3 oldArcVector = get_arcball_vector(oldMouseX, oldMouseY);
		vec3 newArcVector = get_arcball_vector(mouseX, mouseY);
		float amount = acos(dot(oldArcVector, newArcVector));
		puppetRotation = glm::rotate(puppetRotation, amount*2, cross(oldArcVector, newArcVector));
	}
	if(isRotatingJoint)
	{
		vector<SceneNode*> selectedNodes = getAllSelected();
		int i = selectedNodes.size();
		while(i--){
			SceneNode* nd = selectedNodes[i];
			SceneNode* parent = nd->parent;
			if(parent != nullptr){
				if(parent->m_nodeType == NodeType::JointNode){
					JointNode* joint = dynamic_cast<JointNode*>(parent);

					float ramount = dY/100.0f;
					float aamount = ramount * 180/3.1415f;

					if(joint->xr + aamount < joint->m_joint_x.max && joint->xr + aamount > joint->m_joint_x.min ){
						joint->trans = glm::rotate(joint->trans, ramount, vec3(1.0f,0.0f,0.0f));
						joint->xr += aamount;
					}

					if(joint->yr + aamount < joint->m_joint_y.max && joint->yr + aamount > joint->m_joint_y.min ){
						joint->trans = glm::rotate(joint->trans, ramount, vec3(0.0f,1.0f,0.0f));
						joint->yr += aamount;
					}
				}
			}
		}
	}
	if(isRotatingHead){
		
		if(A3::nameMap["head"]->isSelected){
			float ramount = dY/100.0f;
			float aamount = ramount * 180/3.1415f;

			JointNode* joint = dynamic_cast<JointNode*>(A3::nameMap["neckjoint"]);
			if(joint->yr + aamount < joint->m_joint_y.max && joint->yr + aamount > joint->m_joint_y.min ){
				joint->trans = glm::rotate(joint->trans, ramount, vec3(0.0f,1.0f,0.0f));
				joint->yr += aamount;
			}
		}
	}

	oldMouseX = mouseX;
	oldMouseY = mouseY;

	return eventHandled;
}

// https://en.wikibooks.org/wiki/OpenGL_Programming/Modern_OpenGL_Tutorial_Arcball
glm::vec3 A3::get_arcball_vector(float x, float y)
{
	glm::vec3 P = glm::vec3(x/1024 * 2 - 1.0, y / 768 * 2 - 1.0, 0);
	
	float OP_squared = P.x * P.x + P.y * P.y;
	if (OP_squared <= 1) {
		P.z = sqrt(1 - OP_squared);
	} else {
		P = glm::normalize(P);
	}
	return P;
}

std::vector<JointNode*> A3::getAllSelectedJoints(){
	vector<SceneNode*> selectedNodes = getAllSelected();
	vector<JointNode*> ret;
	int i = selectedNodes.size();
	while(i--){
		SceneNode* nd = selectedNodes[i];
		SceneNode* parent = nd->parent;
		if(parent != nullptr){
			if(parent->m_nodeType == NodeType::JointNode){
				JointNode* joint = dynamic_cast<JointNode*>(parent);
				ret.push_back(joint);
			}
		}
	}
	return ret;
}

void A3::pushCommands()
{
	vector<JointNode*> selectedJoints = getAllSelectedJoints();
	if(selectedJoints.size() == 0){
		return;
	}
	vector<Command> commands;
	int i = selectedJoints.size();
	while(i--){
		Command c = {selectedJoints[i]->m_nodeId, selectedJoints[i]->trans, selectedJoints[i]->xr, selectedJoints[i]->yr};
		commands.push_back(c);
	}
	commandStack.push_back(commands);
}

void A3::pushHeadCommands()
{
	cout<<"asd"<<endl;
	if(A3::nameMap["head"]->isSelected){
		JointNode* joint = dynamic_cast<JointNode*>(A3::nameMap["neckjoint"]);
		vector<Command> singletonVector;
		Command c = {joint->m_nodeId, joint->trans, joint->xr, joint->yr};
		singletonVector.push_back(c);
		commandStack.push_back(singletonVector);
	}

}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles mouse button events.
 */
bool A3::mouseButtonInputEvent (
		int button,
		int actions,
		int mods
) {
	bool eventHandled(false);

	if (ImGui::IsMouseHoveringAnyWindow()) return eventHandled;

	// Fill in with event handling code...
	if(button == GLFW_MOUSE_BUTTON_LEFT && actions == GLFW_PRESS) {
		if(mode == JMode)
		{
			unsigned int objectId = getObjectIdAt((int)mouseX, (int)768-mouseY);
			if(objectId != 0 )
			{
				SceneNode* parent = nodeMap[objectId]->parent;
				if(parent != nullptr){
					if(parent->m_nodeType == NodeType::JointNode){
						A3::nodeMap[objectId]->isSelected = !A3::nodeMap[objectId]->isSelected;
					}
				}
			}
			
		}
		if(mode == PMode){
			isMovingPuppet = true;
		}
	}

	if(button == GLFW_MOUSE_BUTTON_RIGHT && actions == GLFW_PRESS) {
		if(mode == JMode)
		{
			isRotatingHead = true;
			pushHeadCommands();
			redoStack.clear();
		}
		if(mode == PMode){
			isRotatingPuppet = true;
		}
	}

	if(button == GLFW_MOUSE_BUTTON_MIDDLE && actions == GLFW_PRESS) {
		if(mode == JMode)
		{
			isRotatingJoint = true;
			pushCommands();
			redoStack.clear();
		}
		if(mode == PMode){
			isMovingPuppetZ = true;
		}
	}

	if(button == GLFW_MOUSE_BUTTON_LEFT && actions == GLFW_RELEASE) {
		if(mode == PMode){
			isMovingPuppet = false;
		}
	}

	if(button == GLFW_MOUSE_BUTTON_RIGHT && actions == GLFW_RELEASE) {
		if(mode == JMode)
		{
			isRotatingHead = false;
		}
		if(mode == PMode){
			isRotatingPuppet = false;
		}
	}

	if(button == GLFW_MOUSE_BUTTON_MIDDLE && actions == GLFW_RELEASE) {
		if(mode == JMode)
		{
			isRotatingJoint = false;
		}
		if(mode == PMode){
			isMovingPuppetZ = false;
		}
	}

	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles mouse scroll wheel events.
 */
bool A3::mouseScrollEvent (
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
bool A3::windowResizeEvent (
		int width,
		int height
) {
	bool eventHandled(false);
	initPerspectiveMatrix();
	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles key input events.
 */
bool A3::keyInputEvent (
		int key,
		int action,
		int mods
) {
	bool eventHandled(false);

	if( action == GLFW_PRESS ) {
		if( key == GLFW_KEY_M ) {
			show_gui = !show_gui;
			eventHandled = true;
		}
		if( key ==  GLFW_KEY_Q ) {
			glfwSetWindowShouldClose(m_window, GL_TRUE);
		}
		if( key ==  GLFW_KEY_R ) {
			redo();
		}
		if( key ==  GLFW_KEY_C ) {
			circleEnabled = !circleEnabled;
		}
		if( key ==  GLFW_KEY_B ) {
			cullBack = !cullBack;
		}
		if( key ==  GLFW_KEY_F ) {
			cullFront = !cullFront;
		}
		if( key ==  GLFW_KEY_L ) {
			mode = PMode;
		}
		if( key ==  GLFW_KEY_J ) {
			mode = JMode;
		}
		if( key ==  GLFW_KEY_Z ) {
			depthTesting = !depthTesting;
		}


		if( key ==  GLFW_KEY_I ) {
			resetPosition();
		}
		if( key ==  GLFW_KEY_O ) {
			resetRotation();
		}
		if( key ==  GLFW_KEY_N ) {
			resetJoints();
		}
		if( key ==  GLFW_KEY_A ) {
			resetAll();
		}


		if( key ==  GLFW_KEY_U ) {
			undo();
		}
		if( key ==  GLFW_KEY_COMMA ) {
			if(mode == PMode)
			{
				isMovingPuppetZ = true;
			}
			if(mode == JMode)
			{
				pushCommands();
				redoStack.clear();
				isRotatingJoint = true;
			}
		}
	}
	if( action == GLFW_RELEASE ) {
		if( key == GLFW_KEY_M ) {
			show_gui = !show_gui;
			eventHandled = true;
		}
		if( key ==  GLFW_KEY_Q ) {
			glfwSetWindowShouldClose(m_window, GL_TRUE);
		}
		if( key ==  GLFW_KEY_COMMA ) {
			if(mode == PMode)
			{
				isMovingPuppetZ = false;
			}
			if(mode == JMode)
			{
				isRotatingJoint = false;
			}
		}
	}
	// Fill in with event handling code...

	return eventHandled;
}
