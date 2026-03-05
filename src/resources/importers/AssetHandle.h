#pragma once
#include "../../core/Core.h"


namespace EnGl
{
	using AssetId = u32;

	template<typename AssetT>
	struct AssetHandle
	{
		AssetId Id = 0;
		u32 Generation = 0;

		bool operator==(const AssetHandle& other) const noexcept {
			return Id == other.Id;
		}
	};
}

namespace std
{
	template<typename AssetT>
	struct hash<EnGl::AssetHandle<AssetT>> 
	{
		std::size_t operator()(const EnGl::AssetHandle<AssetT>& handle) const noexcept 
		{
			return std::hash<uint32_t>{}(handle.Id);
		}
	};
}