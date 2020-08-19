// slang-dxc-support.cpp
#include "slang-compiler.h"

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

#include "../core/slang-platform.h"

namespace Slang
{
    String GetHLSLProfileName(Profile profile);


    SlangResult locateDXCCompilers(const String& path, ISlangSharedLibraryLoader* loader, DownstreamCompilerSet* set);

    static UnownedStringSlice _getSlice(IDxcBlob* blob)
    {
        if (blob)
        {
            const char* chars = (const char*)blob->GetBufferPointer();
            size_t len = blob->GetBufferSize();
            len -= size_t(len > 0 && chars[len - 1] == 0);
            return UnownedStringSlice(chars, len);
        }
        return UnownedStringSlice();
    }

    // IDxcIncludeHandler
    // 7f61fc7d-950d-467f-b3e3-3c02fb49187c
    static const Guid IID_IDxcIncludeHandler = { 0x7f61fc7d, 0x950d, 0x467f, { 0x3c, 0x02, 0xfb, 0x49, 0x18, 0x7c } };
    static const Guid IID_IUnknown = SLANG_UUID_ISlangUnknown;

    class DxcIncludeHandler : public IDxcIncludeHandler
    {
    public:
        // Implement IUnknown
        SLANG_NO_THROW HRESULT SLANG_MCALL QueryInterface(const IID& uuid, void** out)
        { 
            ISlangUnknown* intf = getInterface(reinterpret_cast<const Guid&>(uuid)); 
            if (intf) 
            { 
                *out = intf; 
                return SLANG_OK;
            } 
            return SLANG_E_NO_INTERFACE;
        }
        SLANG_NO_THROW ULONG SLANG_MCALL AddRef() SLANG_OVERRIDE { return 1; }
        SLANG_NO_THROW ULONG SLANG_MCALL Release() SLANG_OVERRIDE { return 1; }

        // Implement IDxcIncludeHandler
        virtual HRESULT SLANG_MCALL LoadSource(LPCWSTR inFilename, IDxcBlob** outSource) SLANG_OVERRIDE
        {
            // Hmm DXC does something a bit odd - when it sees a path, it just passes that in with ./ in front!!
            // NOTE! It doesn't make any difference if it is "" or <> quoted.

            // So we just do a work around where we strip if we see a path starting with ./
            String filePath = String::fromWString(inFilename);

            // If it starts with ./ then attempt to strip it
            if (filePath.startsWith("./"))
            {
                String remaining(filePath.subString(2, filePath.getLength() - 2));

                // Okay if we strip ./ and what we have is absolute, then it's the absolute path that we care about,
                // otherwise we just leave as is.
                if (Path::isAbsolute(remaining))
                {
                    filePath = remaining;
                }
            }

            ComPtr<ISlangBlob> blob;
            PathInfo pathInfo;
            SlangResult res = m_system.findAndLoadFile(filePath, String(), pathInfo, blob);

            // NOTE! This only works because ISlangBlob is *binary compatible* with IDxcBlob, if either
            // change things could go boom
            *outSource = (IDxcBlob*)blob.detach();
            return res;
        }

        DxcIncludeHandler(SearchDirectoryList* searchDirectories, ISlangFileSystemExt* fileSystemExt, SourceManager* sourceManager = nullptr) :
            m_system(searchDirectories, fileSystemExt, sourceManager)
        {
        }

    protected:

        // Used by QueryInterface for casting
        ISlangUnknown* getInterface(const Guid& guid)
        {
            if (guid == IID_IUnknown || guid == IID_IDxcIncludeHandler)
            {
                return (ISlangUnknown*)(static_cast<IDxcIncludeHandler*>(this));
            }
            return nullptr;
        }

        IncludeSystem m_system;
    };


    SlangResult emitDXILForEntryPointUsingDXC(
        ComponentType*          program,
        BackEndCompileRequest*  compileRequest,
        Int                     entryPointIndex,
        TargetRequest*          targetReq,
        EndToEndCompileRequest* endToEndReq,
        List<uint8_t>&          outCode)
    {
        auto session = compileRequest->getSession();
        auto sink = compileRequest->getSink();

        // First deal with all the rigamarole of loading
        // the `dxcompiler` library, and creating the
        // top-level COM objects that will be used to
        // compile things.

        auto dxcCreateInstance = (DxcCreateInstanceProc)session->getSharedLibraryFunc(Session::SharedLibraryFuncType::Dxc_DxcCreateInstance, sink);
        if (!dxcCreateInstance)
        {
            return SLANG_FAIL;
        }

        ComPtr<IDxcCompiler> dxcCompiler;
        SLANG_RETURN_ON_FAIL(dxcCreateInstance(
            CLSID_DxcCompiler,
            __uuidof(dxcCompiler),
            (LPVOID*)dxcCompiler.writeRef()));

        ComPtr<IDxcLibrary> dxcLibrary;
        SLANG_RETURN_ON_FAIL(dxcCreateInstance(
            CLSID_DxcLibrary,
            __uuidof(dxcLibrary),
            (LPVOID*)dxcLibrary.writeRef()));

        // Now let's go ahead and generate HLSL for the entry
        // point, since we'll need that to feed into dxc.
        SourceResult source;
        SLANG_RETURN_ON_FAIL(emitEntryPointSource(compileRequest, entryPointIndex,
            targetReq, CodeGenTarget::HLSL, endToEndReq, source));

        const auto& hlslCode = source.source;

        maybeDumpIntermediate(compileRequest, hlslCode.getBuffer(), CodeGenTarget::HLSL);

        // Wrap the 

        // Create blob from the string
        ComPtr<IDxcBlobEncoding> dxcSourceBlob;
        SLANG_RETURN_ON_FAIL(dxcLibrary->CreateBlobWithEncodingFromPinned(
            (LPBYTE)hlslCode.getBuffer(),
            (UINT32)hlslCode.getLength(),
            0,
            dxcSourceBlob.writeRef()));

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

        switch( targetReq->getFloatingPointMode() )
        {
        default:
            break;

        case FloatingPointMode::Precise:
            args[argCount++] = L"-Gis"; // "force IEEE strictness"
            break;
        }

        auto linkage = compileRequest->getLinkage();
        switch( linkage->optimizationLevel )
        {
        default:
            break;

        case OptimizationLevel::None:       args[argCount++] = L"-Od"; break;
        case OptimizationLevel::Default:    args[argCount++] = L"-O1"; break;
        case OptimizationLevel::High:       args[argCount++] = L"-O2"; break;
        case OptimizationLevel::Maximal:    args[argCount++] = L"-O3"; break;
        }

        switch( linkage->debugInfoLevel )
        {
        case DebugInfoLevel::None:
            break;

        default:
            args[argCount++] = L"-Zi";
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

        EntryPoint* entryPoint = program->getEntryPoint(entryPointIndex);
        String entryPointName = getText(entryPoint->getName());
        OSString wideEntryPointName = entryPointName.toWString();

        auto profile = getEffectiveProfile(entryPoint, targetReq);
        String profileName = GetHLSLProfileName(profile);
        OSString wideProfileName = profileName.toWString();

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
        if( profile.getVersion() >= ProfileVersion::DX_6_2 )
        {
            args[argCount++] = L"-enable-16bit-types";
        }

        const String sourcePath = calcSourcePathForEntryPoint(endToEndReq, entryPointIndex);

        DxcIncludeHandler includeHandler(&linkage->searchDirectories, linkage->getFileSystemExt(), compileRequest->getSourceManager());

        ComPtr<IDxcOperationResult> dxcResult;
        SLANG_RETURN_ON_FAIL(dxcCompiler->Compile(dxcSourceBlob,
            sourcePath.toWString().begin(),
            profile.getStage() == Stage::Unknown ? L"" : wideEntryPointName.begin(),
            wideProfileName.begin(),
            args,
            argCount,
            nullptr,            // `#define`s
            0,                  // `#define` count
            &includeHandler,    // `#include` handler
            dxcResult.writeRef()));

        // Retrieve result.
        HRESULT resultCode = S_OK;
        SLANG_RETURN_ON_FAIL(dxcResult->GetStatus(&resultCode));
        
        // Note: it seems like the dxcompiler interface
        // doesn't support querying diagnostic output
        // *unless* the compile failed (no way to get
        // warnings out!?).

        // Verify compile result
        if (SLANG_FAILED(resultCode))
        {
            // Compilation failed.
            // Try to read any diagnostic output.
            ComPtr<IDxcBlobEncoding> dxcErrorBlob; 
            SLANG_RETURN_ON_FAIL(dxcResult->GetErrorBuffer(dxcErrorBlob.writeRef()));

            // Note: the error blob returned by dxc doesn't always seem
            // to be nul-terminated, so we should be careful and turn it
            // into a string for safety.
            //

            reportExternalCompileError("dxc", resultCode, _getSlice(dxcErrorBlob), compileRequest->getSink());
            return resultCode;
        }

        // Okay, the compile supposedly succeeded, so we
        // just need to grab the buffer with the output DXIL.
        ComPtr<IDxcBlob> dxcResultBlob;
        SLANG_RETURN_ON_FAIL(dxcResult->GetResult(dxcResultBlob.writeRef()));
        
        outCode.addRange(
            (uint8_t const*)dxcResultBlob->GetBufferPointer(),
            (int)           dxcResultBlob->GetBufferSize());

        return SLANG_OK;
    }

    SlangResult dissassembleDXILUsingDXC(
        BackEndCompileRequest*  compileRequest,
        void const*             data,
        size_t                  size,
        String&                 stringOut)
    {
        stringOut = String();
        auto session = compileRequest->getSession();
        auto sink = compileRequest->getSink();

        // First deal with all the rigamarole of loading
        // the `dxcompiler` library, and creating the
        // top-level COM objects that will be used to
        // compile things.

        auto dxcCreateInstance = (DxcCreateInstanceProc)session->getSharedLibraryFunc(Session::SharedLibraryFuncType::Dxc_DxcCreateInstance, sink);
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

        stringOut = _getSlice(dxcResultBlob);
        
        return SLANG_OK;
    }


} // namespace Slang

#endif



