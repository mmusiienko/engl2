#include "../AssetImporter.h"


namespace EnGl
{
	static std::vector<Shader::ShaderUnit> ParseUnits(const std::filesystem::path& directory);

	Shader AssetImporter<Shader>::Import(const Params& params)
	{
		return Shader{ ParseUnits(params.Directory) };
	}

	ComputeShader AssetImporter<ComputeShader>::Import(const Params& params)
	{
		return ComputeShader{ ParseUnits(params.Directory) };
	}

	static std::vector<Shader::ShaderUnit> ParseUnits(const std::filesystem::path& directory)
	{
		spdlog::info("Reading shader data at {}", directory.string());

		static const std::unordered_map<std::string, GLenum> SHADER_EXTENSION_TO_TYPE
		{
			{".vert", GL_VERTEX_SHADER},
			{".frag", GL_FRAGMENT_SHADER},
			{".comp", GL_COMPUTE_SHADER},
			{".tesc", GL_TESS_CONTROL_SHADER},
			{".tese", GL_TESS_EVALUATION_SHADER},
		};

		std::vector<Shader::ShaderUnit> out;

		for (const auto& entry : std::filesystem::directory_iterator(directory))
		{
			if (!entry.is_regular_file()) continue;
			auto extenstion = entry.path().extension().string();

			if (!SHADER_EXTENSION_TO_TYPE.contains(extenstion)) continue;

			spdlog::info("Reading shader unit at {}", entry.path().string());

			auto shaderSource = FileSystem::ReadFile(entry);
			auto shaderType = SHADER_EXTENSION_TO_TYPE.at(extenstion);

			out.emplace_back
			(
				shaderType,
				std::move(shaderSource)
			);
		}

		return out;
	}
}