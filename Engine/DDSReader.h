#pragma once

#include <d3d11_1.h>
#include "EngineUtil.h"
#include <vector>

namespace DDSReader {
	HRESULT LoadDDSCubeMap(const wchar_t* fileName, std::vector<uint8_t*>& cubeFacePtrs);
}

