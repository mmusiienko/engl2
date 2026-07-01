#include <stdexcept>

#include "renderer/base/Shader.h"
#include "renderer/base/SSBO.h"
#include "renderer/base/Texture.h"

#include "math/Math.h"
#include "core/Core.h"


namespace EnGl 
{
	Shader::Shader(const std::vector<ShaderUnit>& shaderUnits)
	{
		if (shaderUnits.empty())
		{
			spdlog::error("Empty shader created");
			return;
		}

		GL_CHECK(m_Id = glCreateProgram());

		std::vector<u32> toDelete;

		for (const auto& [type, source] : shaderUnits)
		{
			u32 shader = CompileShader(type, source);

			toDelete.push_back(shader);
			GL_CHECK(glAttachShader(m_Id, shader));
		}

		GL_CHECK(glLinkProgram(m_Id));

		int  success;
		char infoLog[512];
		GL_CHECK(glGetProgramiv(m_Id, GL_LINK_STATUS, &success));

		if (!success)
		{
			GL_CHECK(glGetProgramInfoLog(m_Id, 512, NULL, infoLog));

			spdlog::error(infoLog);

			throw std::runtime_error("Failed to link");
		}

		for (u32 id : toDelete)
		{
			GL_CHECK(glDeleteShader(id));
		}
	}

	Shader::~Shader()
	{
		GL_CHECK(glDeleteProgram(m_Id));
	}

	u32 Shader::CompileShader(u32 shaderType, const std::vector<char>& shaderSource)
	{
		u32 shader;
		GL_CHECK(shader = glCreateShader(shaderType));

		const char* shaderData = shaderSource.data();
		GL_CHECK(glShaderSource(shader, 1, &shaderData, NULL));
		GL_CHECK(glCompileShader(shader));

		int  success;
		char infoLog[512];
		GL_CHECK(glGetShaderiv(shader, GL_COMPILE_STATUS, &success));

		if (!success)
		{
			GL_CHECK(glGetShaderInfoLog(shader, 512, NULL, infoLog));
			throw std::runtime_error(infoLog);
		}

		return shader;
	}

	u32 Shader::GetLocation(std::string_view name) const
	{
		auto it = m_LocationCache.find(name);
		if (it != m_LocationCache.end())
			return it->second;

		
		u32 loc;
		auto nameStr = std::string{ name };
		GL_CHECK(loc = glGetUniformLocation(m_Id, nameStr.c_str()));
		m_LocationCache.insert({ std::move(nameStr), loc });

		return loc;
	}

	void Shader::SetUniform(std::string_view name, i32 value) const
	{
		GL_CHECK( glUniform1i(GetLocation(name), value) );
	}

	void Shader::SetUniform(std::string_view name, u32 value) const
	{
		GL_CHECK(glUniform1ui(GetLocation(name), value));
	}

	void Shader::SetUniform(std::string_view name, f32 value) const
	{
		GL_CHECK( glUniform1f(GetLocation(name), value) );
	}

	void Shader::SetUniform(std::string_view name, f64 value) const
	{
		GL_CHECK(glUniform1d(GetLocation(name), value));
	}

	void Shader::SetUniform(std::string_view name, bool value) const
	{
		SetUniform(name, static_cast<u32> (value));
	}

	void Shader::SetUniform(std::string_view name, const glm::mat4& value) const
	{
		GL_CHECK( glUniformMatrix4fv(GetLocation(name), 1, GL_FALSE, glm::value_ptr(value)) );
	}

	void Shader::SetUniform(std::string_view name, const glm::mat4x3& value) const
	{
		GL_CHECK(glUniformMatrix4x3fv(GetLocation(name), 1, GL_FALSE, glm::value_ptr(value)));
	}

	void Shader::SetUniform(std::string_view name, const glm::mat3x4& value) const
	{
		GL_CHECK(glUniformMatrix3x4fv(GetLocation(name), 1, GL_FALSE, glm::value_ptr(value)));
	}

	void Shader::SetUniform(std::string_view name, const glm::mat3& value) const
	{
		GL_CHECK(glUniformMatrix3fv(GetLocation(name), 1, GL_FALSE, glm::value_ptr(value)));
	}

	void Shader::SetUniform(std::string_view name, const glm::vec3& value) const
	{
		GL_CHECK( glUniform3f(GetLocation(name), value.x, value.y, value.z) );
	}

	void Shader::SetUniform(std::string_view name, const glm::vec4& value) const
	{
		GL_CHECK(glUniform4f(GetLocation(name), value.x, value.y, value.z, value.w));
	}

	void Shader::SetUniform(std::string_view name, const glm::vec2& value) const
	{
		GL_CHECK(glUniform2f(GetLocation(name), value.x, value.y));
	}

	void Shader::SetUniform(std::string_view name, const glm::uvec2& value) const
	{
		GL_CHECK(glUniform2ui(GetLocation(name), value.x, value.y));
	}

	void Shader::SetUniform(std::string_view name, const UniformDirectionalLight& value) const
	{
		SetUniform(std::format("{}.Direction", name), value.Direction);
		SetUniform(std::format("{}.Color", name), value.Color);
	}

	void Shader::SetUniform(std::string_view name, const std::array<UniformPointLight, MAX_LIGHTS>& value) const
	{
		for (size_t i = 0; i < value.size(); i++)
		{
			SetUniform(std::format("{}[{}].Position", name, i), value[i].Position);
			SetUniform(std::format("{}[{}].Color", name, i), value[i].Color);
			SetUniform(std::format("{}[{}].Intensity", name, i), value[i].Intensity);
		}
	}

	void Shader::BindTextureUnit(const Texture& tex, u32 unit) const
	{
		GL_CHECK(glBindTextureUnit(unit, tex.Id()));
	}

	void Shader::SetUniform(std::string_view name, const Texture& value, u32 unit) const
	{
		SetUniform(name, static_cast<i32>(unit));
		GL_CHECK(glBindTextureUnit(unit, value.Id()));
	}

	void Shader::Use() const
	{
		GL_CHECK( glUseProgram(m_Id) );
	}

	void Shader::BindSSBO(const SSBO& ssbo, u32 unit) const
	{
		GL_CHECK(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, unit, ssbo.Id()));
	}
}

