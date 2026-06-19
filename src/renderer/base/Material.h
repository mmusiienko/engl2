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
			virtual void SetCommonUniforms(Shader* shader, const GameContext& context)
			{
				shader->Use();
			};

			virtual void SetCommonUniformsUnlit(Shader* shader, const GameContext& context)
			{
				return SetCommonUniforms(shader, context);
			};

			virtual void SetUniforms(Shader* shader) const {};

			virtual void SetUniformsUnlit(Shader* shader) const { SetUniforms(shader); }

			void SetModel(Shader* shader, const glm::mat4& model, const glm::mat3x4& normal) const
			{
				shader->SetUniform("uModel", model);
				shader->SetUniform("uNormal", normal);
			}

			void SetModelUnlit(Shader* shader, const glm::mat4& model, const glm::mat3x4& normal) const
			{
				shader->SetUniform("uModel", model);
			}

			inline AssetHandle<Shader> Get() const { return m_ShaderHandle; }
			inline AssetHandle<Shader> GetUnlit() const { return m_UnlitShaderHandle; }
			inline AssetHandle<Shader> GetAnimated() const { return m_UnlitShaderHandle; }

			void Set(const std::filesystem::path& path) { m_ShaderHandle = AssetManager::Load<Shader>(path); }
			void SetUnlit(const std::filesystem::path& path) { m_UnlitShaderHandle = AssetManager::Load<Shader>(path); }

			virtual void Editor() {}

			virtual ~Base() = default;

			
		protected:
			Base(const std::filesystem::path& path) : m_ShaderHandle(AssetManager::Load<Shader>(path)) {}

			AssetHandle<Shader> m_ShaderHandle{};
			AssetHandle<Shader> m_UnlitShaderHandle = AssetManager::Load<Shader>(AssetManager::GRAPHICS_SHADER_DIR / "Unlit");
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

			void SetCommonUniforms(Shader* shader, const GameContext& context) override
			{
				Base::SetCommonUniforms(shader, context);
				shader->SetUniform("uViewProjection", context.Camera.Get().ViewProjection);
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

			void SetCommonUniforms(Shader* shader, const GameContext& context) override
			{
				Base::SetCommonUniforms(shader, context);
				shader->SetUniform("uViewProjection", context.Camera.Get().ViewProjection);
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

			void SetCommonUniforms(Shader* shader, const GameContext& context) override
			{
				Base::SetCommonUniforms(shader, context);
				shader->SetUniform("uViewProjection", context.Camera.Get().ViewProjection);
				shader->SetUniform("uCameraPos", context.Camera.Get().Position);
				shader->SetUniform("uNear", context.Camera.Get().Near);
				shader->SetUniform("uDirectionalLight", context.DirLight.Data);
				shader->SetUniform("uPointLights", context.PointLights);
				shader->SetUniform("uResolution", context.Framebuffer.MainFramebuffer->Resolution());

				auto shadowMap = AssetManager::GetAsset(context.Renderer.CascadedShadowMap.ShadowMap).Asset;
				if (shadowMap)
				{
					shader->SetUniform("uShadowMap", *shadowMap, 0);
				}
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

			void SetCommonUniforms(Shader* shader, const GameContext& context) override
			{
				Base::SetCommonUniforms(shader, context);
				shader->SetUniform("uViewProjection", context.Camera.Get().ViewProjection);
				shader->SetUniform("uCameraPos", context.Camera.Get().Position);
				shader->SetUniform("uDirectionalLight", context.DirLight.Data);
				shader->SetUniform("uPointLights", context.PointLights);

				auto shadowMap = AssetManager::GetAsset(context.Renderer.CascadedShadowMap.ShadowMap).Asset;
				if (shadowMap)
				{
					shader->SetUniform("uShadowMap", *shadowMap, 0);
				}
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

			void SetCommonUniforms(Shader* shader, const GameContext& context) override
			{
				Base::SetCommonUniforms(shader, context);
				shader->SetUniform("uViewProjection", context.Camera.Get().ViewProjection);
				shader->SetUniform("uCameraPos", context.Camera.Get().Position);
				shader->SetUniform("uDirectionalLight", context.DirLight.Data);
				shader->SetUniform("uPointLights", context.PointLights);
				shader->SetUniform("uResolution", context.Framebuffer.MainFramebuffer->Resolution());

				auto shadowMap = AssetManager::GetAsset(context.Renderer.CascadedShadowMap.ShadowMap).Asset;
				if (shadowMap)
				{
					shader->SetUniform("uShadowMap", *shadowMap, 5);
				}
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


			void SetCommonUniforms(Shader* shader, const GameContext& context) override
			{
				Base::SetCommonUniforms(shader, context);
				shader->SetUniform("uViewProjection", context.Camera.Get().ViewProjection);
				shader->SetUniform("uCameraPos", context.Camera.Get().Position);
				shader->SetUniform("uDirectionalLight", context.DirLight.Data);
				shader->SetUniform("uPointLights", context.PointLights);
				shader->SetUniform("uResolution", context.Framebuffer.MainFramebuffer->Resolution());

				auto shadowMap = AssetManager::GetAsset(context.Renderer.CascadedShadowMap.ShadowMap).Asset;
				if (shadowMap)
				{
					shader->SetUniform("uShadowMap", *shadowMap, 3);
				}
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

			void SetCommonUniforms(Shader* shader, const GameContext& context) override
			{
				Base::SetCommonUniforms(shader, context);
				shader->SetUniform("uViewProjection", context.Camera.Get().ViewProjection);
				shader->SetUniform("uCameraPos", context.Camera.Get().Position);
				shader->SetUniform("uDirectionalLight", context.DirLight.Data);
				shader->SetUniform("uPointLights", context.PointLights);
				shader->SetUniform("uResolution", context.Framebuffer.MainFramebuffer->Resolution());

				auto shadowMap = AssetManager::GetAsset(context.Renderer.CascadedShadowMap.ShadowMap).Asset;
				if (shadowMap)
				{
					shader->SetUniform("uShadowMap", *shadowMap, 1);
				}
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

			void SetCommonUniforms(Shader* shader, const GameContext& context) override
			{
				Base::SetCommonUniforms(shader, context);
				auto sky = AssetManager::GetAsset(context.SkyTexture).Asset;
				auto depth = AssetManager::GetAsset(context.Framebuffer.MainFramebuffer->Depth()).Asset;

				if (depth)
				{
					shader->SetUniform("uDepth", *depth, 1);
				}

				if (sky)
				{
					shader->SetUniform("uSkyTexture", *sky, 2);
				}


				shader->SetUniform("uDirectionalLight", context.DirLight.Data);
				shader->SetUniform("uNear", context.Camera.Get().Near);
			}
		};

		struct Phong : public Base
		{
			AssetHandle<Texture2D> DiffuseHandle{};
			AssetHandle<Texture2D> SpecularHandle{};
			AssetHandle<Texture2D> NormalsHandle{};
			f32 Shininess = 0.0f;

			Phong(bool isInstanced = false) :
				Base(!isInstanced ?
					AssetManager::GRAPHICS_SHADER_DIR / "Phong" :
					AssetManager::GRAPHICS_SHADER_DIR / "PhongInstanced"
				)
			{
			}

			void SetCommonUniforms(Shader* shader, const GameContext& context) override
			{
				Base::SetCommonUniforms(shader, context);
				shader->SetUniform("uViewProjection", context.Camera.Get().ViewProjection);
				shader->SetUniform("uCameraPos", context.Camera.Get().Position);
				shader->SetUniform("uDirectionalLight", context.DirLight.Data);
				shader->SetUniform("uPointLights", context.PointLights);

				shader->SetUniform("uResolution", context.Framebuffer.MainFramebuffer->Resolution());

				auto shadowMap = AssetManager::GetAsset(context.Renderer.CascadedShadowMap.ShadowMap).Asset;
				if (shadowMap)
				{
					shader->SetUniform("uShadowMap", *shadowMap, 3);
				}
			}

			void SetUniforms(Shader* shader) const override
			{
				auto textureD = AssetManager::GetAsset(DiffuseHandle).Asset;
				auto textureS = AssetManager::GetAsset(SpecularHandle).Asset;
				auto normals = AssetManager::GetAsset(NormalsHandle).Asset;
				
				if (textureD && textureS)
				{
					shader->SetUniform("uMaterial.Diffuse", *textureD, 0);
					shader->SetUniform("uMaterial.Specular", *textureS, 1);
					shader->SetUniform("uMaterial.Shininess", Shininess);
					normals = normals ? normals : AssetManager::GetAsset(PlaceholderTextures::Normals).Asset;
					shader->SetUniform("uMaterial.Normals", *normals, 2);
				}
			}
		};

		struct CoordinatePlane : public Base
		{
			CoordinatePlane() : Base(AssetManager::GRAPHICS_SHADER_DIR / "CoordinatePlane") {}

			void SetCommonUniforms(Shader* shader, const GameContext& context) override
			{
				Base::SetCommonUniforms(shader, context);
				shader->SetUniform("uViewProjection", context.Camera.Get().ViewProjection);
				shader->SetUniform("uCameraPos", context.Camera.Get().Position);
			}
		};

		struct CubemapObj : public Base
		{
			AssetHandle<Cubemap> CubemapHandle;

			CubemapObj(AssetHandle<Cubemap> cubemapHandle) : Base(AssetManager::GRAPHICS_SHADER_DIR / "Cubemap"), CubemapHandle(cubemapHandle) {}

			void SetCommonUniforms(Shader* shader, const GameContext& context) override
			{
				Base::SetCommonUniforms(shader, context);
				auto cubemap = AssetManager::GetAsset(CubemapHandle).Asset;

				if (cubemap)
				{
					shader->SetUniform("uViewProjection", context.Camera.Get().Projection * glm::mat4{ glm::mat3{context.Camera.Get().View } });
					shader->SetUniform("uCameraPos", context.Camera.Get().Position);
					shader->SetUniform("uCubemap", *cubemap, 2);
				}
			}
		};

		struct PBRTexturedARMAnimated : public PBRTexturedARM
		{
			PBRTexturedARMAnimated(bool isInstanced = false) : PBRTexturedARM(isInstanced)
			{
				m_ShaderHandle = AssetManager::Load<Shader>(AssetManager::GRAPHICS_SHADER_DIR / "PBRARMAnimated");
				m_UnlitShaderHandle = AssetManager::Load<Shader>(AssetManager::GRAPHICS_SHADER_DIR / "UnlitAnimated");
			}
		};

		struct PBRAnimated : public PBR
		{
			PBRAnimated(bool isInstanced = false) : PBR(isInstanced)
			{
				m_ShaderHandle = AssetManager::Load<Shader>(AssetManager::GRAPHICS_SHADER_DIR / "PBRAnimated");
				m_UnlitShaderHandle = AssetManager::Load<Shader>(AssetManager::GRAPHICS_SHADER_DIR / "UnlitAnimated");
			}
		};

		struct PBRTexturedAnimated : public PBRTextured
		{
			PBRTexturedAnimated(bool isInstanced = false) : PBRTextured(isInstanced)
			{
				m_ShaderHandle = AssetManager::Load<Shader>(AssetManager::GRAPHICS_SHADER_DIR / "PBRTexturedAnimated");
				m_UnlitShaderHandle = AssetManager::Load<Shader>(AssetManager::GRAPHICS_SHADER_DIR / "UnlitAnimated");
			}
		};

		struct PhongAnimated : public Phong
		{
			PhongAnimated(bool isInstanced = false) : Phong(isInstanced)
			{
				m_ShaderHandle = AssetManager::Load<Shader>(AssetManager::GRAPHICS_SHADER_DIR / "PhongAnimated");
				m_UnlitShaderHandle = AssetManager::Load<Shader>(AssetManager::GRAPHICS_SHADER_DIR / "UnlitAnimated");
			}
		};
	}
}