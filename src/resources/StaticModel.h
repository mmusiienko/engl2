#pragma once
#include "importers/AssetManager.h"


namespace EnGl
{
	class Model;
	namespace Material { struct Base; }

	class StaticModel
	{
	public:
		static AssetHandle<Model> Quad(AssetHandle<scope<Material::Base>> mat = GetDefaultMaterial());
		static AssetHandle<Model> QuadInstanced(AssetHandle<scope<Material::Base>> mat = GetDefaultMaterial());
		static AssetHandle<Model> QuadTesselated(AssetHandle<scope<Material::Base>> mat, u32 w, u32 h);
		static AssetHandle<Model> Cube(AssetHandle<scope<Material::Base>> mat = GetDefaultMaterial());
		static AssetHandle<Model> CubeInstanced(AssetHandle<scope<Material::Base>> mat = GetDefaultMaterial());
		static AssetHandle<scope<Material::Base>> GetDefaultMaterial();
	};
}