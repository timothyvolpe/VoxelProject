#pragma once
#include <glm\glm.hpp>
#include <memory>
#include "components.h"

class CVertexArray;
class CBufferObject;
class CShaderProgram;

struct Vertex3D
{
	glm::vec3 position;
};

class CRenderSystem : public CSystemBase
{
private:
	std::shared_ptr<CVertexArray> m_vertexArray;
	std::shared_ptr<CBufferObject> m_vertexBuffer;

	std::shared_ptr<CShaderProgram> m_simpleProgram;

	unsigned int m_simpleShaderIndex;
	std::vector<Vertex3D> m_vertices;

	unsigned int m_mvpUniformLocation;
	glm::mat4 m_projectionMatrix, m_viewMatrix, m_modelMatrix;
public:
	CRenderSystem( CGame *pGameHandle, CECSCoordinator *pCoordinator );
	~CRenderSystem();

	bool initialize();
	void shutdown();

	bool onLoad();

	bool update( float deltaT );

	void updateShaderUniforms();
};