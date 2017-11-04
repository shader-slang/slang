// dxc-support.cpp
#include "compiler.h"

// This file implements support for invoking the `dxcompiler`
// library to translate HLSL to DXIL.

#if defined(_WIN32)
#  if !defined(SLANG_ENABLE_DXIL_SUPPORT)
#    define SLANG_ENABLE_DXIL_SUPPORT 1
#  endif
#endif

#if !defined(SLANG_ENABLE_DXIL_SUPPORT)
#  define SLANG_ENABLE_DXIL_SUPPORT 0
#endif

#if SLANG_ENABLE_DXIL_SUPPORT

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <Unknwn.h>
#include "../../external/dxc/dxcapi.h"
#undef WIN32_LEAN_AND_MEAN
#undef NOMINMAX

#include "../core/platform.h"

namespace Slang
{
    char const* GetHLSLProfileName(Profile profile);
    String emitHLSLForEntryPoint(
        EntryPointRequest*  entryPoint,
        TargetRequest*      targetReq);

    SharedLibrary loadDXCSharedLibrary(CompileRequest* request)
    {
        // TODO(tfoley): Let user specify name/path of library to use.
        char const* libraryName = "dxcompiler";

        SharedLibrary library = SharedLibrary::load(libraryName);
        if (!library)
        {
            request->mSink.diagnose(SourceLoc(), Diagnostics::failedToLoadDynamicLibrary, libraryName);
        }
        return library;
    }

    SharedLibrary getDXCSharedLibrary(CompileRequest* request)
    {
        static SharedLibrary library =  loadDXCSharedLibrary(request);
        return library;
    }

    int emitDXILForEntryPointUsingDXC(
        EntryPointRequest*  entryPoint,
        TargetRequest*      targetReq,
        List<uint8_t>&      outCode)
    {
        auto compileRequest = entryPoint->compileRequest;

        // First deal with all the rigamarole of loading
        // the `dxcompiler` library, and creating the
        // top-level COM objects that will be used to
        // compile things.

        static DxcCreateInstanceProc dxcCreateInstance = nullptr;
        if (!dxcCreateInstance)
        {
            auto dxcSharedLibrary = getDXCSharedLibrary(compileRequest);
            if (!dxcSharedLibrary)
                return 1;

            dxcCreateInstance = (DxcCreateInstanceProc) dxcSharedLibrary.findFuncByName("DxcCreateInstance");
            if (!dxcCreateInstance)
                return 1;
        }

        IDxcCompiler* dxcCompiler = nullptr;
        if (FAILED(dxcCreateInstance(
            CLSID_DxcCompiler,
            __uuidof(dxcCompiler),
            (LPVOID*) &dxcCompiler)))
        {
            return 1;
        }

        IDxcLibrary* dxcLibrary = nullptr;
        if (FAILED(dxcCreateInstance(
            CLSID_DxcLibrary,
            __uuidof(dxcLibrary),
            (LPVOID*) &dxcLibrary)))
        {
            return 1;
        }

        // Now let's go ahead and generate HLSL for the entry
        // point, since we'll need that to feed into dxc.
        auto hlslCode = emitHLSLForEntryPoint(entryPoint, targetReq);
        maybeDumpIntermediate(entryPoint->compileRequest, hlslCode.Buffer(), CodeGenTarget::HLSL);

        // Wrap the 

        // Create blob from the string
        IDxcBlobEncoding* dxcSourceBlob = nullptr;
        if (FAILED(dxcLibrary->CreateBlobWithEncodingFromPinned(
            (LPBYTE)hlslCode.Buffer(),
            (UINT32)hlslCode.Length(),
            0,
            &dxcSourceBlob)))
        {
            return 1;
        }

        WCHAR const* args[16];
        UINT32 argCount = 0;

        // TODO: deal with
        bool treatWarningsAsErrors = false;
        if (treatWarningsAsErrors)
        {
            args[argCount++] = L"-WX";
        }

        String entryPointName = getText(entryPoint->name);
        OSString wideEntryPointName = entryPointName.ToWString();

        String profileName = GetHLSLProfileName(entryPoint->profile);
        OSString wideProfileName = profileName.ToWString();


        IDxcOperationResult* dxcResult = nullptr;
        if (FAILED(dxcCompiler->Compile(dxcSourceBlob,
            L"slang",
            wideEntryPointName.begin(),
            wideProfileName.begin(),
            args,
            argCount,
            nullptr,        // `#define`s
            0,              // `#define` count
            nullptr,        // `#include` handler
            &dxcResult)))
        {
            return 1;
        }

        // Retrieve result.
        HRESULT resultCode = S_OK;
        if (FAILED(dxcResult->GetStatus(&resultCode)))
        {
            // This indicates that we failed to retrieve the reuslt...
            return 1;
        }

        // Note: it seems like the dxcompiler interface
        // doesn't support querying diagnostic output
        // *unless* the compile failed (no way to get
        // warnings out!?).

        // Verify compile result
        if (FAILED(resultCode))
        {
            // Compilation failed.


            // Try to read any diagnostic output.
            IDxcBlobEncoding* dxcErrorBlob = nullptr;
            if (!FAILED(dxcResult->GetErrorBuffer(&dxcErrorBlob)))
            {
                compileRequest->mSink.diagnoseRaw(
                    FAILED(resultCode) ? Severity::Error : Severity::Warning,
                    (char const*)dxcErrorBlob->GetBufferPointer());
                dxcErrorBlob->Release();
            }


            return 1;
        }

        // Okay, the compile supposedly succeeded, so we
        // just need to grab the buffer with the output DXIL.
        IDxcBlob* dxcResultBlob = nullptr;
        if (FAILED(dxcResult->GetResult(&dxcResultBlob)))
        {
            return 1;
        }

        outCode.AddRange(
            (uint8_t const*)dxcResultBlob->GetBufferPointer(),
            (int)           dxcResultBlob->GetBufferSize());

        // Clean up after ourselves.

        if(dxcResultBlob)   dxcResultBlob   ->Release();
        if(dxcResult)       dxcResult       ->Release();
        if(dxcLibrary)      dxcLibrary      ->Release();
        if(dxcCompiler)     dxcCompiler     ->Release();

        return 0;
    }

    String dissassembleDXILUsingDXC(
        CompileRequest*     compileRequest,
        void const*         data,
        size_t              size)
    {
        // First deal with all the rigamarole of loading
        // the `dxcompiler` library, and creating the
        // top-level COM objects that will be used to
        // compile things.

        static DxcCreateInstanceProc dxcCreateInstance = nullptr;
        if (!dxcCreateInstance)
        {
            auto dxcSharedLibrary = getDXCSharedLibrary(compileRequest);
            if (!dxcSharedLibrary)
                return 1;

            dxcCreateInstance = (DxcCreateInstanceProc) dxcSharedLibrary.findFuncByName("DxcCreateInstance");
            if (!dxcCreateInstance)
                return 1;
        }

        IDxcCompiler* dxcCompiler = nullptr;
        if (FAILED(dxcCreateInstance(
            CLSID_DxcCompiler,
            __uuidof(dxcCompiler),
            (LPVOID*) &dxcCompiler)))
        {
            return 1;
        }

        IDxcLibrary* dxcLibrary = nullptr;
        if (FAILED(dxcCreateInstance(
            CLSID_DxcLibrary,
            __uuidof(dxcLibrary),
            (LPVOID*) &dxcLibrary)))
        {
            return 1;
        }

        // Create blob from the input data
        IDxcBlobEncoding* dxcSourceBlob = nullptr;
        if (FAILED(dxcLibrary->CreateBlobWithEncodingFromPinned(
            (LPBYTE) data,
            (UINT32) size,
            0,
            &dxcSourceBlob)))
        {
            return 1;
        }

        IDxcBlobEncoding* dxcResultBlob = nullptr;
        if(FAILED(dxcCompiler->Disassemble(
            dxcSourceBlob,
            &dxcResultBlob)))
        {
            return 1;
        }

        String result;
        char const* codeBegin = (char const*)dxcResultBlob->GetBufferPointer();
        char const* codeEnd = codeBegin + dxcResultBlob->GetBufferSize() - 1;
        result.append(codeBegin, codeEnd);

        if(dxcResultBlob)   dxcResultBlob   ->Release();
        if(dxcLibrary)      dxcLibrary      ->Release();
        if(dxcCompiler)     dxcCompiler     ->Release();

        return result;
    }


} // namespace Slang

#endif



