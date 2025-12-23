#pragma once
#include <string>
#include <vector>

#include "../math/Math.h"
#include "Resource.h"


namespace EnGl 
{
	class Texture;
	class SSBO;

	class Shader : public Resource
	{
	public:

		struct ShaderUnit
		{
			u32 Type = GL_VERTEX_SHADER;
			std::vector<char> Source;
		};

		Shader(const std::vector<ShaderUnit>& shaderUnits);

		Shader(Shader&& other) noexcept = default;
		Shader& operator=(Shader&& other) noexcept = default;

		static u32 CompileShader(u32 shaderType, const std::vector<char>& shaderSource);

		void Use() const;



		u32 GetLocation(const std::string& name) const;
		void SetUniform(const std::string& name, i32 value) const;
		void SetUniform(const std::string& name, u32 value) const;
		void SetUniform(const std::string& name, f32 value) const;
		void SetUniform(const std::string& name, f64 value) const;
		void SetUniform(const std::string& name, bool value) const;		
		void SetUniform(const std::string& name, const glm::mat4& value) const;
		void SetUniform(const std::string& name, const glm::mat4x3& value) const;
		void SetUniform(const std::string& name, const glm::vec4& value) const;
		void SetUniform(const std::string& name, const glm::vec3& value) const;
		void SetUniform(const std::string& name, const glm::vec2& value) const;
		void SetUniform(const std::string& name, const Texture& value, u32 unit) const;
		void SetUniform(const std::string& name, void*) = delete;
		void SetUniform(const std::string& name, const void*) = delete;
		void SetUniform(const std::string& name, std::nullptr_t) = delete;

		void BindTextureUnit(const Texture& tex, u32 unit) const;
		void BindSSBO(const SSBO& ssbo, u32 unit) const;

		virtual ~Shader();
	};
}