// slang-nvrtc-compiler.cpp
#include "slang-nvrtc-compiler.h"

#include "slang-common.h"
#include "../../slang-com-helper.h"

#include "../core/slang-blob.h"

#include "slang-string-util.h"

#include "slang-io.h"
#include "slang-shared-library.h"
#include "slang-semantic-version.h"

namespace nvrtc
{

typedef enum {
  NVRTC_SUCCESS = 0,
  NVRTC_ERROR_OUT_OF_MEMORY = 1,
  NVRTC_ERROR_PROGRAM_CREATION_FAILURE = 2,
  NVRTC_ERROR_INVALID_INPUT = 3,
  NVRTC_ERROR_INVALID_PROGRAM = 4,
  NVRTC_ERROR_INVALID_OPTION = 5,
  NVRTC_ERROR_COMPILATION = 6,
  NVRTC_ERROR_BUILTIN_OPERATION_FAILURE = 7,
  NVRTC_ERROR_NO_NAME_EXPRESSIONS_AFTER_COMPILATION = 8,
  NVRTC_ERROR_NO_LOWERED_NAMES_BEFORE_COMPILATION = 9,
  NVRTC_ERROR_NAME_EXPRESSION_NOT_VALID = 10,
  NVRTC_ERROR_INTERNAL_ERROR = 11
} nvrtcResult;

typedef struct _nvrtcProgram *nvrtcProgram;

#define SLANG_NVRTC_FUNCS(x) \
    x(const char*, nvrtcGetErrorString, (nvrtcResult result)) \
    x(nvrtcResult, nvrtcVersion, (int *major, int *minor)) \
    x(nvrtcResult, nvrtcCreateProgram, (nvrtcProgram *prog, const char *src, const char *name, int numHeaders, const char * const *headers, const char * const *includeNames)) \
    x(nvrtcResult, nvrtcDestroyProgram, (nvrtcProgram *prog)) \
    x(nvrtcResult, nvrtcCompileProgram, (nvrtcProgram prog, int numOptions, const char * const *options)) \
    x(nvrtcResult, nvrtcGetPTXSize, (nvrtcProgram prog, size_t *ptxSizeRet)) \
    x(nvrtcResult, nvrtcGetPTX, (nvrtcProgram prog, char *ptx)) \
    x(nvrtcResult, nvrtcGetProgramLogSize, (nvrtcProgram prog, size_t *logSizeRet)) \
    x(nvrtcResult, nvrtcGetProgramLog, (nvrtcProgram prog, char *log))\
    x(nvrtcResult, nvrtcAddNameExpression, (nvrtcProgram prog, const char * const name_expression)) \
    x(nvrtcResult, nvrtcGetLoweredName, (nvrtcProgram prog, const char *const name_expression, const char** lowered_name))

} // namespace nvrtc

namespace Slang
{
using namespace nvrtc;

static SlangResult _asResult(nvrtcResult res)
{
    switch (res)
    {
        case NVRTC_SUCCESS:
        {
            return SLANG_OK;
        }
        case NVRTC_ERROR_OUT_OF_MEMORY:
        {
            return SLANG_E_OUT_OF_MEMORY;
        }
        case NVRTC_ERROR_PROGRAM_CREATION_FAILURE: 
        case NVRTC_ERROR_INVALID_INPUT:
        case NVRTC_ERROR_INVALID_PROGRAM:
        {
            return SLANG_FAIL;
        }
        case NVRTC_ERROR_INVALID_OPTION:
        {
            return SLANG_E_INVALID_ARG;
        }
        case NVRTC_ERROR_COMPILATION:
        case NVRTC_ERROR_BUILTIN_OPERATION_FAILURE:
        case NVRTC_ERROR_NO_NAME_EXPRESSIONS_AFTER_COMPILATION:
        case NVRTC_ERROR_NO_LOWERED_NAMES_BEFORE_COMPILATION:
        case NVRTC_ERROR_NAME_EXPRESSION_NOT_VALID:
        {
            return SLANG_FAIL;
        }
        case NVRTC_ERROR_INTERNAL_ERROR:
        {
            return SLANG_E_INTERNAL_FAIL;
        }
        default: return SLANG_FAIL;
    }
}

class NVRTCDownstreamCompiler : public DownstreamCompiler
{
public:
    typedef DownstreamCompiler Super;

    // DownstreamCompiler
    virtual SlangResult compile(const CompileOptions& options, RefPtr<DownstreamCompileResult>& outResult) SLANG_OVERRIDE;
    virtual ISlangSharedLibrary* getSharedLibrary() SLANG_OVERRIDE { return m_sharedLibrary; }

        /// Must be called before use
    SlangResult init(ISlangSharedLibrary* library);

    NVRTCDownstreamCompiler() {}
    
protected:

    struct ScopeProgram
    {
        ScopeProgram(NVRTCDownstreamCompiler* compiler, nvrtcProgram program):
            m_compiler(compiler),
            m_program(program)
        {
        }
        ~ScopeProgram()
        {
            m_compiler->m_nvrtcDestroyProgram(&m_program);
        }
        NVRTCDownstreamCompiler* m_compiler;
        nvrtcProgram m_program;
    };


#define SLANG_NVTRC_MEMBER_FUNCS(ret, name, params) \
    ret (*m_##name) params;

    SLANG_NVRTC_FUNCS(SLANG_NVTRC_MEMBER_FUNCS);

    ComPtr<ISlangSharedLibrary> m_sharedLibrary;  
};

#define SLANG_NVRTC_RETURN_ON_FAIL(x) { nvrtcResult _res = x; if (_res != NVRTC_SUCCESS) return _asResult(_res); } 

SlangResult NVRTCDownstreamCompiler::init(ISlangSharedLibrary* library)
{
#define SLANG_NVTRC_GET_FUNC(ret, name, params)  \
    m_##name = (ret (*) params)library->findFuncByName(#name); \
    if (m_##name == nullptr) return SLANG_FAIL;

    SLANG_NVRTC_FUNCS(SLANG_NVTRC_GET_FUNC)

    m_sharedLibrary = library;

    m_desc.type = SLANG_PASS_THROUGH_NVRTC;

    int major, minor;
    m_nvrtcVersion(&major, &minor);
    m_desc.majorVersion = major;
    m_desc.minorVersion = minor;

    return SLANG_OK;
}

static SlangResult _parseLocation(const UnownedStringSlice& in, DownstreamDiagnostic& outDiagnostic)
{
    const Index startIndex = in.indexOf('(');

    if (startIndex >= 0)
    {
        outDiagnostic.filePath = UnownedStringSlice(in.begin(), in.begin() + startIndex);
        UnownedStringSlice remaining(in.begin() + startIndex + 1, in.end());
        const Int endIndex = remaining.indexOf(')');

        UnownedStringSlice lineText = UnownedStringSlice(remaining.begin(), remaining.begin() + endIndex);

        Int line;
        SLANG_RETURN_ON_FAIL(StringUtil::parseInt(lineText, line));
        outDiagnostic.fileLine = line;
    }
    else
    {
        outDiagnostic.fileLine = 0;
        outDiagnostic.filePath = in;
    }
    return SLANG_OK;
}

static bool _isDriveLetter(char c)
{
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

static bool _hasDriveLetter(const UnownedStringSlice& line)
{
    return line.getLength() > 2 && line[1] == ':' && _isDriveLetter(line[0]);
}

static SlangResult _parseNVRTCLine(const UnownedStringSlice& line, DownstreamDiagnostic& outDiagnostic)
{
    typedef DownstreamDiagnostic Diagnostic;
    typedef Diagnostic::Type Type;

    outDiagnostic.stage = Diagnostic::Stage::Compile;

    List<UnownedStringSlice> split;
    if (_hasDriveLetter(line))
    {
        // The drive letter has :, which confuses things, so skip that and then fix up first entry 
        UnownedStringSlice lineWithoutDrive(line.begin() + 2, line.end());
        StringUtil::split(lineWithoutDrive, ':', split);
        split[0] = UnownedStringSlice(line.begin(), split[0].end());
    }
    else
    {
        StringUtil::split(line, ':', split);
    }

    if (split.getCount() >= 3)
    {
        // tests/cuda/cuda-compile.cu(7): warning: variable "c" is used before its value is set

        const auto split1 = split[1].trim();

        if (split1 == "error")
        {
            outDiagnostic.type = Type::Error;
        }
        else if (split1 == "warning")
        {
            outDiagnostic.type = Type::Warning;
        }
        outDiagnostic.text = split[2].trim();

        SLANG_RETURN_ON_FAIL(_parseLocation(split[0], outDiagnostic));
        return SLANG_OK;
    }
   
    return SLANG_E_NOT_FOUND;
}

SlangResult NVRTCDownstreamCompiler::compile(const CompileOptions& options, RefPtr<DownstreamCompileResult>& outResult)
{
    // This compiler doesn't read files, they should be read externally and stored in sourceContents/sourceContentsPath
    if (options.sourceFiles.getCount() > 0)
    {
        return SLANG_FAIL;
    }

    CommandLine cmdLine;

    switch (options.debugInfoType)
    {
        case DebugInfoType::None:
        {
            break;
        }
        default:
        {
            cmdLine.addArg("--device-debug");
            break;
        }
        case DebugInfoType::Maximal:
        {
            cmdLine.addArg("--device-debug");
            cmdLine.addArg("--generate-line-info");
            break;
        }
    }

    // Don't seem to have such a control, so ignore for now
    //switch (options.optimizationLevel)
    //{
    //    default: break;
    //}

    switch (options.floatingPointMode)
    {
        case FloatingPointMode::Default: break;
        case FloatingPointMode::Precise:
        {
            break;
        }
        case FloatingPointMode::Fast:
        {
            cmdLine.addArg("--use_fast_math");
            break;
        }
    }

    // Add defines
    for (const auto& define : options.defines)
    {
        StringBuilder builder;
        builder << "-D";
        builder << define.nameWithSig;
        if (define.value.getLength())
        {
            builder << "=" << define.value;
        }

        cmdLine.addArg(builder);
    }

    // Add includes
    for (const auto& include : options.includePaths)
    {
        cmdLine.addArg("-I");
        cmdLine.addArg(include);
    }

    // Neither of these options are strictly required, for general use of nvrtc,
    // but are enabled to make use withing Slang work more smoothly
    {
        // Require c++14, as makes initialization construction with {} available and so simplifies code generation
        cmdLine.addArg("-std=c++14");

        // Disable all warnings
        // This is arguably too much - but nvrtc does not appear to have a mechanism to switch off individual warnings.
        // I tried the -Xcudafe mechanism but that does not appear to work for nvrtc
        cmdLine.addArg("-w");
    }

    {
        // The lowest supported CUDA architecture version supported
        // by NVRTC is `compute_30`.
        //
        SemanticVersion version(3);

        // Newer releases of NVRTC only support `compute_35` and up
        // (with everything before `compute_52` being deprecated).
        //
        if( m_desc.majorVersion >= 11 )
        {
            version = SemanticVersion(3, 5);
        }

        // If constructs used in the code to be compield require
        // a higher architecture version than the minimum, then
        // we will set the version to the highest version listed
        // among the requirements.
        //
        for (const auto& capabilityVersion : options.requiredCapabilityVersions)
        {
            if (capabilityVersion.kind == DownstreamCompiler::CapabilityVersion::Kind::CUDASM)
            {
                if (capabilityVersion.version > version)
                {
                    version = capabilityVersion.version;
                }
            }
        }

        StringBuilder builder;
        builder << "-arch=compute_";
        builder << version.m_major;

        SLANG_ASSERT(version.m_minor >= 0 && version.m_minor <= 9);
        builder << char('0' + version.m_minor);

        cmdLine.addArg(builder);
    }

    List<const char*> headers;
    List<const char*> headerIncludeNames;

    // If compiling for OptiX, we need to add the appropriate search paths to the command line.
    //
    if(options.pipelineType == PipelineType::RayTracing)
    {
        // The device-side OptiX API is accessed through a constellation
        // of headers provided by the OptiX SDK, so we need to set an
        // include path for the compile that makes those visible.
        //
        // TODO: The OptiX SDK installer doesn't set any kind of environment
        // variable to indicate where the SDK was installed, so we seemingly
        // need to probe paths instead. The form of the path will differ
        // betwene Windows and Unix-y platforms, and we will need some kind
        // of approach to probe multiple versiosn and use the latest.
        //
        // HACK: For now I'm using the fixed path for the most recent SDK
        // release on Windows. This means that OptiX cross-compilation will
        // only "work" on a subset of platforms, but that doesn't matter
        // for now since it doesn't really "work" at all.
        //
        cmdLine.addArg("-I");
        cmdLine.addArg("C:/ProgramData/NVIDIA Corporation/OptiX SDK 7.0.0/include/");

        // The OptiX headers in turn `#include <stddef.h>` and expect that
        // to work. We could try to also add in an include path from the CUDA
        // SDK (which seems to provide a `stddef.h` in the most recent version),
        // but using that version doesn't seem to work (and also bakes in a
        // requirement that the user have the CUDA SDK installed in addition
        // to the OptiX SDK).
        //
        // Instead, we will rely on the NVRTC feature that lets us set up
        // memory buffers to be used as include files by the we compile.
        // We will define a dummy `stddef.h` that includes the bare minimum
        // lines required to get the OptiX headers to compile without complaint.
        //
        // TODO: Confirm that the `LP64` definition herei s actually needed.
        //
        headerIncludeNames.add("stddef.h");
        headers.add("#pragma once\n" "#define LP64\n");

        // Finally, we want the CUDA prelude to be able to react to whether
        // or not OptiX is required (most notably by `#include`ing the appropriate
        // header(s)), so we will insert a preprocessor define to indicate
        // the requirement.
        //
        cmdLine.addArg("-DSLANG_CUDA_ENABLE_OPTIX");
    }

    SLANG_ASSERT(headers.getCount() == headerIncludeNames.getCount());

    nvrtcProgram program = nullptr;
    nvrtcResult res = m_nvrtcCreateProgram(&program, options.sourceContents.getBuffer(), options.sourceContentsPath.getBuffer(),
        (int) headers.getCount(),
        headers.getBuffer(),
        headerIncludeNames.getBuffer());
    if (res != NVRTC_SUCCESS)
    {
        return _asResult(res);
    }
    ScopeProgram scope(this, program);

    List<const  char*> dstOptions;
    dstOptions.setCount(cmdLine.m_args.getCount());
    for (Index i = 0; i < cmdLine.m_args.getCount(); ++i)
    {
        dstOptions[i] = cmdLine.m_args[i].value.getBuffer();
    }

    res  = m_nvrtcCompileProgram(program, int(dstOptions.getCount()), dstOptions.getBuffer());

    RefPtr<ListBlob> blob;
    DownstreamDiagnostics diagnostics;

    diagnostics.result = _asResult(res);

    {
        String rawDiagnostics;

        size_t logSize = 0;
        SLANG_NVRTC_RETURN_ON_FAIL(m_nvrtcGetProgramLogSize(program, &logSize));

        if (logSize)
        {
            char* dst = rawDiagnostics.prepareForAppend(Index(logSize));
            SLANG_NVRTC_RETURN_ON_FAIL(m_nvrtcGetProgramLog(program, dst));
            rawDiagnostics.appendInPlace(dst, Index(logSize));

            diagnostics.rawDiagnostics = rawDiagnostics;
        }

        // Parse the diagnostics here
        for (auto line : LineParser(diagnostics.rawDiagnostics.getUnownedSlice()))
        {
            DownstreamDiagnostic diagnostic;
            SlangResult lineRes = _parseNVRTCLine(line, diagnostic);

            if (SLANG_SUCCEEDED(lineRes))
            {
                diagnostics.diagnostics.add(diagnostic);
            }
            else if (lineRes != SLANG_E_NOT_FOUND)
            {
                return lineRes;
            }
        }

        // if it has a compilation error.. set on output
        if (diagnostics.has(DownstreamDiagnostic::Type::Error))
        {
            diagnostics.result = SLANG_FAIL;
        }
    }

    if (res == nvrtc::NVRTC_SUCCESS)
    {
        // We should parse the log to set up the diagnostics
        size_t ptxSize;
        SLANG_NVRTC_RETURN_ON_FAIL(m_nvrtcGetPTXSize(program, &ptxSize));

        List<uint8_t> ptx;
        ptx.setCount(Index(ptxSize));

        SLANG_NVRTC_RETURN_ON_FAIL(m_nvrtcGetPTX(program, (char*)ptx.getBuffer()));

        blob = ListBlob::moveCreate(ptx);
    }

    outResult = new BlobDownstreamCompileResult(diagnostics, blob);

    return SLANG_OK;
}

/* static */SlangResult NVRTCDownstreamCompilerUtil::locateCompilers(const String& path, ISlangSharedLibraryLoader* loader, DownstreamCompilerSet* set)
{
    ComPtr<ISlangSharedLibrary> library;

    if (path.getLength() != 0)
    {
        SLANG_RETURN_ON_FAIL(loader->loadSharedLibrary(path.getBuffer(), library.writeRef()));
    }
    else
    {
        // If the user doesn't supply a path to their preferred version of NVRTC,
        // we will search for a suitable library version, proceeding from more
        // recent versions to less recent ones.
        //
        // TODO: The list here was cobbled together from what NRTC releases I
        // could easily identify. It would be good to ver this against some
        // kind of official list.
        //
        // It would probably be good to support 32- and 64-bit here, and also
        // to deal with any variation in the shared library name across platforms
        // (
        //
        static const char* kNVRTCLibraryNames[]
        {
            // As a catch-all for non-Windows platforms, we search for
            // a library simply named `nvrtc` (well, `libnvrtc`) which
            // is expected to match whatever the user has installed.
            //
            "nvrtc",

            "nvrtc64_110_0",
            "nvrtc64_102_0",
            "nvrtc64_101_0",
            "nvrtc64_100_0",
            "nvrtc64_92",
            "nvrtc64_91",
            "nvrtc64_90",
            "nvrtc64_80",
            "nvrtc64_75",
        };

        SlangResult result = SLANG_FAIL;
        for( auto libraryName : kNVRTCLibraryNames )
        {
            // If we succeed at loading one of the library versions
            // from our list, we will not continue to search; this
            // approach assumes that the `kNVRTCLibraryNames` array
            // has been sorted so that earlier entries are preferable.
            //
            result = loader->loadSharedLibrary(libraryName, library.writeRef());
            if(!SLANG_FAILED(result))
                break;
        }

        // If we tried to load all of the candidate versions and none
        // was successful, then we report back a failure.
        //
        if(SLANG_FAILED(result))
            return result;
    }

    RefPtr<NVRTCDownstreamCompiler> compiler(new NVRTCDownstreamCompiler);
    SLANG_RETURN_ON_FAIL(compiler->init(library));

    set->addCompiler(compiler);
    return SLANG_OK;
}


}
