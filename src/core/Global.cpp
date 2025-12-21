#include "Global.h"

namespace EnGl
{
	f32 Global::DeltaTime = 0.0f;
	f32 Global::LastFrame = 0.0f;
	f32 Global::Time = 0.0f;
	glm::uvec2 Global::WindowResolution{1280u,720u};
	bool Global::IsPaused = false;

	void Global::ShutDown()
	{
		AssetManager::ShutDown();
	}
}