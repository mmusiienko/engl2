#include "core/Global.h"

#include "resources/importers/AssetManager.h"
#include "renderer/base/Material.h"


namespace EnGl
{
	f32 Global::DeltaTime = 0.0f;
	f32 Global::LastFrame = 0.0f;
	f32 Global::Time = 0.0f;
	glm::uvec2 Global::WindowResolution{1280u,720u};
	bool Global::IsPaused = false;


	void Global::StartUp()
	{
		auto aoroughvec = std::vector<f32>{ 1.0f };
		auto metalvec = std::vector<f32>{ 0.0f };
		auto aoroughdata = Texture::CreationInfoFromData{ .Data = aoroughvec.data(), .CpuFormat = GL_RED, .GpuFormat = GL_R8, .DataType = GL_FLOAT };
		auto metaldata = Texture::CreationInfoFromData{ .Data = metalvec.data(), .CpuFormat = GL_RED, .GpuFormat = GL_R8, .DataType = GL_FLOAT };

		auto normalvec = std::vector<glm::vec3>{ {0.5f, 0.5f, 1.0f } };
		auto normaldata = Texture::CreationInfoFromData{ .Data = normalvec.data(), .CpuFormat = GL_RGB, .GpuFormat = GL_RGB8, .DataType = GL_FLOAT };

		auto armvec = std::vector<glm::vec3>{ {0.0f, 1.0f, 0.0f } };
		auto armdata = Texture::CreationInfoFromData{ .Data = armvec.data(), .CpuFormat = GL_RGB, .GpuFormat = GL_RGB8, .DataType = GL_FLOAT };

		Material::PlaceholderTextures::AO = AssetManager::Put<Texture2D>(1u, aoroughdata);
		Material::PlaceholderTextures::Roughness = Material::PlaceholderTextures::AO;
		Material::PlaceholderTextures::Opacity = Material::PlaceholderTextures::AO;
		Material::PlaceholderTextures::Normals = AssetManager::Put<Texture2D>(1u, normaldata);
		Material::PlaceholderTextures::Metallic = AssetManager::Put<Texture2D>(1u, metaldata);
		Material::PlaceholderTextures::ARM = AssetManager::Put<Texture2D>(1u, armdata);
	}

	void Global::ShutDown()
	{
		AssetManager::ShutDown();
	}
}