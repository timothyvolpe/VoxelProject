#pragma once
#include <glm\glm.hpp>
#include <memory>
#include "components.h"

class CVertexArray;
class CBufferObject;

struct Vertex3D
{
	glm::vec3 position;
};

class CRenderSystem : public CSystemBase
{
private:
	std::shared_ptr<CVertexArray> m_vertexArray;
	std::shared_ptr<CBufferObject> m_vertexBuffer;

	unsigned int m_simpleShaderIndex;
	unsigned int m_vertexCount;

	unsigned int m_mvpUniformLocation;
public:
	CRenderSystem( CGame *pGameHandle, CECSCoordinator *pCoordinator );
	~CRenderSystem();

	bool initialize();
	void shutdown();

	bool onLoad();

	bool update( float deltaT );
};