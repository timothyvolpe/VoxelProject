#pragma once
#include <memory>
#include "components.h"

class CVertexArray;
class CBufferObject;

class CRenderSystem : public CSystemBase
{
private:
	std::unique_ptr<CVertexArray> m_vertexArray;
	std::shared_ptr<CBufferObject> m_vertexBuffer;
public:
	CRenderSystem( CGame *pGameHandle );
	~CRenderSystem();

	bool initialize();
	void shutdown();

	bool update( float deltaT );
};