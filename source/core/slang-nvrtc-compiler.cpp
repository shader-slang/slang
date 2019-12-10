// slang-nvrtc-compiler.cpp
#include "slang-nvrtc-compiler.h"

#include "slang-common.h"
#include "../../slang-com-helper.h"

#include "../core/slang-blob.h"

#include "slang-string-util.h"

#include "slang-io.h"
#include "slang-shared-library.h"

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

    m_desc.type = CompilerType::NVRTC;

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

static SlangResult _parseNVRTCLine(const UnownedStringSlice& line, DownstreamDiagnostic& outDiagnostic)
{
    typedef DownstreamDiagnostic Diagnostic;
    typedef Diagnostic::Type Type;

    outDiagnostic.stage = Diagnostic::Stage::Compile;

    List<UnownedStringSlice> split;
    StringUtil::split(line, ':', split);

    if (split.getCount() == 3)
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
    

    nvrtcProgram program = nullptr;
    nvrtcResult res = m_nvrtcCreateProgram(&program, options.sourceContents.getBuffer(), options.sourceContentsPath.getBuffer(), 0, nullptr, nullptr);
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

/* static */SlangResult NVRTCDownstreamCompilerUtil::createCompiler(ISlangSharedLibrary* library, RefPtr<DownstreamCompiler>& outCompiler)
{
    RefPtr<NVRTCDownstreamCompiler> compiler(new NVRTCDownstreamCompiler);

    SLANG_RETURN_ON_FAIL(compiler->init(library));

    outCompiler = compiler;
    return SLANG_OK;
}

}
