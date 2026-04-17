#pragma once

#include "renderer/base/Shader.h"
#include "resources/importers/AssetManager.h"
#include "ecs/GameContext.h"
#include "ui/ImGuiEntry.h"
#include "core/Global.h"


namespace EnGl
{
	namespace Material
	{
		struct PlaceholderTextures
		{
			inline static AssetHandle<Texture2D> Roughness;
			inline static AssetHandle<Texture2D> Normals;
			inline static AssetHandle<Texture2D> Metallic;
			inline static AssetHandle<Texture2D> AO;
			inline static AssetHandle<Texture2D> ARM;
			inline static AssetHandle<Texture2D> Opacity;
		};

		struct Base
		{
			virtual bool SetCommonUniforms(const GameContext& context)
			{
				m_Shader = AssetManager::GetAsset(m_ShaderHandle).Asset;

				if (m_Shader)
				{
					m_Shader->Use();
				}

				return m_Shader != nullptr;
			};

			virtual void SetUniforms(Shader* shader) const {};

			void SetModel(Shader* shader, const glm::mat4& model, const glm::mat3x4& normal) const
			{
				shader->SetUniform("uModel", model);
				shader->SetUniform("uNormal", normal);
			}

			inline AssetHandle<Shader> Get() const { return m_ShaderHandle; }

			void Set(const std::filesystem::path& path) { m_ShaderHandle = AssetManager::Load<Shader>(path); }

			virtual void Editor() {}

			virtual ~Base() = default;
		protected:
			Base(const std::filesystem::path& path) : m_ShaderHandle(AssetManager::Load<Shader>(path)) {}

			AssetHandle<Shader> m_ShaderHandle{};
			mutable Shader* m_Shader = nullptr;
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

			void SetUniforms(Shader* shader) const override
			{
				shader->SetUniform("uColor", Color);
			}

			void Editor() override
			{
				ImGui::Separator();
				ImGui::Text("Unlit material");
				ImGui::ColorEdit4("Color", glm::value_ptr(Color));
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

			void SetUniforms(Shader* shader) const override
			{
				auto [texture, gen] = AssetManager::GetAsset(TextureHandle);
				if (texture)
				{
					shader->SetUniform("uTexture", *texture, 0);
				}
			}
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

			void SetUniforms(Shader* shader) const override
			{
				shader->SetUniform("uColor", Color);
			}

			void Editor() override
			{
				ImGui::Separator();
				ImGui::Text("Lit material");
				ImGui::ColorEdit4("Color", glm::value_ptr(Color));
			}
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

			void SetUniforms(Shader* shader) const override
			{
				shader->SetUniform("uMaterial.Albedo", Albedo);
				shader->SetUniform("uMaterial.Metallic", Metallic);
				shader->SetUniform("uMaterial.Roughness", Roughness);
				shader->SetUniform("uMaterial.AO", AO);
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
		};

		struct PBRTextured : public Base
		{
			AssetHandle<Texture2D> AlbedoHandle{};
			AssetHandle<Texture2D> MetallicHandle{};
			AssetHandle<Texture2D> RoughnessHandle{};
			AssetHandle<Texture2D> AOHandle{};
			AssetHandle<Texture2D> NormalsHandle{};

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

				auto shadowMap = AssetManager::GetAsset(context.Framebuffer.DirShadowFramebuffer->Depth()).Asset;
				if (shadowMap)
				{
					m_Shader->SetUniform("uShadowMap", *shadowMap, 5);
					m_Shader->SetUniform("uShadowMapViewProjection", context.Camera.GetDirShadowCamera().ViewProjection);
				}

				return ok;
			}

			void SetUniforms(Shader* shader) const override
			{
				auto albedo = AssetManager::GetAsset(AlbedoHandle).Asset;
				auto metallic = AssetManager::GetAsset(MetallicHandle).Asset;
				auto roughness = AssetManager::GetAsset(RoughnessHandle).Asset;
				auto ao = AssetManager::GetAsset(AOHandle).Asset;
				auto normals = AssetManager::GetAsset(NormalsHandle).Asset;
				
				ao = ao ? ao : AssetManager::GetAsset(PlaceholderTextures::AO).Asset;
				metallic = metallic ? metallic : AssetManager::GetAsset(PlaceholderTextures::Metallic).Asset;
				roughness = roughness ? roughness : AssetManager::GetAsset(PlaceholderTextures::Roughness).Asset;
				normals = normals ? normals : AssetManager::GetAsset(PlaceholderTextures::Normals).Asset;

				if (albedo)
				{
					shader->SetUniform("uMaterial.Albedo", *albedo, 0);
					shader->SetUniform("uMaterial.Metallic", *metallic, 1);
					shader->SetUniform("uMaterial.Roughness", *roughness, 2);
					shader->SetUniform("uMaterial.AO", *ao, 3);
					shader->SetUniform("uMaterial.Normals", *normals , 4);
				}
			}
		};

		struct PBRTexturedARM : public Base
		{
			AssetHandle<Texture2D> AlbedoHandle{};
			AssetHandle<Texture2D> ARMHandle{};
			AssetHandle<Texture2D> NormalsHandle{};

			PBRTexturedARM(bool isInstanced = false) :
				Base(!isInstanced ?
					AssetManager::GRAPHICS_SHADER_DIR / "PBRARM" :
					AssetManager::GRAPHICS_SHADER_DIR / "PBRARM"
				)
			{}

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
					m_Shader->SetUniform("uShadowMap", *shadowMap, 4);
					m_Shader->SetUniform("uShadowMapViewProjection", context.Camera.GetDirShadowCamera().ViewProjection);
				}

				return ok;
			}

			void SetUniforms(Shader* shader) const override
			{
				auto albedo = AssetManager::GetAsset(AlbedoHandle).Asset;
				auto arm = AssetManager::GetAsset(ARMHandle).Asset;
				auto normals = AssetManager::GetAsset(NormalsHandle).Asset;
				normals = normals ? normals : AssetManager::GetAsset(PlaceholderTextures::Normals).Asset;

				shader->SetUniform("uMaterial.Albedo", *albedo, 0);
				shader->SetUniform("uMaterial.ARM", *arm, 1);
				shader->SetUniform("uMaterial.Normals", *normals, 2);
			}
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

			void SetUniforms(Shader* shader) const override
			{
				auto [texture, gen] = AssetManager::GetAsset(TextureHandle);
				if (texture)
				{
					shader->SetUniform("uTexture", *texture, 0);
				}
			}
		};

		struct ScreenSpaceTextured : public Base
		{
			AssetHandle<Texture2D> TextureHandle;

			ScreenSpaceTextured(bool isInstanced = false)
				: Base(
					!isInstanced ?
					AssetManager::GRAPHICS_SHADER_DIR / "ScreenSpaceTextured" :
					AssetManager::GRAPHICS_SHADER_DIR / "ScreenSpaceTexturedInstanced"
				)
			{
			}

			void SetUniforms(Shader* shader) const
			{
				auto [texture, gen] = AssetManager::GetAsset(TextureHandle);
				if (texture)
				{
					shader->SetUniform("uTexture", *texture, 0);
				}
			}
		};

		struct MainQuad : public Base
		{
			AssetHandle<Texture2D> TextureHandle;

			MainQuad(bool isInstanced = false)
				: Base(
					!isInstanced ?
					AssetManager::GRAPHICS_SHADER_DIR / "MainQuad" :
					AssetManager::GRAPHICS_SHADER_DIR / "MainQuad"
				)
			{}

			void SetUniforms(Shader* shader) const
			{
				auto [texture, gen] = AssetManager::GetAsset(TextureHandle);
				if (texture)
				{
					shader->SetUniform("uTexture", *texture, 0);
				}
			}

			bool SetCommonUniforms(const GameContext& context) override
			{
				bool ok = Base::SetCommonUniforms(context);
				auto sky = AssetManager::GetAsset(context.SkyTexture).Asset;
				auto depth = AssetManager::GetAsset(context.Framebuffer.MainFramebuffer->Depth()).Asset;
				auto skyDepth = AssetManager::GetAsset(context.SkyDepthTexture).Asset;

				if (ok && depth && skyDepth)
				{
					m_Shader->SetUniform("uSkyTexture", *sky, 1);
					m_Shader->SetUniform("uDepth", *depth, 2);
					m_Shader->SetUniform("uSkyDepth", *skyDepth, 3);
				}

				m_Shader->SetUniform("uNear", context.Camera.Get().Near);
				m_Shader->SetUniform("uFar", context.Camera.Get().Far);

				return ok;
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

			void SetUniforms(Shader* shader) const override
			{
				auto [textureD, gen1] = AssetManager::GetAsset(DiffuseHandle);
				auto [textureS, gen2] = AssetManager::GetAsset(SpecularHandle);
				if (textureD && textureS)
				{
					shader->SetUniform("uMaterial.Diffuse", *textureD, 0);
					shader->SetUniform("uMaterial.Specular", *textureS, 1);
					shader->SetUniform("uMaterial.Shininess", Shininess);
				}
			}
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
		};
	}
}