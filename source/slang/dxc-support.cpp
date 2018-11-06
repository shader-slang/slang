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
    String GetHLSLProfileName(Profile profile);
    String emitHLSLForEntryPoint(
        EntryPointRequest*  entryPoint,
        TargetRequest*      targetReq);

    int emitDXILForEntryPointUsingDXC(
        EntryPointRequest*  entryPoint,
        TargetRequest*      targetReq,
        List<uint8_t>&      outCode)
    {
        auto compileRequest = entryPoint->compileRequest;
        auto session = compileRequest->mSession;

        // First deal with all the rigamarole of loading
        // the `dxcompiler` library, and creating the
        // top-level COM objects that will be used to
        // compile things.

        auto dxcCreateInstance = (DxcCreateInstanceProc)session->getSharedLibraryFunc(Session::SharedLibraryFuncType::Dxc_DxcCreateInstance, &compileRequest->mSink);
        if (!dxcCreateInstance)
        {
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

        switch( targetReq->getDefaultMatrixLayoutMode() )
        {
        default:
            break;

        case kMatrixLayoutMode_RowMajor:
            args[argCount++] = L"-Zpr";
            break;
        }

        switch( targetReq->floatingPointMode )
        {
        default:
            break;

        case FloatingPointMode::Precise:
            args[argCount++] = L"-Gis"; // "force IEEE strictness"
            break;
        }

        // Slang strives to produce correct code, and by default
        // we do not show the user warnings produced by a downstream
        // compiler. When the downstream compiler *does* produce an
        // error, then we dump its entire diagnostic log, which can
        // include many distracting spurious warnings that have nothing
        // to do with the user's code, and just relate to the idiomatic
        // way that Slang outputs HLSL.
        //
        // It would be nice to use fine-grained flags to disable specific
        // warnings here, so that we keep ourselves honest (e.g., only
        // use `-Wno-parentheses` to eliminate that class of false positives),
        // but alas dxc doesn't support these options even though they
        // work on mainline Clang. Thus the only option we have available
        // is the big hammer of turning off *all* warnings coming from dxc.
        //
        args[argCount++] = L"-no-warnings";

        String entryPointName = getText(entryPoint->name);
        OSString wideEntryPointName = entryPointName.ToWString();

        auto profile = getEffectiveProfile(entryPoint, targetReq);
        String profileName = GetHLSLProfileName(profile);
        OSString wideProfileName = profileName.ToWString();

        // We will enable the flag to generate proper code for 16-bit types
        // by default, as long as the user is requesting a sufficiently
        // high shader model.
        //
        // TODO: Need to check that this is safe to enable in all cases,
        // or if it will make a shader demand hardware features that
        // aren't always present.
        //
        // TODO: Ideally the dxc back-end should be passed some information
        // on the "capabilities" that were used and/or requested in the code.
        //
        if( profile.GetVersion() >= ProfileVersion::DX_6_2 )
        {
            args[argCount++] = L"-enable-16bit-types";
        }

        IDxcOperationResult* dxcResult = nullptr;
        if (FAILED(dxcCompiler->Compile(dxcSourceBlob,
            L"slang",
            profile.GetStage() == Stage::Unknown ? L"" : wideEntryPointName.begin(),
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
                // Note: the error blob returned by dxc doesn't always seem
                // to be nul-terminated, so we should be careful and turn it
                // into a string for safety.
                //
                // TODO: Alternatively, `diagnoseRaw()` should accept an
                // `UnownedStringSlice` instead of a `const char*`.
                //
                auto errorBegin = (char const*) dxcErrorBlob->GetBufferPointer();
                auto errorEnd = errorBegin + dxcErrorBlob->GetBufferSize();
                String errorString = UnownedStringSlice(errorBegin, errorEnd);

                compileRequest->mSink.diagnoseRaw(
                    FAILED(resultCode) ? Severity::Error : Severity::Warning,
                    errorString.Buffer());
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

    SlangResult dissassembleDXILUsingDXC(
        CompileRequest*     compileRequest,
        void const*         data,
        size_t              size,
        String&             stringOut)
    {
        stringOut = String();
        auto session = compileRequest->mSession;

        // First deal with all the rigamarole of loading
        // the `dxcompiler` library, and creating the
        // top-level COM objects that will be used to
        // compile things.

        auto dxcCreateInstance = (DxcCreateInstanceProc)session->getSharedLibraryFunc(Session::SharedLibraryFuncType::Dxc_DxcCreateInstance, &compileRequest->mSink);
        if (!dxcCreateInstance)
        {
            return SLANG_FAIL;
        }

        ComPtr<IDxcCompiler> dxcCompiler;
        SLANG_RETURN_ON_FAIL(dxcCreateInstance(CLSID_DxcCompiler, __uuidof(dxcCompiler), (LPVOID*) dxcCompiler.writeRef()));
        ComPtr<IDxcLibrary> dxcLibrary;
        SLANG_RETURN_ON_FAIL(dxcCreateInstance(CLSID_DxcLibrary, __uuidof(dxcLibrary), (LPVOID*) dxcLibrary.writeRef()));
        
        // Create blob from the input data
        ComPtr<IDxcBlobEncoding> dxcSourceBlob;
        SLANG_RETURN_ON_FAIL(dxcLibrary->CreateBlobWithEncodingFromPinned((LPBYTE) data, (UINT32) size, 0, dxcSourceBlob.writeRef()));

        ComPtr<IDxcBlobEncoding> dxcResultBlob;
        SLANG_RETURN_ON_FAIL(dxcCompiler->Disassemble(dxcSourceBlob, dxcResultBlob.writeRef()));

        String result;
        char const* codeBegin = (char const*)dxcResultBlob->GetBufferPointer();
        char const* codeEnd = codeBegin + dxcResultBlob->GetBufferSize() - 1;
        result.append(codeBegin, codeEnd);
        stringOut = result;

        return SLANG_OK;
    }


} // namespace Slang

#endif



