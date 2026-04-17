#pragma once

#include "resources/importers/AssetManager.h"


namespace EnGl
{
	class Model;
	namespace Material { struct Base; }

	class StaticMesh
	{
	public:
		static AssetHandle<Mesh> Quad();
		static AssetHandle<Mesh> Cube();
		static AssetHandle<Mesh> Sphere();
	};

	class StaticModel
	{
	public:
		static AssetHandle<Model> Quad(AssetHandle<Material::Base> mat = GetDefaultMaterial(), bool isInstanced = false);
		static AssetHandle<Model> QuadTesselated(AssetHandle<Material::Base> mat, u32 w, u32 h);
		static AssetHandle<Model> Cube(AssetHandle<Material::Base> mat = GetDefaultMaterial(), bool isInstanced = false);
		static AssetHandle<Model> Sphere(AssetHandle<Material::Base> mat = GetDefaultMaterial(), bool isInstanced = false);
		static AssetHandle<Material::Base> GetDefaultMaterial();
	};
}