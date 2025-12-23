#pragma once
#include "Shader.h"
#include "../../resources/importers/AssetManager.h"
#include "../../ecs/GameContext.h"
#include "../../Printer.h"


namespace EnGl
{
	namespace Material
	{
		struct Base
		{
			virtual bool SetCommonUniforms(const GameContext& context)
			{
				auto [shader, gen] = AssetManager::GetAsset(m_ShaderHandle);

				m_Shader = shader;

				if (m_Shader)
				{
					m_Shader->Use();
				}

				return m_Shader != nullptr;
			};

			virtual void SetUniforms() const {};

			void SetModel(const glm::mat4& model) const
			{
				m_Shader->SetUniform("uModel", model);
			}

			inline AssetHandle<Shader> Get() const { return m_ShaderHandle; }
			void Set(const std::filesystem::path& path) { m_ShaderHandle = AssetManager::Load<Shader>(path); }
		protected:
			Base(const std::filesystem::path& path) : m_ShaderHandle(AssetManager::Load<Shader>(path)) {}

			Shader* m_Shader = nullptr;
			AssetHandle<Shader> m_ShaderHandle;
		};

		struct Unlit : public Base
		{
			glm::vec4 Color{ 1.0f, 0.0f, 0.0f, 1.0f };

			Unlit(bool instanced = false) : 
				Base(!instanced ? 
					(AssetManager::GRAPHICS_SHADER_DIR / "Unlit") :
					(AssetManager::GRAPHICS_SHADER_DIR / "UnlitInstanced")
				) {}

			Unlit(glm::vec4 color, bool instanced = false)
				: Unlit(instanced)
			{
				Color = std::move(color);
			}

			bool SetCommonUniforms(const GameContext& context) override
			{
				bool ok = Base::SetCommonUniforms(context);

				if (ok)
				{
					m_Shader->SetUniform("uViewProjection", context.Camera.Get().ViewProjection);
				}
				
				return ok;
			}

			void SetUniforms() const override
			{
				m_Shader->SetUniform("uColor", Color);
			}
		};

		struct UnlitTextured : public Base
		{
			AssetHandle<Texture2D> TextureHandle;

			UnlitTextured(AssetHandle<Texture2D> texture, bool isInstanced = false)
				: TextureHandle(texture), 
				Base(!isInstanced ? 
					AssetManager::GRAPHICS_SHADER_DIR / "UnlitTextured" :
					AssetManager::GRAPHICS_SHADER_DIR / "UnlitTexturedInstanced"
				)
			{
			}

			bool SetCommonUniforms(const GameContext& context) override
			{
				bool ok = Base::SetCommonUniforms(context);
				if (ok)
				{
					m_Shader->SetUniform("uViewProjection", context.Camera.Get().ViewProjection);
				}
				return ok;
			}

			void SetUniforms() const override
			{
				auto [texture, gen] = AssetManager::GetAsset(TextureHandle);
				if (texture)
				{
					m_Shader->SetUniform("uTexture", *texture, 0);
				}
			}
		};

		struct ScreenSpaceTextured : public Base
		{
			AssetHandle<Texture2D> TextureHandle;

			ScreenSpaceTextured(AssetHandle<Texture2D> texture, bool isInstanced = false)
				: TextureHandle(texture), Base(
					!isInstanced ? 
					AssetManager::GRAPHICS_SHADER_DIR / "ScreenSpaceTextured" :
					AssetManager::GRAPHICS_SHADER_DIR / "ScreenSpaceTexturedInstanced"
				)
			{
			}

			void SetUniforms() const
			{
				auto [texture, gen] = AssetManager::GetAsset(TextureHandle);
				if (texture)
				{
					m_Shader->SetUniform("uTexture", *texture, 0);
				}
			}
		};

		struct Phong : public Base
		{
			AssetHandle<Texture2D> DiffuseHandle;
			AssetHandle<Texture2D> SpecularHandle;
			f32 Shininess = 0.0f;

			Phong(AssetHandle<Texture2D> diffuseHandle, AssetHandle<Texture2D> specularHandle, f32 shininess = 1, bool isInstanced = false)
				: DiffuseHandle(diffuseHandle),
				SpecularHandle(specularHandle),
				Shininess(shininess),
				Base(!isInstanced ? 
					AssetManager::GRAPHICS_SHADER_DIR / "Phong" :
					AssetManager::GRAPHICS_SHADER_DIR / "PhongInstanced"
				)
			{
			}

			bool SetCommonUniforms(const GameContext& context) override
			{
				bool ok = Base::SetCommonUniforms(context);
				if (ok)
				{
					m_Shader->SetUniform("uViewProjection", context.Camera.Get().ViewProjection);
				}

				return ok;
			}

			void SetUniforms() const override
			{
				auto [textureD, gen1] = AssetManager::GetAsset(DiffuseHandle);
				auto [textureS, gen2] = AssetManager::GetAsset(SpecularHandle);
				if (textureD && textureS)
				{
					m_Shader->SetUniform("uMaterial.uDiffuse", *textureD, 0);
					m_Shader->SetUniform("uMaterial.uSpecular", *textureS, 1);
					m_Shader->SetUniform("uMaterial.uShininness", Shininess);
				}
			}
		};

		struct CoordinatePlane : public Base
		{
			CoordinatePlane() : Base(AssetManager::GRAPHICS_SHADER_DIR / "CoordinatePlane") {}

			bool SetCommonUniforms(const GameContext & context) override
			{
				bool ok = Base::SetCommonUniforms(context);
				if (ok)
				{
					m_Shader->SetUniform("uViewProjection", context.Camera.Get().ViewProjection);
					m_Shader->SetUniform("uCameraPos", *context.Camera.Get().Position);
				}

				return ok;
			}
		};
	}
}