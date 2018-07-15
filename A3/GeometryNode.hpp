#pragma once

#include "SceneNode.hpp"
#include <stack>
#include <glm/glm.hpp>
#include "cs488-framework/ShaderProgram.hpp"
#include "cs488-framework/CS488Window.hpp"
#include "cs488-framework/OpenGLImport.hpp"
#include "cs488-framework/MeshConsolidator.hpp"

class GeometryNode : public SceneNode {
public:
	GeometryNode(
		const std::string & meshId,
		const std::string & name
	);

	Material material;

	// Mesh Identifier. This must correspond to an object name of
	// a loaded .obj file.
	std::string meshId;

	void render(std::stack<glm::mat4> *matStack, ShaderProgram *shader, BatchInfoMap* batchmap, bool picking = false) const;
};
