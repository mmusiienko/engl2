#include "StaticModel.h"
#include "../renderer/base/Model.h"
#include "../renderer/base/Material.h"
#include "../renderer/base/Mesh.h"


namespace EnGl
{
	AssetHandle<scope<Material::Base>> StaticModel::GetDefaultMaterial()
	{
		static AssetHandle<scope<Material::Base>> mat = AssetManager::Put<scope<Material::Base>>(make_scope<Material::Unlit>());
		return mat;
	}

	static std::vector<Mesh::Vertex> cubeVertices = {
			   {{-1.0f,  1.0f, -1.0f}, {}, {}},
			   {{-1.0f, -1.0f, -1.0f}, {}, {}},
			   {{ 1.0f, -1.0f, -1.0f}, {}, {}},
			   {{ 1.0f, -1.0f, -1.0f}, {}, {}},
			   {{ 1.0f,  1.0f, -1.0f}, {}, {}},
			   {{-1.0f,  1.0f, -1.0f}, {}, {}},

			   {{-1.0f, -1.0f,  1.0f}, {}, {}},
			   {{-1.0f, -1.0f, -1.0f}, {}, {}},
			   {{-1.0f,  1.0f, -1.0f}, {}, {}},
			   {{-1.0f,  1.0f, -1.0f}, {}, {}},
			   {{-1.0f,  1.0f,  1.0f}, {}, {}},
			   {{-1.0f, -1.0f,  1.0f}, {}, {}},

			   {{ 1.0f, -1.0f, -1.0f}, {}, {}},
			   {{ 1.0f, -1.0f,  1.0f}, {}, {}},
			   {{ 1.0f,  1.0f,  1.0f}, {}, {}},
			   {{ 1.0f,  1.0f,  1.0f}, {}, {}},
			   {{ 1.0f,  1.0f, -1.0f}, {}, {}},
			   {{ 1.0f, -1.0f, -1.0f}, {}, {}},

			   {{-1.0f, -1.0f,  1.0f}, {}, {}},
			   {{-1.0f,  1.0f,  1.0f}, {}, {}},
			   {{ 1.0f,  1.0f,  1.0f}, {}, {}},
			   {{ 1.0f,  1.0f,  1.0f}, {}, {}},
			   {{ 1.0f, -1.0f,  1.0f}, {}, {}},
			   {{-1.0f, -1.0f,  1.0f}, {}, {}},

			   {{-1.0f,  1.0f, -1.0f}, {}, {}},
			   {{ 1.0f,  1.0f, -1.0f}, {}, {}},
			   {{ 1.0f,  1.0f,  1.0f}, {}, {}},
			   {{ 1.0f,  1.0f,  1.0f}, {}, {}},
			   {{-1.0f,  1.0f,  1.0f}, {}, {}},
			   {{-1.0f,  1.0f, -1.0f}, {}, {}},

			   {{-1.0f, -1.0f, -1.0f}, {}, {}},
			   {{-1.0f, -1.0f,  1.0f}, {}, {}},
			   {{ 1.0f, -1.0f, -1.0f}, {}, {}},
			   {{ 1.0f, -1.0f, -1.0f}, {}, {}},
			   {{-1.0f, -1.0f,  1.0f}, {}, {}},
			   {{ 1.0f, -1.0f,  1.0f}, {}, {}}
	};

	static std::vector<Mesh::Index> cubeIndices = {
		 0,  1,  2,  3,  4,  5,
		 6,  7,  8,  9, 10, 11,
		12, 13, 14, 15, 16, 17,
		18, 19, 20, 21, 22, 23,
		24, 25, 26, 27, 28, 29,
		30, 31, 32, 33, 34, 35
	};

	AssetHandle<Model> StaticModel::Cube(AssetHandle<scope<Material::Base>> mat)
	{
		static auto meshHandle = AssetManager::Put<Mesh>(Mesh::CreationInfo{ .Vertices = cubeVertices, .Indices = cubeIndices, .HasNormals = true, .HasTextureCoords = true });

		return AssetManager::Put<Model>(Model::Submesh{
			.Mesh = meshHandle,
			.Material = mat
		});
	}

	AssetHandle<Model> StaticModel::CubeInstanced(AssetHandle<scope<Material::Base>> mat)
	{
		static auto meshHandle = AssetManager::Put<Mesh>(Mesh::CreationInfo{ .Vertices = cubeVertices, .Indices = cubeIndices, .HasNormals = true, .HasTextureCoords = true });

		return AssetManager::Put<Model>(Model::Submesh{
			.Mesh = meshHandle,
			.Material = mat
		}, true);
	}

	static std::vector<Mesh::Vertex> quadVertices =
	{
		{{-1.0f, -1.0f, 0.0f}, {}, {0.0f, 0.0f}},
		{{ 1.0f, -1.0f, 0.0f}, {}, {1.0f, 0.0f}},
		{{ 1.0f,  1.0f, 0.0f}, {}, {1.0f, 1.0f}},
		{{-1.0f,  1.0f, 0.0f}, {}, {0.0f, 1.0f}}
	};

	static std::vector<Mesh::Index> quadIndices =
	{
		0, 1, 2,
		2, 3, 0
	};

	AssetHandle<Model> StaticModel::Quad(AssetHandle<scope<Material::Base>> mat)
	{
		static auto meshHandle = AssetManager::Put<Mesh>(Mesh::CreationInfo{.Vertices = quadVertices, .Indices = quadIndices, .HasNormals = true, .HasTextureCoords = true });

		return AssetManager::Put<Model>(Model::Submesh{
			.Mesh = meshHandle,
			.Material = mat
		});
	}

	AssetHandle<Model> StaticModel::QuadInstanced(AssetHandle<scope<Material::Base>> mat)
	{
		static auto meshHandle = AssetManager::Put<Mesh>(Mesh::CreationInfo{ .Vertices = quadVertices, .Indices = quadIndices, .HasNormals = true, .HasTextureCoords = true });
		
		return AssetManager::Put<Model>(Model::Submesh{
			.Mesh = meshHandle,
			.Material = mat
		}, true);
	}
}
