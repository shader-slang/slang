// d3d-util.h
#pragma once

#include <stdint.h>

#include "slang-com-helper.h"

#include "slang-com-ptr.h"
#include "core/slang-list.h"

#include "../flag-combiner.h"

#include "../render.h"

#include <D3Dcommon.h>
#include <DXGIFormat.h>
#include <dxgi.h>

namespace gfx {

class D3DUtil
{
    public:
    enum UsageType
    {
        USAGE_UNKNOWN,                      ///< Generally used to mark an error
        USAGE_TARGET,                       ///< Format should be used when written as target
        USAGE_DEPTH_STENCIL,                ///< Format should be used when written as depth stencil
        USAGE_SRV,                          ///< Format if being read as srv
        USAGE_COUNT_OF,
    };
    enum UsageFlag
    {
        USAGE_FLAG_MULTI_SAMPLE = 0x1,      ///< If set will be used form multi sampling (such as MSAA)
        USAGE_FLAG_SRV = 0x2,               ///< If set means will be used as a shader resource view (SRV)
    };

        /// Get primitive topology as D3D primitive topology
    static D3D_PRIMITIVE_TOPOLOGY getPrimitiveTopology(PrimitiveTopology prim);

        /// Calculate size taking into account alignment. Alignment must be a power of 2
    static UInt calcAligned(UInt size, UInt alignment) { return (size + alignment - 1) & ~(alignment - 1); }

        /// Compile HLSL code to DXBC
    static Slang::Result compileHLSLShader(char const* sourcePath, char const* source, char const* entryPointName, char const* dxProfileName, Slang::ComPtr<ID3DBlob>& shaderBlobOut);

        /// Given a slang pixel format returns the equivalent DXGI_ pixel format. If the format is not known, will return DXGI_FORMAT_UNKNOWN
    static DXGI_FORMAT getMapFormat(Format format);

        /// Given the usage, flags, and format will return the most suitable format. Will return DXGI_UNKNOWN if combination is not possible
    static DXGI_FORMAT calcFormat(UsageType usage, DXGI_FORMAT format);
        /// Calculate appropriate format for creating a buffer for usage and flags
    static DXGI_FORMAT calcResourceFormat(UsageType usage, Int usageFlags, DXGI_FORMAT format);
        /// True if the type is 'typeless'
    static bool isTypeless(DXGI_FORMAT format);

        /// Returns number of bits used for color channel for format (for channels with multiple sizes, returns smallest ie RGB565 -> 5)
    static Int getNumColorChannelBits(DXGI_FORMAT fmt);

        /// Append text in in, into wide char array
    static void appendWideChars(const char* in, Slang::List<wchar_t>& out);

    
    static SlangResult createFactory(DeviceCheckFlags flags, Slang::ComPtr<IDXGIFactory>& outFactory);

        /// Get the dxgiModule
    static HMODULE getDxgiModule();

        /// Find adapters
    static SlangResult findAdapters(DeviceCheckFlags flags, const Slang::UnownedStringSlice& adapaterName, IDXGIFactory* dxgiFactory, Slang::List<Slang::ComPtr<IDXGIAdapter>>& dxgiAdapters);
        /// Find adapters
    static SlangResult findAdapters(DeviceCheckFlags flags, const Slang::UnownedStringSlice& adapaterName, Slang::List<Slang::ComPtr<IDXGIAdapter>>& dxgiAdapters);

        /// True if the adapter is warp
    static bool isWarp(IDXGIFactory* dxgiFactory, IDXGIAdapter* adapter);

};

} // renderer_test
