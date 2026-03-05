#include "Components.h"


namespace EnGl
{
	UiComponents::ParamsChanged UiComponents::VoronoiNoiseChannelView(VoronoiNoiseParams& params, FractalNoiseParams& fParams)
	{
		ParamsChanged changed{};

		auto sectionName = std::format("Channel {}", params.NChannel);
		ImGui::SeparatorText(sectionName.c_str());

		ImGui::SeparatorText("Voronoi Parameters");

		changed.BaseNoise |= InputUInt("Grid Cells", &params.NPoints);
		params.NPoints = std::clamp(params.NPoints, 2u, 128u);

		ImGui::SeparatorText("Fractal (FBM) Parameters");

		changed.Fbm |= InputUInt("Octaves", &fParams.NOctaves);
		fParams.NOctaves = std::clamp(fParams.NOctaves, 1u, 16u);

		changed.Fbm |= ImGui::InputFloat("Persistence", &fParams.Persistence, 0.0f, 0.0f, "%.3f");
		fParams.Persistence = std::clamp(fParams.Persistence, 0.0f, 1.0f);

		changed.Fbm |= ImGui::InputFloat("Lacunarity", &fParams.Lacunarity, 0.0f, 0.0f, "%.3f");
		fParams.Lacunarity = std::clamp(fParams.Lacunarity, 1.0f, 4.0f);

		return changed;
	}

	UiComponents::ParamsChanged UiComponents::PerlinNoiseChannelView(PerlinNoiseParams& params, FractalNoiseParams& fParams)
	{
		ParamsChanged changed{};

		auto sectionName = std::format("Channel {}", params.NChannel);
		ImGui::SeparatorText(sectionName.c_str());

		ImGui::SeparatorText("Perlin Texture Parameters");

		changed.BaseNoise |= InputUInt("Grid Cells", &params.NCells);
		params.NCells = std::clamp(params.NCells, 2u, 128u);

		ImGui::SeparatorText("FBM Parameters");

		changed.Fbm |= InputUInt("Octaves", &fParams.NOctaves);
		fParams.NOctaves = std::clamp(fParams.NOctaves, 1u, 16u);

		changed.Fbm |= ImGui::InputFloat("Persistence", &fParams.Persistence, 0.0f, 0.0f, "%.3f");
		fParams.Persistence = std::clamp(fParams.Persistence, 0.0f, 1.0f);

		changed.Fbm |= ImGui::InputFloat("Lacunarity", &fParams.Lacunarity, 0.0f, 0.0f, "%.3f");
		fParams.Lacunarity = std::clamp(fParams.Lacunarity, 1.0f, 4.0f);

		return changed;
	}
	
	void UiComponents::VoronoiNoiseView(VoronoiWrapper& wrapper)
	{
		ImGui::Text("Voronoi FBM Parameters");
		
		bool dimChanged = false;
		dimChanged |= InputUInt("Texture Size", &wrapper.Dim);

		ImGui::PushID(0);
		auto [BaseNoiseChangedR, FbmChangedR] = VoronoiNoiseChannelView(wrapper.ParamsR, wrapper.FParamsR);
		ImGui::PopID();
		ImGui::PushID(1);
		auto [BaseNoiseChangedG, FbmChangedG] = VoronoiNoiseChannelView(wrapper.ParamsG, wrapper.FParamsG);
		ImGui::PopID();
		ImGui::PushID(2);
		auto [BaseNoiseChangedB, FbmChangedB] = VoronoiNoiseChannelView(wrapper.ParamsB, wrapper.FParamsB);
		ImGui::PopID();
		ImGui::PushID(3);
		auto [BaseNoiseChangedA, FbmChangedA] = VoronoiNoiseChannelView(wrapper.ParamsA, wrapper.FParamsA);
		ImGui::PopID();
		auto [tex, gen] = AssetManager::GetAsset(wrapper.Voronoi);
		auto [texFbm, gen2] = AssetManager::GetAsset(wrapper.VoronoiFBM);

		if (!tex || !texFbm)
		{
			spdlog::error("Voronoi textures are not loaded.");
			return;
		}

		if (dimChanged)
		{
			tex->Properties().w = wrapper.Dim;
			tex->Properties().h = wrapper.Dim;
			tex->Properties().d = wrapper.Dim;
			tex->Update();

			texFbm->Properties().w = wrapper.Dim;
			texFbm->Properties().h = wrapper.Dim;
			texFbm->Properties().d = wrapper.Dim;
			texFbm->Update();

			VoronoiNoise3D::Fill(wrapper.Voronoi, wrapper.ParamsR);
			VoronoiNoise3D::Fill(wrapper.Voronoi, wrapper.ParamsG);
			VoronoiNoise3D::Fill(wrapper.Voronoi, wrapper.ParamsB);
			VoronoiNoise3D::Fill(wrapper.Voronoi, wrapper.ParamsA);
			FractalNoise3D::Fill(wrapper.VoronoiFBM, wrapper.Voronoi, wrapper.FParamsR);
			FractalNoise3D::Fill(wrapper.VoronoiFBM, wrapper.Voronoi, wrapper.FParamsG);
			FractalNoise3D::Fill(wrapper.VoronoiFBM, wrapper.Voronoi, wrapper.FParamsB);
			FractalNoise3D::Fill(wrapper.VoronoiFBM, wrapper.Voronoi, wrapper.FParamsA);
		}
		else
		{
			if (BaseNoiseChangedR)
				VoronoiNoise3D::Fill(wrapper.Voronoi, wrapper.ParamsR);
			if (FbmChangedR || BaseNoiseChangedR)
				FractalNoise3D::Fill(wrapper.VoronoiFBM, wrapper.Voronoi, wrapper.FParamsR);
			if (BaseNoiseChangedG)
				VoronoiNoise3D::Fill(wrapper.Voronoi, wrapper.ParamsG);
			if (FbmChangedG || BaseNoiseChangedG)
				FractalNoise3D::Fill(wrapper.VoronoiFBM, wrapper.Voronoi, wrapper.FParamsG);
			if (BaseNoiseChangedB)
				VoronoiNoise3D::Fill(wrapper.Voronoi, wrapper.ParamsB);
			if (FbmChangedB || BaseNoiseChangedB)
				FractalNoise3D::Fill(wrapper.VoronoiFBM, wrapper.Voronoi, wrapper.FParamsG);
			if (BaseNoiseChangedA)
				VoronoiNoise3D::Fill(wrapper.Voronoi, wrapper.ParamsA);
			if (FbmChangedA || BaseNoiseChangedA)
				FractalNoise3D::Fill(wrapper.VoronoiFBM, wrapper.Voronoi, wrapper.FParamsA);
		}

		ImGui::Separator();
	}

	void UiComponents::PerlinNoiseView(PerlinWrapper& wrapper)
	{
		ImGui::Text("Perlin FBM Parameters");

		bool dimChanged = false;
		dimChanged |= InputUInt("Texture Size", &wrapper.Dim);

		ImGui::PushID(0);
		auto [BaseNoiseChangedR, FbmChangedR] = PerlinNoiseChannelView(wrapper.ParamsR, wrapper.FParamsR);
		ImGui::PopID();
		ImGui::PushID(1);
		auto [BaseNoiseChangedG, FbmChangedG] = PerlinNoiseChannelView(wrapper.ParamsG, wrapper.FParamsG);
		ImGui::PopID();
		ImGui::PushID(2);
		auto [BaseNoiseChangedB, FbmChangedB] = PerlinNoiseChannelView(wrapper.ParamsB, wrapper.FParamsB);
		ImGui::PopID();
		ImGui::PushID(3);
		auto [BaseNoiseChangedA, FbmChangedA] = PerlinNoiseChannelView(wrapper.ParamsA, wrapper.FParamsA);
		ImGui::PopID();
		auto [tex, gen] = AssetManager::GetAsset(wrapper.Perlin);
		auto [texFbm, gen2] = AssetManager::GetAsset(wrapper.PerlinFBM);

		if (!tex || !texFbm)
		{
			spdlog::error("Perlin textures are not loaded.");
			return;
		}

		if (dimChanged)
		{
			tex->Properties().w = wrapper.Dim;
			tex->Properties().h = wrapper.Dim;
			tex->Properties().d = wrapper.Dim;
			tex->Update();

			texFbm->Properties().w = wrapper.Dim;
			texFbm->Properties().h = wrapper.Dim;
			texFbm->Properties().d = wrapper.Dim;
			texFbm->Update();

			PerlinNoise3D::Fill(wrapper.Perlin, wrapper.ParamsR);
			PerlinNoise3D::Fill(wrapper.Perlin, wrapper.ParamsG);
			PerlinNoise3D::Fill(wrapper.Perlin, wrapper.ParamsB);
			PerlinNoise3D::Fill(wrapper.Perlin, wrapper.ParamsA);
			FractalNoise3D::Fill(wrapper.PerlinFBM, wrapper.Perlin, wrapper.FParamsR);
			FractalNoise3D::Fill(wrapper.PerlinFBM, wrapper.Perlin, wrapper.FParamsG);
			FractalNoise3D::Fill(wrapper.PerlinFBM, wrapper.Perlin, wrapper.FParamsB);
			FractalNoise3D::Fill(wrapper.PerlinFBM, wrapper.Perlin, wrapper.FParamsA);
		}
		else
		{
			if (BaseNoiseChangedR)
				PerlinNoise3D::Fill(wrapper.Perlin, wrapper.ParamsR);
			if (FbmChangedR || BaseNoiseChangedR)
				FractalNoise3D::Fill(wrapper.PerlinFBM, wrapper.Perlin, wrapper.FParamsR);
			if (BaseNoiseChangedG)
				PerlinNoise3D::Fill(wrapper.Perlin, wrapper.ParamsG);
			if (FbmChangedG || BaseNoiseChangedG)
				FractalNoise3D::Fill(wrapper.PerlinFBM, wrapper.Perlin, wrapper.FParamsG);
			if (BaseNoiseChangedB)
				PerlinNoise3D::Fill(wrapper.Perlin, wrapper.ParamsB);
			if (FbmChangedB || BaseNoiseChangedB)
				FractalNoise3D::Fill(wrapper.PerlinFBM, wrapper.Perlin, wrapper.FParamsG);
			if (BaseNoiseChangedA)
				PerlinNoise3D::Fill(wrapper.Perlin, wrapper.ParamsA);
			if (FbmChangedA || BaseNoiseChangedA)
				FractalNoise3D::Fill(wrapper.PerlinFBM, wrapper.Perlin, wrapper.FParamsA);
		}

		ImGui::Separator();
	}

	bool UiComponents::InputUInt(const char* label, uint32_t* v, int step, int step_fast)
	{
		int temp = static_cast<int>(*v);
		bool changed = ImGui::InputInt(label, &temp, step, step_fast);
		if (changed) *v = static_cast<uint32_t>(std::max(temp, 0));
		return changed;
	}
}