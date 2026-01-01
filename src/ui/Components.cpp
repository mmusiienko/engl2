#include "Components.h"


namespace EnGl
{
	void UiComponents::ColorView(Color& color)
	{
		ImGui::Text("Color");

		ImGui::ColorEdit3("Ambient", glm::value_ptr(color.Ambient));
		ImGui::ColorEdit3("Diffuse", glm::value_ptr(color.Diffuse));
		ImGui::ColorEdit3("Specular", glm::value_ptr(color.Specular));
	}

	UiComponents::ParamsChanged UiComponents::VoronoiNoiseChannelView(VoronoiNoise3DParamsShort& params, FractalNoiseParams& fParams, u32 id)
	{
		ParamsChanged changed{};
		ImGui::PushID(id);

		auto sectionName = std::format("Channel {}", params.NChannel);
		ImGui::SeparatorText(sectionName.c_str());

		ImGui::SeparatorText("Voronoi Parameters");

		changed.BaseNoise |= InputUInt("Grid Cells", &params.NPoints);
		params.NPoints = std::clamp(params.NPoints, 2u, 128u);

		ImGui::SeparatorText("Fractal (FBM) Parameters");

		changed.Fbm |= InputUInt("Octaves", &fParams.NOctaves);
		fParams.NOctaves = std::clamp(fParams.NOctaves, 1u, 16u);

		changed.Fbm |= ImGui::InputFloat("Persistence", &fParams.persistence, 0.0f, 0.0f, "%.3f");
		fParams.persistence = std::clamp(fParams.persistence, 0.0f, 1.0f);

		changed.Fbm |= ImGui::InputFloat("Lacunarity", &fParams.lacunarity, 0.0f, 0.0f, "%.3f");
		fParams.lacunarity = std::clamp(fParams.lacunarity, 1.0f, 4.0f);
		ImGui::PopID();

		return changed;
	}

	UiComponents::ParamsChanged UiComponents::PerlinNoiseChannelView(PerlinNoise3DParamsShort& params, FractalNoiseParams& fParams, u32 id)
	{
		ParamsChanged changed{};

		ImGui::PushID(id);
		auto sectionName = std::format("Channel {}", params.NChannel);
		ImGui::SeparatorText(sectionName.c_str());

		ImGui::SeparatorText("Perlin Texture Parameters");

		changed.BaseNoise |= InputUInt("Grid Cells", &params.NCells);
		params.NCells = std::clamp(params.NCells, 2u, 128u);

		ImGui::SeparatorText("FBM Parameters");

		changed.Fbm |= InputUInt("Octaves", &fParams.NOctaves);
		fParams.NOctaves = std::clamp(fParams.NOctaves, 1u, 16u);

		changed.Fbm |= ImGui::InputFloat("Persistence", &fParams.persistence, 0.0f, 0.0f, "%.3f");
		fParams.persistence = std::clamp(fParams.persistence, 0.0f, 1.0f);

		changed.Fbm |= ImGui::InputFloat("Lacunarity", &fParams.lacunarity, 0.0f, 0.0f, "%.3f");
		fParams.lacunarity = std::clamp(fParams.lacunarity, 1.0f, 4.0f);
		ImGui::PopID();

		return changed;
	}
	
	void UiComponents::VoronoiNoiseView(VoronoiWrapper& wrapper)
	{
		ImGui::Text("Voronoi FBM Parameters");
		
		bool dimChanged = false;
		dimChanged |= InputUInt("Texture Size", &wrapper.Dim);

		auto [BaseNoiseChangedR, FbmChangedR] = VoronoiNoiseChannelView(wrapper.ParamsR, wrapper.FParamsR, 1);
		auto [BaseNoiseChangedG, FbmChangedG] = VoronoiNoiseChannelView(wrapper.ParamsG, wrapper.FParamsG, 2);
		auto [BaseNoiseChangedB, FbmChangedB] = VoronoiNoiseChannelView(wrapper.ParamsB, wrapper.FParamsB, 3);
		auto [BaseNoiseChangedA, FbmChangedA] = VoronoiNoiseChannelView(wrapper.ParamsA, wrapper.FParamsA, 4);

		if (dimChanged)
		{
			wrapper.ParamsR.dim = wrapper.Dim;
			wrapper.ParamsG.dim = wrapper.Dim;
			wrapper.ParamsB.dim = wrapper.Dim;
			wrapper.ParamsA.dim = wrapper.Dim;

			wrapper.Voronoi = make_ref<VoronoiNoise3D>(wrapper.ParamsR);
			wrapper.Voronoi->Fill(wrapper.ParamsG);
			wrapper.Voronoi->Fill(wrapper.ParamsB);
			wrapper.Voronoi->Fill(wrapper.ParamsA);
			wrapper.VoronoiFBM = make_ref<FractalNoise3D>(wrapper.FParamsR, *wrapper.Voronoi);
			wrapper.VoronoiFBM->Fill(wrapper.FParamsG, *wrapper.Voronoi);
			wrapper.VoronoiFBM->Fill(wrapper.FParamsB, *wrapper.Voronoi);
			wrapper.VoronoiFBM->Fill(wrapper.FParamsA, *wrapper.Voronoi);
		}
		else
		{
			if (BaseNoiseChangedR)
				wrapper.Voronoi->Fill(wrapper.ParamsR);
			if (FbmChangedR || BaseNoiseChangedR)
				wrapper.VoronoiFBM->Fill(wrapper.FParamsR, *wrapper.Voronoi);
			if (BaseNoiseChangedG)
				wrapper.Voronoi->Fill(wrapper.ParamsG);
			if (FbmChangedG || BaseNoiseChangedG)
				wrapper.VoronoiFBM->Fill(wrapper.FParamsG, *wrapper.Voronoi);
			if (BaseNoiseChangedB)
				wrapper.Voronoi->Fill(wrapper.ParamsB);
			if (FbmChangedB || BaseNoiseChangedB)
				wrapper.VoronoiFBM->Fill(wrapper.FParamsB, *wrapper.Voronoi);
			if (BaseNoiseChangedA)
				wrapper.Voronoi->Fill(wrapper.ParamsA);
			if (FbmChangedA || BaseNoiseChangedA)
				wrapper.VoronoiFBM->Fill(wrapper.FParamsA, *wrapper.Voronoi);
		}

		ImGui::Separator();
	}

	void UiComponents::PerlinNoiseView(PerlinWrapper& wrapper)
	{
		ImGui::Text("Perlin FBM Parameters");

		bool dimChanged = false;
		dimChanged |= InputUInt("Texture Size", &wrapper.Dim);

		auto [BaseNoiseChangedR, FbmChangedR] = PerlinNoiseChannelView(wrapper.ParamsR, wrapper.FParamsR, 1);
		auto [BaseNoiseChangedG, FbmChangedG] = PerlinNoiseChannelView(wrapper.ParamsG, wrapper.FParamsG, 2);
		auto [BaseNoiseChangedB, FbmChangedB] = PerlinNoiseChannelView(wrapper.ParamsB, wrapper.FParamsB, 3);
		auto [BaseNoiseChangedA, FbmChangedA] = PerlinNoiseChannelView(wrapper.ParamsA, wrapper.FParamsA, 4);

		if (dimChanged)
		{
			wrapper.ParamsR.dim = wrapper.Dim;
			wrapper.ParamsG.dim = wrapper.Dim;
			wrapper.ParamsB.dim = wrapper.Dim;
			wrapper.ParamsA.dim = wrapper.Dim;

			wrapper.Perlin = make_ref<PerlinNoise3D>(wrapper.ParamsR);
			wrapper.Perlin->Fill(wrapper.ParamsG);
			wrapper.Perlin->Fill(wrapper.ParamsB);
			wrapper.Perlin->Fill(wrapper.ParamsA);
			wrapper.PerlinFBM = make_ref<FractalNoise3D>(wrapper.FParamsR, *wrapper.Perlin);
			wrapper.PerlinFBM->Fill(wrapper.FParamsG, *wrapper.Perlin);
			wrapper.PerlinFBM->Fill(wrapper.FParamsB, *wrapper.Perlin);
			wrapper.PerlinFBM->Fill(wrapper.FParamsA, *wrapper.Perlin);
		}
		else
		{
			if (BaseNoiseChangedR)
				wrapper.Perlin->Fill(wrapper.ParamsR);
			if (FbmChangedR || BaseNoiseChangedR)
				wrapper.PerlinFBM->Fill(wrapper.FParamsR, *wrapper.Perlin);

			if (BaseNoiseChangedG)
				wrapper.Perlin->Fill(wrapper.ParamsG);
			if (FbmChangedG || BaseNoiseChangedG)
				wrapper.PerlinFBM->Fill(wrapper.FParamsG, *wrapper.Perlin);

			if (BaseNoiseChangedB)
				wrapper.Perlin->Fill(wrapper.ParamsB);
			if (FbmChangedB || BaseNoiseChangedB)
				wrapper.PerlinFBM->Fill(wrapper.FParamsB, *wrapper.Perlin);

			if (BaseNoiseChangedA)
				wrapper.Perlin->Fill(wrapper.ParamsA);
			if (FbmChangedA || BaseNoiseChangedA)
				wrapper.PerlinFBM->Fill(wrapper.FParamsA, *wrapper.Perlin);
		}

		ImGui::Separator();
	}


	void UiComponents::AttenuationView(Attenuation& attenuation)
	{
		ImGui::Text("Attenuation");

		ImGui::InputFloat("Intensity", &attenuation.Intensity);
	}

	void UiComponents::Vec3View(const std::string& title, glm::vec3& vec)
	{
		ImGui::InputFloat3(title.c_str(), glm::value_ptr(vec));
	}

	bool UiComponents::InputUInt(const char* label, uint32_t* v, int step, int step_fast)
	{
		int temp = static_cast<int>(*v);
		bool changed = ImGui::InputInt(label, &temp, step, step_fast);
		if (changed) *v = static_cast<uint32_t>(std::max(temp, 0));
		return changed;
	}
}