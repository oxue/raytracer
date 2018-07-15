#include "GeometryNode.hpp"
#include <iostream>
#include "A3.hpp"
#include "cs488-framework/CS488Window.hpp"
#include "cs488-framework/OpenGLImport.hpp"
#include "cs488-framework/ShaderProgram.hpp"
#include "cs488-framework/MeshConsolidator.hpp"

//---------------------------------------------------------------------------------------
GeometryNode::GeometryNode(
		const std::string & meshId,
		const std::string & name
)
	: SceneNode(name),
	  meshId(meshId)
{
	m_nodeType = NodeType::GeometryNode;
}

void GeometryNode::render(std::stack<glm::mat4> *matStack, ShaderProgram *shader, BatchInfoMap* batchmap, bool picking) const
{

	//render childs
	if(!picking){
		A3::updateShaderUniforms(*shader, *this, matStack->top());
	} else {
		A3::updateShaderUniformsPicking(*shader, *this, matStack->top());
	}
	
	BatchInfo batchInfo = (*batchmap)[meshId];
	
	shader->enable();
		glDrawArrays(GL_TRIANGLES, batchInfo.startIndex, batchInfo.numIndices);
	shader->disable();


	SceneNode::render(matStack, shader, batchmap, picking);
}