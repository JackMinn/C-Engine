#include "stdafx.h"
#include "DDSReader.h"

#include <assert.h>
#include <algorithm>
#include <memory>

//--------------------------------------------------------------------------------------
// Macros
//--------------------------------------------------------------------------------------
#ifndef MAKEFOURCC
#define MAKEFOURCC(ch0, ch1, ch2, ch3)                              \
                ((uint32_t)(uint8_t)(ch0) | ((uint32_t)(uint8_t)(ch1) << 8) |       \
                ((uint32_t)(uint8_t)(ch2) << 16) | ((uint32_t)(uint8_t)(ch3) << 24 ))
#endif /* defined(MAKEFOURCC) */

//--------------------------------------------------------------------------------------
// DDS file structure definitions
//
// See DDS.h in the 'Texconv' sample and the 'DirectXTex' library
//--------------------------------------------------------------------------------------

const uint32_t DDS_MAGIC = 0x20534444; // "DDS "

struct DDS_PIXELFORMAT
{
	uint32_t    size;
	uint32_t    flags;
	uint32_t    fourCC;
	uint32_t    RGBBitCount;
	uint32_t    RBitMask;
	uint32_t    GBitMask;
	uint32_t    BBitMask;
	uint32_t    ABitMask;
};

#define DDS_FOURCC      0x00000004  // DDPF_FOURCC
#define DDS_RGB         0x00000040  // DDPF_RGB
#define DDS_LUMINANCE   0x00020000  // DDPF_LUMINANCE
#define DDS_ALPHA       0x00000002  // DDPF_ALPHA
#define DDS_BUMPDUDV    0x00080000  // DDPF_BUMPDUDV

#define DDS_HEADER_FLAGS_VOLUME         0x00800000  // DDSD_DEPTH

#define DDS_HEIGHT 0x00000002 // DDSD_HEIGHT
#define DDS_WIDTH  0x00000004 // DDSD_WIDTH

#define DDS_CUBEMAP_POSITIVEX 0x00000600 // DDSCAPS2_CUBEMAP | DDSCAPS2_CUBEMAP_POSITIVEX
#define DDS_CUBEMAP_NEGATIVEX 0x00000a00 // DDSCAPS2_CUBEMAP | DDSCAPS2_CUBEMAP_NEGATIVEX
#define DDS_CUBEMAP_POSITIVEY 0x00001200 // DDSCAPS2_CUBEMAP | DDSCAPS2_CUBEMAP_POSITIVEY
#define DDS_CUBEMAP_NEGATIVEY 0x00002200 // DDSCAPS2_CUBEMAP | DDSCAPS2_CUBEMAP_NEGATIVEY
#define DDS_CUBEMAP_POSITIVEZ 0x00004200 // DDSCAPS2_CUBEMAP | DDSCAPS2_CUBEMAP_POSITIVEZ
#define DDS_CUBEMAP_NEGATIVEZ 0x00008200 // DDSCAPS2_CUBEMAP | DDSCAPS2_CUBEMAP_NEGATIVEZ

#define DDS_CUBEMAP_ALLFACES ( DDS_CUBEMAP_POSITIVEX | DDS_CUBEMAP_NEGATIVEX |\
                               DDS_CUBEMAP_POSITIVEY | DDS_CUBEMAP_NEGATIVEY |\
                               DDS_CUBEMAP_POSITIVEZ | DDS_CUBEMAP_NEGATIVEZ )

#define DDS_CUBEMAP 0x00000200 // DDSCAPS2_CUBEMAP

enum DDS_MISC_FLAGS2
{
	DDS_MISC_FLAGS2_ALPHA_MODE_MASK = 0x7L,
};

struct DDS_HEADER
{
	uint32_t        size;
	uint32_t        flags;
	uint32_t        height;
	uint32_t        width;
	uint32_t        pitchOrLinearSize;
	uint32_t        depth; // only if DDS_HEADER_FLAGS_VOLUME is set in flags
	uint32_t        mipMapCount;
	uint32_t        reserved1[11];
	DDS_PIXELFORMAT ddspf;
	uint32_t        caps;
	uint32_t        caps2;
	uint32_t        caps3;
	uint32_t        caps4;
	uint32_t        reserved2;
};

struct DDS_HEADER_DXT10
{
	DXGI_FORMAT     dxgiFormat;
	uint32_t        resourceDimension;
	uint32_t        miscFlag; // see D3D11_RESOURCE_MISC_FLAG
	uint32_t        arraySize;
	uint32_t        miscFlags2;
};

namespace
{
	struct handle_closer { void operator()(HANDLE h) { if (h) CloseHandle(h); } };
	typedef std::unique_ptr<void, handle_closer> ScopedHandle;
	inline HANDLE safe_handle(HANDLE h) { return (h == INVALID_HANDLE_VALUE) ? nullptr : h; }
}


HRESULT DDSReader::LoadDDSCubeMap(_In_z_ const wchar_t* fileName, std::vector<uint8_t*>& cubeFacePtrs)
{
	std::unique_ptr<uint8_t[]> ddsData = nullptr;

	// open the file
#if (_WIN32_WINNT >= _WIN32_WINNT_WIN8)
	ScopedHandle hFile(safe_handle(CreateFile2(fileName,
		GENERIC_READ,
		FILE_SHARE_READ,
		OPEN_EXISTING,
		nullptr)));
#else
	ScopedHandle hFile(safe_handle(CreateFileW(fileName,
		GENERIC_READ,
		FILE_SHARE_READ,
		nullptr,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		nullptr)));
#endif

	if (!hFile)
	{
		return HRESULT_FROM_WIN32(GetLastError());
	}

	// Get the file size
	FILE_STANDARD_INFO fileInfo;
	if (!GetFileInformationByHandleEx(hFile.get(), FileStandardInfo, &fileInfo, sizeof(fileInfo)))
	{
		return HRESULT_FROM_WIN32(GetLastError());
	}

	// File is too big for 32-bit allocation, so reject read
	if (fileInfo.EndOfFile.HighPart > 0)
	{
		return E_FAIL;
	}

	// Need at least enough data to fill the header and magic number to be a valid DDS
	if (fileInfo.EndOfFile.LowPart < (sizeof(DDS_HEADER) + sizeof(uint32_t)))
	{
		return E_FAIL;
	}

	// create enough space for the file data
	ddsData.reset(new (std::nothrow) uint8_t[fileInfo.EndOfFile.LowPart]);
	if (!ddsData)
	{
		return E_OUTOFMEMORY;
	}

	// read the data in
	DWORD BytesRead = 0;
	if (!ReadFile(hFile.get(),
		ddsData.get(),
		fileInfo.EndOfFile.LowPart,
		&BytesRead,
		nullptr
	))
	{
		return HRESULT_FROM_WIN32(GetLastError());
	}

	if (BytesRead < fileInfo.EndOfFile.LowPart)
	{
		return E_FAIL;
	}

	// DDS files always start with the same magic number ("DDS ")
	uint32_t dwMagicNumber = *reinterpret_cast<const uint32_t*>(ddsData.get());
	if (dwMagicNumber != DDS_MAGIC)
	{
		return E_FAIL;
	}

	auto hdr = reinterpret_cast<const DDS_HEADER*>(ddsData.get() + sizeof(uint32_t));

	// Verify header to validate DDS file
	if (hdr->size != sizeof(DDS_HEADER) ||
		hdr->ddspf.size != sizeof(DDS_PIXELFORMAT))
	{
		return E_FAIL;
	}

	// Check for DX10 extension
	bool bDXT10Header = false;
	if ((hdr->ddspf.flags & DDS_FOURCC) &&
		(MAKEFOURCC('D', 'X', '1', '0') == hdr->ddspf.fourCC))
	{
		// Must be long enough for both headers and magic value
		if (fileInfo.EndOfFile.LowPart < (sizeof(DDS_HEADER) + sizeof(uint32_t) + sizeof(DDS_HEADER_DXT10)))
		{
			return E_FAIL;
		}

		bDXT10Header = true;
	}

	int offset = sizeof(uint32_t) + sizeof(DDS_HEADER)
		+ (bDXT10Header ? sizeof(DDS_HEADER_DXT10) : 0);

	//Custom debug code
	DebugLog("si", "Number of bytes is: ", fileInfo.EndOfFile.LowPart);
	DebugLog("si", "Number of bytes in offset: ", offset);

	if (hdr->caps2 & DDS_CUBEMAP)
	{
		// We require all six faces to be defined
		if ((hdr->caps2 & DDS_CUBEMAP_ALLFACES) == DDS_CUBEMAP_ALLFACES) {
			DebugLog("s", "This is a cubemap");
			assert(true);
		}
		else {
			assert(false);
		}
	}
	DebugLog("s", bDXT10Header ? "DX10 Header" : "No DX10 Header");

	unsigned int bytesPerFace = hdr->width * hdr->height * 4;
	unsigned int temp = bytesPerFace;
	unsigned int bytesForMips = 0;
	for (unsigned int i = 1; i < hdr->mipMapCount; i++) {
		temp /= 4;
		bytesForMips += temp;
	}

	uint8_t* ddsColorData = ddsData.get() + offset;

	uint8_t* posX = new uint8_t[bytesPerFace];
	uint8_t* negX = new uint8_t[bytesPerFace];
	uint8_t* posY = new uint8_t[bytesPerFace];
	uint8_t* negY = new uint8_t[bytesPerFace];
	uint8_t* posZ = new uint8_t[bytesPerFace];
	uint8_t* negZ = new uint8_t[bytesPerFace];

	cubeFacePtrs.emplace_back(posX);
	cubeFacePtrs.emplace_back(negX);
	cubeFacePtrs.emplace_back(posY);
	cubeFacePtrs.emplace_back(negY);
	cubeFacePtrs.emplace_back(posZ);
	cubeFacePtrs.emplace_back(negZ);
	
	//format is DXGI_FORMAT_B8G8R8X8_UNORM
	unsigned int ddsDataIndex = 0;
	memcpy(posX, ddsColorData + ddsDataIndex, bytesPerFace);
	ddsDataIndex += bytesPerFace + bytesForMips;
	memcpy(negX, ddsColorData + ddsDataIndex, bytesPerFace);
	ddsDataIndex += bytesPerFace + bytesForMips;
	memcpy(posY, ddsColorData + ddsDataIndex, bytesPerFace);
	ddsDataIndex += bytesPerFace + bytesForMips;
	memcpy(negY, ddsColorData + ddsDataIndex, bytesPerFace);
	ddsDataIndex += bytesPerFace + bytesForMips;
	memcpy(posZ, ddsColorData + ddsDataIndex, bytesPerFace);
	ddsDataIndex += bytesPerFace + bytesForMips;
	memcpy(negZ, ddsColorData + ddsDataIndex, bytesPerFace);
	ddsDataIndex += bytesPerFace + bytesForMips;

	DebugLog("si", "Total copied bytes is: ", ddsDataIndex);

	
	//image starts top left and goes left to right, then to the next row etc
	//DebugLog("si", "Random color B is: ", *(posX + (1023*4) + 0));
	//DebugLog("si", "Random color G is: ", *(posX + (1023*4) + 1));
	//DebugLog("si", "Random color R is: ", *(posX + (1023*4) + 2));
	//DebugLog("si", "Random color A is: ", *(posX + (1023*4) + 3));

	//https://github.com/belcour/IntegralSH/blob/master/include/SphericalHarmonics.hpp
	//we have the data for the cubemap, we should be able to produce a function which takes 6 pointers, a direction and returns a color value
	//we then need to sample a hemisphere of directions from a given point on a unit sphere in order to make an irradiance map from the cube map
	//this irradiance map functions as an arbitrary function which we can then project onto the SH basis and get our SH coefficients

	//delete[] posX;
	//delete[] negX;
	//delete[] posY;
	//delete[] negY;
	//delete[] posZ;
	//delete[] negZ;

	assert(fileInfo.EndOfFile.LowPart == offset + ddsDataIndex);
	return S_OK;
}

