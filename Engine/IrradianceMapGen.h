#pragma once

#include <d3d11.h>
#include "SimpleMath.h"
#include <vector>
#include "EngineUtil.h"

namespace IrradianceMapGen {
	void GenerateIrradianceSH(const std::vector<uint8_t*>& cubeFacePtrs, const uint32_t& faceDimension, const uint32_t& numChannels);
}

