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
		struct UniformDirectionalLight
		{
			glm::vec3 Direction;
			glm::vec3 Color;
		};

		static constexpr size_t MAX_LIGHTS = 16;
		struct UniformPointLight
		{
			glm::vec3 Position;
			glm::vec3 Color;
			f32 Intensity;
		};

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

		u32 GetLocation(std::string_view name) const;
		void SetUniform(std::string_view name, i32 value) const;
		void SetUniform(std::string_view name, u32 value) const;
		void SetUniform(std::string_view name, f32 value) const;
		void SetUniform(std::string_view name, f64 value) const;
		void SetUniform(std::string_view name, bool value) const;		
		void SetUniform(std::string_view name, const glm::mat4& value) const;
		void SetUniform(std::string_view name, const glm::mat4x3& value) const;
		void SetUniform(std::string_view name, const glm::mat3x4& value) const;
		void SetUniform(std::string_view name, const glm::vec4& value) const;
		void SetUniform(std::string_view name, const glm::vec3& value) const;
		void SetUniform(std::string_view name, const glm::vec2& value) const;
		void SetUniform(std::string_view name, const glm::uvec2& value) const;
		void SetUniform(std::string_view name, const UniformDirectionalLight& value) const;
		void SetUniform(std::string_view name, const std::array<UniformPointLight, MAX_LIGHTS>& value) const;
		void SetUniform(std::string_view name, const Texture& value, u32 unit) const;
		void SetUniform(std::string_view name, void*) = delete;
		void SetUniform(std::string_view name, const void*) = delete;
		void SetUniform(std::string_view name, std::nullptr_t) = delete;

		void BindTextureUnit(const Texture& tex, u32 unit) const;
		void BindSSBO(const SSBO& ssbo, u32 unit) const;

		virtual ~Shader();

	private:
		struct StringHash 
		{
			using is_transparent = void;
			size_t operator()(std::string_view sv) const
			{
				return std::hash<std::string_view>{}(sv);
			}
		};

		mutable std::unordered_map<std::string, u32, StringHash, std::equal_to<>> m_LocationCache;
	};
}