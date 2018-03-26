// d3d-util.h
#pragma once

#include <stdint.h>
#include "../../source/core/slang-result.h"
#include "../../source/core/slang-com-ptr.h"

#include "render.h"

#include <D3Dcommon.h>
#include <DXGIFormat.h>

namespace renderer_test {

class D3DUtil
{
    public:

        /// Calculate size taking into account alignment. Alignment must be a power of 2
    static UInt calcAligned(UInt size, UInt alignment) { return (size + alignment - 1) & ~(alignment - 1); }

        /// The Slang compiler currently generates HLSL source, so we'll need a utility
        /// routine (defined later) to translate that into D3D11 shader bytecode.
        /// Definition of the HLSL-to-bytecode compilation logic.
    static Slang::Result compileHLSLShader(char const* sourcePath, char const* source, char const* entryPointName, char const* dxProfileName, Slang::ComPtr<ID3DBlob>& shaderBlobOut);

    static DXGI_FORMAT getMapFormat(Format format);

};

} // renderer_test
