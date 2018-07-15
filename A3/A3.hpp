#pragma once

#include "cs488-framework/CS488Window.hpp"
#include "cs488-framework/OpenGLImport.hpp"
#include "cs488-framework/ShaderProgram.hpp"
#include "cs488-framework/MeshConsolidator.hpp"

#include "SceneNode.hpp"
#include "GeometryNode.hpp"
#include "JointNode.hpp"

#include <glm/glm.hpp>
#include <memory>
#include <stack>
#include <map>
#include <string>
#include <vector>

struct LightSource {
	glm::vec3 position;
	glm::vec3 rgbIntensity;
};

struct Command {
	unsigned int objectId;
	glm::mat4 originalTrans;
	float oxr;
	float oyr;
};


class A3 : public CS488Window {
public:
	A3(const std::string & luaSceneFile);
	static void updateShaderUniforms(
		const ShaderProgram & shader,
		const GeometryNode & node,
		const glm::mat4 & viewMatrix
	);
	static void updateShaderUniformsPicking(
		const ShaderProgram & shader,
		const GeometryNode & node, 
		const glm::mat4 & viewMatrix
);
	virtual ~A3();

	static std::map<unsigned int, SceneNode*> nodeMap;
	static std::map<std::string, SceneNode*> nameMap;

	std::vector<SceneNode*> getAllSelected();

protected:
	virtual void init() override;
	virtual void appLogic() override;
	virtual void guiLogic() override;
	virtual void draw() override;
	virtual void cleanup() override;

	//-- Virtual callback methods
	virtual bool cursorEnterWindowEvent(int entered) override;
	virtual bool mouseMoveEvent(double xPos, double yPos) override;
	virtual bool mouseButtonInputEvent(int button, int actions, int mods) override;
	virtual bool mouseScrollEvent(double xOffSet, double yOffSet) override;
	virtual bool windowResizeEvent(int width, int height) override;
	virtual bool keyInputEvent(int key, int action, int mods) override;

	//-- One time initialization methods:
	void processLuaSceneFile(const std::string & filename);
	void createShaderProgram();
	void enableVertexShaderInputSlots();
	void uploadVertexDataToVbos(const MeshConsolidator & meshConsolidator);
	void mapVboDataToVertexShaderInputLocations();
	void initViewMatrix();
	void initLightSources();
	void undo();
	void redo();
	bool initPicking();
	unsigned int getObjectIdAt(int x, int y);
	glm::vec3 get_arcball_vector(float x, float y);

	void resetPosition();
	void resetRotation();
	void resetJoints();
	void resetAll();
	void pushCommands();
	void pushHeadCommands();
	std::vector<JointNode*> getAllSelectedJoints();

	void initPerspectiveMatrix();
	void uploadCommonSceneUniforms();
	void renderSceneGraph(const SceneNode &node);
	void renderPicking(const SceneNode &node);
	void renderArcCircle();

	glm::mat4 m_perpsective;
	glm::mat4 m_view;

	LightSource m_light;

	//-- GL resources for mesh geometry data:
	GLuint m_vao_meshData;
	GLuint m_vbo_vertexPositions;
	GLuint m_vbo_vertexNormals;
	GLint m_positionAttribLocation;
	GLint m_normalAttribLocation;
	ShaderProgram m_shader;

	//-- GL resources for trackball circle geometry:
	GLuint m_vbo_arcCircle;
	GLuint m_vao_arcCircle;
	GLint m_arc_positionAttribLocation;
	ShaderProgram m_shader_arcCircle;

	// BatchInfoMap is an associative container that maps a unique MeshId to a BatchInfo
	// object. Each BatchInfo object contains an index offset and the number of indices
	// required to render the mesh with identifier MeshId.
	BatchInfoMap m_batchInfoMap;

	std::string m_luaSceneFile;

	std::shared_ptr<SceneNode> m_rootNode;

	std::stack<glm::mat4> matStack;

	// for picking
	GLuint m_fbo;
    GLuint m_pickingTexture;
    GLuint m_depthTexture;
    GLuint m_vao_picking;
    GLint m_pickng_positionAttribLocation;
    ShaderProgram m_shader_picking;
    double mouseX;
    double mouseY;
    double oldMouseX;
    double oldMouseY;

    //options
    bool circleEnabled = false;
    bool depthTesting = true;
    bool cullBack = false;
    bool cullFront = false;

    int PMode = 0;
    int JMode = 1;
    int mode = PMode;

    bool isMovingPuppet = false;
    bool isMovingPuppetZ = false;
    bool isRotatingPuppet = false;
    bool isRotatingJoint = false;
    bool isRotatingHead = false;

    glm::mat4 puppetTranslation;
    glm::mat4 puppetRotation;

    glm::mat4 initialTranslation;
    glm::mat4 initialRotation;

    std::vector<std::vector<Command>> commandStack;
    std::vector<std::vector<Command>> redoStack;
};
