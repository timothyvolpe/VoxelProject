#pragma once
#include "components.h"

class CRenderSystem : public CSystemBase
{
public:
	bool initialize();
	void shutdown();

	bool update( float deltaT );
};