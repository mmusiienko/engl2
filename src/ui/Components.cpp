#include "ui/Components.h"
#include "ui/ImGuiEntry.h"


namespace EnGl
{
	bool UiComponents::Noise3DChannelView(Noise3DChannel& channel)
	{
		bool changed = false;

		auto sectionName = std::format("Channel {}", channel.Params.NChannel);
		ImGui::SeparatorText(sectionName.c_str());

		changed |= InputUInt("Grid cells", &channel.Params.NCells);
		changed |= InputUInt("Seed", &channel.Params.Seed);
		changed |= InputUInt("Octaves", &channel.Params.Octaves);

		return changed;
	}

	void UiComponents::Noise3DView(Noise3DWrapper& noise)
	{
		ImGui::Text("Noise 3D Parameters");

		auto tex = AssetManager::GetAsset(noise.Texture).Asset;
		
		if (!tex)
		{
			spdlog::error("Noise texture is not loaded.");
			return;
		}

		bool changed = false;

		if (InputUInt("Texture Size", &tex->Properties().w))
		{
			tex->Properties().h = tex->Properties().w;
			tex->Properties().d = tex->Properties().w;
			tex->Update();
		}

		for (u32 i = 0; i < Noise3DChannels; i++)
		{
			ImGui::PushID(i);

			if (Noise3DChannelView(noise.Channels[i]) && !changed)
			{
				noise.Channels[i].Strategy(noise.Texture, noise.Channels[i].Params);
			}

			ImGui::PopID();
		}

		if (changed)
			noise.Fill();
	}

	bool UiComponents::Noise2DChannelView(Noise2DChannel& channel)
	{
		bool changed = false;

		auto sectionName = std::format("Channel {}", channel.Params.NChannel);
		ImGui::SeparatorText(sectionName.c_str());

		changed |= InputUInt("Grid cells", &channel.Params.NCells);
		changed |= InputUInt("Seed", &channel.Params.Seed);
		changed |= InputUInt("Octaves", &channel.Params.Octaves);

		return changed;
	}

	void UiComponents::Noise2DView(Noise2DWrapper& noise)
	{
		ImGui::Text("Noise 2D Parameters");

		auto tex = AssetManager::GetAsset(noise.Texture).Asset;

		if (!tex)
		{
			spdlog::error("Noise texture is not loaded.");
			return;
		}

		bool changed = false;

		if (InputUInt("Texture Size", &tex->Properties().w))
		{
			tex->Properties().h = tex->Properties().w;
			tex->Update();
		}

		for (u32 i = 0; i < Noise3DChannels; i++)
		{
			ImGui::PushID(i);

			if (Noise2DChannelView(noise.Channels[i]) && !changed)
			{
				noise.Channels[i].Strategy(noise.Texture, noise.Channels[i].Params);
			}

			ImGui::PopID();
		}

		Texture2DView(noise.Texture);

		if (changed)
		{
			noise.Fill();
			tex->GenerateMips();
		}
	}

	void UiComponents::Texture2DView(AssetHandle<Texture2D> texA)
	{
		auto tex = AssetManager::GetAsset(texA).Asset;
		if (!tex)
		{
			ImGui::Text("Texture is not loaded.");
			return;
		}

		ImGui::Image((ImTextureID)(intptr_t)tex->Id(), ImVec2(static_cast<f32>(tex->Properties().w), static_cast<f32>(tex->Properties().h)));
	}

	void UiComponents::Texture3DView(AssetHandle<Texture3D> texA)
	{
		return;
	}

	bool UiComponents::InputUInt(const char* label, u32* v, int step, int step_fast)
	{
		int temp = static_cast<int>(*v);
		bool changed = ImGui::InputInt(label, &temp, step, step_fast);
		if (changed) *v = static_cast<uint32_t>(std::max(temp, 0));
		return changed;
	}
}