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

		// Back face (Z-)
		{{-1.0f,  1.0f, -1.0f}, { 0,  0, -1}, {0, 1}},
		{{-1.0f, -1.0f, -1.0f}, { 0,  0, -1}, {0, 0}},
		{{ 1.0f, -1.0f, -1.0f}, { 0,  0, -1}, {1, 0}},
		{{ 1.0f, -1.0f, -1.0f}, { 0,  0, -1}, {1, 0}},
		{{ 1.0f,  1.0f, -1.0f}, { 0,  0, -1}, {1, 1}},
		{{-1.0f,  1.0f, -1.0f}, { 0,  0, -1}, {0, 1}},

		// Left face (X-)
		{{-1.0f, -1.0f,  1.0f}, {-1,  0,  0}, {0, 0}},
		{{-1.0f, -1.0f, -1.0f}, {-1,  0,  0}, {1, 0}},
		{{-1.0f,  1.0f, -1.0f}, {-1,  0,  0}, {1, 1}},
		{{-1.0f,  1.0f, -1.0f}, {-1,  0,  0}, {1, 1}},
		{{-1.0f,  1.0f,  1.0f}, {-1,  0,  0}, {0, 1}},
		{{-1.0f, -1.0f,  1.0f}, {-1,  0,  0}, {0, 0}},

		// Right face (X+)
		{{ 1.0f, -1.0f, -1.0f}, { 1,  0,  0}, {0, 0}},
		{{ 1.0f, -1.0f,  1.0f}, { 1,  0,  0}, {1, 0}},
		{{ 1.0f,  1.0f,  1.0f}, { 1,  0,  0}, {1, 1}},
		{{ 1.0f,  1.0f,  1.0f}, { 1,  0,  0}, {1, 1}},
		{{ 1.0f,  1.0f, -1.0f}, { 1,  0,  0}, {0, 1}},
		{{ 1.0f, -1.0f, -1.0f}, { 1,  0,  0}, {0, 0}},

		// Front face (Z+)
		{{-1.0f, -1.0f,  1.0f}, { 0,  0,  1}, {0, 0}},
		{{-1.0f,  1.0f,  1.0f}, { 0,  0,  1}, {0, 1}},
		{{ 1.0f,  1.0f,  1.0f}, { 0,  0,  1}, {1, 1}},
		{{ 1.0f,  1.0f,  1.0f}, { 0,  0,  1}, {1, 1}},
		{{ 1.0f, -1.0f,  1.0f}, { 0,  0,  1}, {1, 0}},
		{{-1.0f, -1.0f,  1.0f}, { 0,  0,  1}, {0, 0}},

		// Top face (Y+)
		{{-1.0f,  1.0f, -1.0f}, { 0,  1,  0}, {0, 0}},
		{{ 1.0f,  1.0f, -1.0f}, { 0,  1,  0}, {1, 0}},
		{{ 1.0f,  1.0f,  1.0f}, { 0,  1,  0}, {1, 1}},
		{{ 1.0f,  1.0f,  1.0f}, { 0,  1,  0}, {1, 1}},
		{{-1.0f,  1.0f,  1.0f}, { 0,  1,  0}, {0, 1}},
		{{-1.0f,  1.0f, -1.0f}, { 0,  1,  0}, {0, 0}},

		// Bottom face (Y-)
		{{-1.0f, -1.0f, -1.0f}, { 0, -1,  0}, {0, 0}},
		{{-1.0f, -1.0f,  1.0f}, { 0, -1,  0}, {0, 1}},
		{{ 1.0f, -1.0f, -1.0f}, { 0, -1,  0}, {1, 0}},
		{{ 1.0f, -1.0f, -1.0f}, { 0, -1,  0}, {1, 0}},
		{{-1.0f, -1.0f,  1.0f}, { 0, -1,  0}, {0, 1}},
		{{ 1.0f, -1.0f,  1.0f}, { 0, -1,  0}, {1, 1}},
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
		{{-1.0f, -1.0f, 0.0f}, {-1.0f, -1.0f, 1.0f}, {0.0f, 0.0f}},
		{{ 1.0f, -1.0f, 0.0f}, { 1.0f, -1.0f, 1.0f}, {1.0f, 0.0f}},
		{{ 1.0f,  1.0f, 0.0f}, { 1.0f,  1.0f, 1.0f}, {1.0f, 1.0f}},
		{{-1.0f,  1.0f, 0.0f}, {-1.0f,  1.0f, 1.0f}, {0.0f, 1.0f}}
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

	static AssetHandle<Mesh> GetSphere()
	{
		std::vector<Mesh::Vertex> vertices;
		std::vector<Mesh::Index> indices;

		const unsigned int X_SEGMENTS = 64;
		const unsigned int Y_SEGMENTS = 64;
		const float PI = 3.14159265359f;
		for (unsigned int x = 0; x <= X_SEGMENTS; ++x)
		{
			for (unsigned int y = 0; y <= Y_SEGMENTS; ++y)
			{
				float xSegment = (float)x / (float)X_SEGMENTS;
				float ySegment = (float)y / (float)Y_SEGMENTS;
				float xPos = std::cos(xSegment * 2.0f * PI) * std::sin(ySegment * PI);
				float yPos = std::cos(ySegment * PI);
				float zPos = std::sin(xSegment * 2.0f * PI) * std::sin(ySegment * PI);

				vertices.emplace_back(
					Mesh::Vertex
					{
						.Position = {xPos, yPos, zPos},
						.Normal = {xPos, yPos, zPos},
						.TexCoord = {xSegment, ySegment}
					}
				);
			}
		}

		bool oddRow = false;
		for (unsigned int y = 0; y < Y_SEGMENTS; ++y)
		{
			if (!oddRow) // even rows: y == 0, y == 2; and so on
			{
				for (unsigned int x = 0; x <= X_SEGMENTS; ++x)
				{
					indices.push_back(y * (X_SEGMENTS + 1) + x);
					indices.push_back((y + 1) * (X_SEGMENTS + 1) + x);
				}
			}
			else
			{
				for (int x = X_SEGMENTS; x >= 0; --x)
				{
					indices.push_back((y + 1) * (X_SEGMENTS + 1) + x);
					indices.push_back(y * (X_SEGMENTS + 1) + x);
				}
			}
			oddRow = !oddRow;
		}
		auto info = Mesh::CreationInfo{ .Vertices = vertices, .Indices = indices, .HasNormals = true, .HasTextureCoords = true };
		info.DrawType = GL_TRIANGLE_STRIP;
		return AssetManager::Put<Mesh>(info);
	}

	AssetHandle<Model> StaticModel::Sphere(AssetHandle<scope<Material::Base>> mat)
	{
		static auto meshHandle = GetSphere();

		return AssetManager::Put<Model>(Model::Submesh{
			.Mesh = meshHandle,
			.Material = mat
		}, false);
	}
}
