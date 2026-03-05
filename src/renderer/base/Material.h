#pragma once
#include "Shader.h"
#include "../../resources/importers/AssetManager.h"
#include "../../ecs/GameContext.h"
#include "../../Printer.h"
#include "../../ui/ImGuiEntry.h"

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

			void SetModel(const glm::mat4& model, const glm::mat3x4& normal) const
			{
				m_Shader->SetUniform("uModel", model);
				m_Shader->SetUniform("uNormal", normal);
			}

			inline AssetHandle<Shader> Get() const { return m_ShaderHandle; }
			void Set(const std::filesystem::path& path) { m_ShaderHandle = AssetManager::Load<Shader>(path); }
			virtual const std::string& Name() const = 0;

			virtual void Editor() {}
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
				)
			{
			}

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

			const std::string& Name() const override { return "Unlit"; };
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

			const std::string& Name() const override { return "UnlitTextured"; };
		};

		struct Lit : public Base
		{
			glm::vec4 Color{ 0.4f, 0.4f, 0.4f, 1.0f };

			Lit(bool isInstanced = false) : 
				Base(!isInstanced ?
					AssetManager::GRAPHICS_SHADER_DIR / "Lit" :
					AssetManager::GRAPHICS_SHADER_DIR / "Lit"
				)
			{
			}

			bool SetCommonUniforms(const GameContext& context) override
			{
				bool ok = Base::SetCommonUniforms(context);
				if (!ok)
				{
					return ok;
				}

				m_Shader->SetUniform("uViewProjection", context.Camera.Get().ViewProjection);
				m_Shader->SetUniform("uCameraPos", *context.Camera.Get().Position);
				m_Shader->SetUniform("uDirectionalLight", context.DirLight);
				m_Shader->SetUniform("uPointLights", context.PointLights);

				return ok;
			}

			void SetUniforms() const override
			{
				m_Shader->SetUniform("uColor", Color);
			}

			const std::string& Name() const override { return "Lit"; };
		};

		struct PBR : public Base
		{
			glm::vec3 Albedo { 1.0f, 0.4f, 0.4f };
			f32 Metallic = 0.0f;
			f32 Roughness = 0.2f;
			f32 AO = 0.0f;

			PBR(bool isInstanced = false) :
				Base(!isInstanced ?
					AssetManager::GRAPHICS_SHADER_DIR / "PBR" :
					AssetManager::GRAPHICS_SHADER_DIR / "PBR"
				)
			{
			}

			bool SetCommonUniforms(const GameContext& context) override
			{
				bool ok = Base::SetCommonUniforms(context);
				if (!ok)
				{
					return ok;
				}

				m_Shader->SetUniform("uViewProjection", context.Camera.Get().ViewProjection);
				m_Shader->SetUniform("uCameraPos", *context.Camera.Get().Position);
				m_Shader->SetUniform("uDirectionalLight", context.DirLight);
				m_Shader->SetUniform("uPointLights", context.PointLights);

				auto shadowMap = AssetManager::GetAsset(context.Framebuffer.DirShadowFramebuffer->Depth()).Asset;
				if (shadowMap)
				{
					m_Shader->SetUniform("uShadowMap", *shadowMap, 0);
					m_Shader->SetUniform("uShadowMapViewProjection", context.Camera.GetDirShadowCamera().ViewProjection);
				}

				return ok;
			}

			void SetUniforms() const override
			{
				m_Shader->SetUniform("uMaterial.Albedo", Albedo);
				m_Shader->SetUniform("uMaterial.Metallic", Metallic);
				m_Shader->SetUniform("uMaterial.Roughness", Roughness);
				m_Shader->SetUniform("uMaterial.AO", AO);
			}

			void Editor() override
			{
				ImGui::Separator();
				ImGui::Text("PBR material");
				ImGui::ColorEdit3("Albedo", glm::value_ptr(Albedo));
				ImGui::InputFloat("Metallic", &Metallic);
				ImGui::InputFloat("Roughness", &Roughness);
				ImGui::InputFloat("Ambient", &AO);
			}

			const std::string& Name() const override { return "PBR"; };
		};

		struct PBRTextured : public Base
		{
			AssetHandle<Texture2D> AlbedoHandle{};
			AssetHandle<Texture2D> MetallicHandle{};
			AssetHandle<Texture2D> RoughnessHandle{};
			AssetHandle<Texture2D> AOHandle{};

			PBRTextured(bool isInstanced = false) :
				Base(!isInstanced ?
					AssetManager::GRAPHICS_SHADER_DIR / "PBRTextured" :
					AssetManager::GRAPHICS_SHADER_DIR / "PBRTextured"
				)
			{
			}

			bool SetCommonUniforms(const GameContext& context) override
			{
				bool ok = Base::SetCommonUniforms(context);
				if (!ok)
				{
					return ok;
				}

				m_Shader->SetUniform("uViewProjection", context.Camera.Get().ViewProjection);
				m_Shader->SetUniform("uCameraPos", *context.Camera.Get().Position);
				m_Shader->SetUniform("uDirectionalLight", context.DirLight);
				m_Shader->SetUniform("uPointLights", context.PointLights);

				return ok;
			}

			void SetUniforms() const override
			{
				auto [albedo, g0] = AssetManager::GetAsset(AlbedoHandle);
				auto [metallic, g1] = AssetManager::GetAsset(MetallicHandle);
				auto [roughness, g2] = AssetManager::GetAsset(RoughnessHandle);
				auto [ao, g3] = AssetManager::GetAsset(AOHandle);
				
				if (albedo && roughness && metallic)
				{
					m_Shader->SetUniform("uMaterial.Albedo", *albedo, 0);
					m_Shader->SetUniform("uMaterial.Metallic", *metallic, 1);
					m_Shader->SetUniform("uMaterial.Roughness", *roughness, 2);
					if (ao)
					{
						m_Shader->SetUniform("uMaterial.AO", *ao, 3);
					}
				}
			}

			const std::string& Name() const override { return "PBRTextured"; };
		};

		struct LitTextured : public Base
		{
			AssetHandle<Texture2D> TextureHandle;

			LitTextured(AssetHandle<Texture2D> texture, bool isInstanced = false)
				: TextureHandle(texture),
				Base(!isInstanced ?
					AssetManager::GRAPHICS_SHADER_DIR / "LitTextured" :
					AssetManager::GRAPHICS_SHADER_DIR / "LitTexturedInstanced"
				)
			{
			}

			bool SetCommonUniforms(const GameContext& context) override
			{
				bool ok = Base::SetCommonUniforms(context);
				if (!ok)
				{
					return ok;
				}

				m_Shader->SetUniform("uViewProjection", context.Camera.Get().ViewProjection);
				m_Shader->SetUniform("uCameraPos", *context.Camera.Get().Position);
				m_Shader->SetUniform("uDirectionalLight", context.DirLight);
				m_Shader->SetUniform("uPointLights", context.PointLights);

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

			const std::string& Name() const override { return "LitTextured"; };
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

			bool SetCommonUniforms(const GameContext& context) override
			{
				bool ok = Base::SetCommonUniforms(context);
				auto [sky, g1] = AssetManager::GetAsset(context.SkyTexture);
				if (ok && sky)
				{
					m_Shader->SetUniform("uSkyTexture", *sky, 1);
				}


				return ok;
			}

			const std::string& Name() const override { return "ScreenSpaceTextured"; };
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
				if (!ok)
				{
					return ok;
				}

				m_Shader->SetUniform("uViewProjection", context.Camera.Get().ViewProjection);
				m_Shader->SetUniform("uCameraPos", *context.Camera.Get().Position);
				m_Shader->SetUniform("uDirectionalLight", context.DirLight);
				m_Shader->SetUniform("uPointLights", context.PointLights);

				return ok;
			}

			void SetUniforms() const override
			{
				auto [textureD, gen1] = AssetManager::GetAsset(DiffuseHandle);
				auto [textureS, gen2] = AssetManager::GetAsset(SpecularHandle);
				if (textureD && textureS)
				{
					m_Shader->SetUniform("uMaterial.Diffuse", *textureD, 0);
					m_Shader->SetUniform("uMaterial.Specular", *textureS, 1);
					m_Shader->SetUniform("uMaterial.Shininess", Shininess);
				}
			}

			const std::string& Name() const override { return "Phong"; };
		};

		struct CoordinatePlane : public Base
		{
			CoordinatePlane() : Base(AssetManager::GRAPHICS_SHADER_DIR / "CoordinatePlane") {}

			bool SetCommonUniforms(const GameContext& context) override
			{
				bool ok = Base::SetCommonUniforms(context);
				if (ok)
				{
					m_Shader->SetUniform("uViewProjection", context.Camera.Get().ViewProjection);
					m_Shader->SetUniform("uCameraPos", *context.Camera.Get().Position);
				}

				return ok;
			}

			const std::string& Name() const override { return "CoordinatePlane"; };
		};

		struct CubemapObj : public Base
		{
			AssetHandle<Cubemap> CubemapHandle;

			CubemapObj(AssetHandle<Cubemap> cubemapHandle) : Base(AssetManager::GRAPHICS_SHADER_DIR / "Cubemap"), CubemapHandle(cubemapHandle) {}

			bool SetCommonUniforms(const GameContext& context) override
			{
				bool ok = Base::SetCommonUniforms(context);
				auto [cubemap, g] = AssetManager::GetAsset(CubemapHandle);

				if (ok && cubemap)
				{
					m_Shader->SetUniform("uView", glm::mat4(glm::mat3(*context.Camera.Get().View)));
					m_Shader->SetUniform("uProjection", *context.Camera.Get().Projection);
					m_Shader->SetUniform("uCameraPos", *context.Camera.Get().Position);
					m_Shader->SetUniform("uCubemap", *cubemap, 2);
				}

				return ok;
			}

			const std::string& Name() const override { return "Cubemap"; };
		};
	}
}