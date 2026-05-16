#include "resources/StaticModel.h"

#include "renderer/base/Model.h"
#include "renderer/base/Material.h"
#include "renderer/base/Mesh.h"


namespace EnGl
{
	AssetHandle<Material::Base> StaticModel::GetDefaultMaterial()
	{
		static AssetHandle<Material::Base> mat = AssetManager::PutScope<Material::Base>(make_scope<Material::Unlit>());
		return mat;
	}

	static std::vector<Mesh::Vertex> cubeVertices = {

		// Back face (Z-)
		{{-1.0f,  1.0f, -1.0f}, { 0,  0, -1}, {0, 1}, {}, {}},
		{{-1.0f, -1.0f, -1.0f}, { 0,  0, -1}, {0, 0}, {}, {}},
		{{ 1.0f, -1.0f, -1.0f}, { 0,  0, -1}, {1, 0}, {}, {}},
		{{ 1.0f, -1.0f, -1.0f}, { 0,  0, -1}, {1, 0}, {}, {}},
		{{ 1.0f,  1.0f, -1.0f}, { 0,  0, -1}, {1, 1}, {}, {}},
		{{-1.0f,  1.0f, -1.0f}, { 0,  0, -1}, {0, 1}, {}, {}},

		// Left face (X-)
		{{-1.0f, -1.0f,  1.0f}, {-1,  0,  0}, {0, 0}, {}, {}},
		{{-1.0f, -1.0f, -1.0f}, {-1,  0,  0}, {1, 0}, {}, {}},
		{{-1.0f,  1.0f, -1.0f}, {-1,  0,  0}, {1, 1}, {}, {}},
		{{-1.0f,  1.0f, -1.0f}, {-1,  0,  0}, {1, 1}, {}, {}},
		{{-1.0f,  1.0f,  1.0f}, {-1,  0,  0}, {0, 1}, {}, {}},
		{{-1.0f, -1.0f,  1.0f}, {-1,  0,  0}, {0, 0}, {}, {}},

		// Right face (X+)
		{{ 1.0f, -1.0f, -1.0f}, { 1,  0,  0}, {0, 0}, {}, {}},
		{{ 1.0f, -1.0f,  1.0f}, { 1,  0,  0}, {1, 0}, {}, {}},
		{{ 1.0f,  1.0f,  1.0f}, { 1,  0,  0}, {1, 1}, {}, {}},
		{{ 1.0f,  1.0f,  1.0f}, { 1,  0,  0}, {1, 1}, {}, {}},
		{{ 1.0f,  1.0f, -1.0f}, { 1,  0,  0}, {0, 1}, {}, {}},
		{{ 1.0f, -1.0f, -1.0f}, { 1,  0,  0}, {0, 0}, {}, {}},

		// Front face (Z+)
		{{-1.0f, -1.0f,  1.0f}, { 0,  0,  1}, {0, 0}, {}, {}},
		{{-1.0f,  1.0f,  1.0f}, { 0,  0,  1}, {0, 1}, {}, {}},
		{{ 1.0f,  1.0f,  1.0f}, { 0,  0,  1}, {1, 1}, {}, {}},
		{{ 1.0f,  1.0f,  1.0f}, { 0,  0,  1}, {1, 1}, {}, {}},
		{{ 1.0f, -1.0f,  1.0f}, { 0,  0,  1}, {1, 0}, {}, {}},
		{{-1.0f, -1.0f,  1.0f}, { 0,  0,  1}, {0, 0}, {}, {}},

		// Top face (Y+)
		{{-1.0f,  1.0f, -1.0f}, { 0,  1,  0}, {0, 0}, {}, {}},
		{{ 1.0f,  1.0f, -1.0f}, { 0,  1,  0}, {1, 0}, {}, {}},
		{{ 1.0f,  1.0f,  1.0f}, { 0,  1,  0}, {1, 1}, {}, {}},
		{{ 1.0f,  1.0f,  1.0f}, { 0,  1,  0}, {1, 1}, {}, {}},
		{{-1.0f,  1.0f,  1.0f}, { 0,  1,  0}, {0, 1}, {}, {}},
		{{-1.0f,  1.0f, -1.0f}, { 0,  1,  0}, {0, 0}, {}, {}},

		// Bottom face (Y-)
		{{-1.0f, -1.0f, -1.0f}, { 0, -1,  0}, {0, 0}, {}, {}},
		{{-1.0f, -1.0f,  1.0f}, { 0, -1,  0}, {0, 1}, {}, {}},
		{{ 1.0f, -1.0f, -1.0f}, { 0, -1,  0}, {1, 0}, {}, {}},
		{{ 1.0f, -1.0f, -1.0f}, { 0, -1,  0}, {1, 0}, {}, {}},
		{{-1.0f, -1.0f,  1.0f}, { 0, -1,  0}, {0, 1}, {}, {}},
		{{ 1.0f, -1.0f,  1.0f}, { 0, -1,  0}, {1, 1}, {}, {}},
	};

	static std::vector<Mesh::Index> cubeIndices = {
		 0,  1,  2,  3,  4,  5,
		 6,  7,  8,  9, 10, 11,
		12, 13, 14, 15, 16, 17,
		18, 19, 20, 21, 22, 23,
		24, 25, 26, 27, 28, 29,
		30, 31, 32, 33, 34, 35
	};

	AssetHandle<Model> StaticModel::Cube(AssetHandle<Material::Base> mat, bool isInstanced)
	{
		return AssetManager::Put<Model>(Model::Submesh{
			.Mesh = StaticMesh::Cube(),
			.Material = mat
		}, isInstanced);
	}

	AssetHandle<Mesh> StaticMesh::Cube()
	{
		static auto meshHandle = AssetManager::Put<Mesh>(Mesh::CreationInfo{ .Vertices = cubeVertices, .Indices = cubeIndices, .HasNormals = true, .HasTextureCoords = true });

		return meshHandle;
	}

	static std::vector<Mesh::Vertex> quadVertices =
	{
		{{-1.0f, -1.0f, 0.0f}, {-1.0f, -1.0f, 1.0f}, {0.0f, 0.0f}, {}, {}},
		{{ 1.0f, -1.0f, 0.0f}, { 1.0f, -1.0f, 1.0f}, {1.0f, 0.0f}, {}, {}},
		{{ 1.0f,  1.0f, 0.0f}, { 1.0f,  1.0f, 1.0f}, {1.0f, 1.0f}, {}, {}},
		{{-1.0f,  1.0f, 0.0f}, {-1.0f,  1.0f, 1.0f}, {0.0f, 1.0f}, {}, {}}
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

	AssetHandle<Model> StaticModel::Quad(AssetHandle<Material::Base> mat, bool isInstanced)
	{
		return AssetManager::Put<Model>(Model::Submesh{
			.Mesh = StaticMesh::Quad(),
			.Material = mat
		}, isInstanced);
	}

	AssetHandle<Mesh> StaticMesh::Quad()
	{
		static auto meshHandle = AssetManager::Put<Mesh>(Mesh::CreationInfo{ .Vertices = quadVertices, .Indices = quadIndices, .HasNormals = true, .HasTextureCoords = true });

		return meshHandle;
	}

	AssetHandle<Model> StaticModel::QuadTesselated(AssetHandle<Material::Base> mat, u32 w, u32 h)
	{
		std::vector<Mesh::Vertex> vertices;
		std::vector<Mesh::Index> indices;

		for (u32 y = 0; y <= h; y++)
		{
			for (u32 x = 0; x <= w; x++)
			{
				f32 u = static_cast<f32>(x) / w;
				f32 v = static_cast<f32>(y) / h;
				vertices.push_back({ {u, v, 0.0f}, {}, {u,v}, {}, {} });
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

	AssetHandle<Model> StaticModel::Sphere(AssetHandle<Material::Base> mat, bool isInstanced)
	{
		return AssetManager::Put<Model>(Model::Submesh{
			.Mesh = StaticMesh::Sphere(),
			.Material = mat
		}, isInstanced);
	}

	static AssetHandle<Mesh> GetSphere()
	{
		std::vector<Mesh::Vertex> vertices;
		std::vector<Mesh::Index> indices;

		const u32 X_SEGMENTS = 64;
		const u32 Y_SEGMENTS = 64;
		const f32 PI = 3.14159265359f;
		for (u32 x = 0; x <= X_SEGMENTS; ++x)
		{
			for (u32 y = 0; y <= Y_SEGMENTS; ++y)
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
						.TexCoord = {xSegment, ySegment},
						.Tangent = {},
						.BiTangent = {}
					}
				);
			}
		}

		bool oddRow = false;
		for (u32 y = 0; y < Y_SEGMENTS; ++y)
		{
			if (!oddRow) // even rows: y == 0, y == 2; and so on
			{
				for (u32 x = 0; x <= X_SEGMENTS; ++x)
				{
					indices.push_back(y * (X_SEGMENTS + 1) + x);
					indices.push_back((y + 1) * (X_SEGMENTS + 1) + x);
				}
			}
			else
			{
				for (i32 x = X_SEGMENTS; x >= 0; --x)
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

	AssetHandle<Mesh> StaticMesh::Sphere()
	{
		static auto meshHandle = GetSphere();
		return meshHandle;
	}
}
