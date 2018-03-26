// d3d-util.cpp
#include "d3d-util.h"

#include <d3dcompiler.h>

// We will use the C standard library just for printing error messages.
#include <stdio.h>

namespace renderer_test {
using namespace Slang;

/* static */DXGI_FORMAT D3DUtil::getMapFormat(Format format)
{
    switch (format)
    {
        case Format::RGB_Float32:
            return DXGI_FORMAT_R32G32B32_FLOAT;
        case Format::RG_Float32:
            return DXGI_FORMAT_R32G32_FLOAT;
        default:
            return DXGI_FORMAT_UNKNOWN;
    }
}

/* static */Result D3DUtil::compileHLSLShader(char const* sourcePath, char const* source, char const* entryPointName, char const* dxProfileName, ComPtr<ID3DBlob>& shaderBlobOut)
{
    // Rather than statically link against the `d3dcompile` library, we
    // dynamically load it.
    //
    // Note: A more realistic application would compile from HLSL text to D3D
    // shader bytecode as part of an offline process, rather than doing it
    // on-the-fly like this
    //
    static pD3DCompile compileFunc = nullptr;
    if (!compileFunc)
    {
        // TODO(tfoley): maybe want to search for one of a few versions of the DLL
        HMODULE compilerModule = LoadLibraryA("d3dcompiler_47.dll");
        if (!compilerModule)
        {
            fprintf(stderr, "error: failed load 'd3dcompiler_47.dll'\n");
            return SLANG_FAIL;
        }

        compileFunc = (pD3DCompile)GetProcAddress(compilerModule, "D3DCompile");
        if (!compileFunc)
        {
            fprintf(stderr, "error: failed load symbol 'D3DCompile'\n");
            return SLANG_FAIL;
        }
    }

    // For this example, we turn on debug output, and turn off all
    // optimization. A real application would only use these flags
    // when shader debugging is needed.
    UINT flags = 0;
    flags |= D3DCOMPILE_DEBUG;
    flags |= D3DCOMPILE_OPTIMIZATION_LEVEL0 | D3DCOMPILE_SKIP_OPTIMIZATION;

    // We will always define `__HLSL__` when compiling here, so that
    // input code can react differently to being compiled as pure HLSL.
    D3D_SHADER_MACRO defines[] = {
        { "__HLSL__", "1" },
        { nullptr, nullptr },
    };

    // The `D3DCompile` entry point takes a bunch of parameters, but we
    // don't really need most of them for Slang-generated code.
    ComPtr<ID3DBlob> shaderBlob;
    ComPtr<ID3DBlob> errorBlob;

    HRESULT hr = compileFunc(source, strlen(source), sourcePath, &defines[0], nullptr, entryPointName, dxProfileName, flags, 0,
        shaderBlob.writeRef(), errorBlob.writeRef());

    // If the HLSL-to-bytecode compilation produced any diagnostic messages
    // then we will print them out (whether or not the compilation failed).
    if (errorBlob)
    {
        ::fputs((const char*)errorBlob->GetBufferPointer(), stderr);
        ::fflush(stderr);
        ::OutputDebugStringA((const char*)errorBlob->GetBufferPointer());
        return SLANG_FAIL;
    }

    SLANG_RETURN_ON_FAIL(hr);
    shaderBlobOut.swap(shaderBlob);
    return SLANG_OK;
}

} // renderer_test
