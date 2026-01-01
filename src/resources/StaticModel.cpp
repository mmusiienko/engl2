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

	static std::vector<Mesh::Index> quadIndicesTesselated =
	{
		0, 1, 2, 3
	};

	AssetHandle<Model> StaticModel::Quad(AssetHandle<scope<Material::Base>> mat)
	{
		static auto meshHandle = AssetManager::Put<Mesh>(Mesh::CreationInfo{.Vertices = quadVertices, .Indices = quadIndices, .HasNormals = true, .HasTextureCoords = true });

		return AssetManager::Put<Model>(Model::Submesh{
			.Mesh = meshHandle,
			.Material = mat
		});
	}

	AssetHandle<Model> StaticModel::QuadTesselated(AssetHandle<scope<Material::Base>> mat, u32 w, u32 h)
	{
		std::vector<Mesh::Vertex> vertices;
		std::vector<Mesh::Index> indices;

		for (u32 y = 0; y <= h; y++)
		{
			for (u32 x = 0; x <= w; x++)
			{
				f32 u = static_cast<f32>(x) / w;
				f32 v = static_cast<f32>(y) / h;
				vertices.push_back({ {2.0f * u - 1.0f, 2.0f * v - 1.0f, 0.0f}, {}, {u,v} });
			}
		}

		for (u32 y = 0; y < h; y++)
		{
			for (u32 x = 0; x < w; x++)
			{
				u32 i0 = y * (w + 1) + x;
				u32 i1 = i0 + 1;
				u32 i2 = i0 + w + 2;
				u32 i3 = i0 + w + 1;

				indices.push_back(i0);
				indices.push_back(i1);
				indices.push_back(i2);
				indices.push_back(i3);
			}
		}

		auto meshHandle = AssetManager::Put<Mesh>(
			Mesh::CreationInfo{ 
				.Vertices = vertices,
				.Indices = indices,
				.HasNormals = true,
				.HasTextureCoords = true,
				.DrawType = GL_PATCHES
			});

		return AssetManager::Put<Model>(Model::Submesh{
			.Mesh = meshHandle,
			.Material = mat,
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
