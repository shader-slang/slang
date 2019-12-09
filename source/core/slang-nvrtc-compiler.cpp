// slang-nvrtc-compiler.cpp
#include "slang-nvrtc-compiler.h"

#include "slang-common.h"
#include "../../slang-com-helper.h"
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

};

namespace Slang
{
using namespace nvrtc;

class NVRTCDownstreamCompiler : public DownstreamCompiler
{
public:
    typedef DownstreamCompiler Super;

    // DownstreamCompiler
    virtual SlangResult compile(const CompileOptions& options, RefPtr<DownstreamCompileResult>& outResult) SLANG_OVERRIDE;

        /// Must be called before use
    SlangResult init(SharedLibrary::Handle handle);

    NVRTCDownstreamCompiler():m_sharedLibraryHandle(0) {}
    ~NVRTCDownstreamCompiler() {}

protected:


#define SLANG_NVTRC_MEMBER_FUNCS(ret, name, params) \
    ret (*m_##name) params;

    SLANG_NVRTC_FUNCS(SLANG_NVTRC_MEMBER_FUNCS);

    SharedLibrary::Handle m_sharedLibraryHandle;        ///< Note that this class will not release the library
};


SlangResult NVRTCDownstreamCompiler::init(SharedLibrary::Handle handle)
{
#define SLANG_NVTRC_GET_FUNC(ret, name, params)  \
    m_##name = (ret (*) params)SharedLibrary::findFuncByName(handle, #name); \
    if (m_##name == nullptr) return SLANG_FAIL;
    
    SLANG_NVRTC_FUNCS(SLANG_NVTRC_GET_FUNC)

    m_sharedLibraryHandle = handle;

    m_desc.type = CompilerType::NVRTC;

    int major, minor;
    m_nvrtcVersion(&major, &minor);
    m_desc.majorVersion = major;
    m_desc.minorVersion = minor;

    return SLANG_OK;
}


SlangResult NVRTCDownstreamCompiler::compile(const CompileOptions& options, RefPtr<DownstreamCompileResult>& outResult)
{
    SLANG_UNUSED(options);
    SLANG_UNUSED(outResult);

    return SLANG_FAIL;
}

/* static */SlangResult NVRTCDownstreamCompilerUtil::createCompiler(SharedLibrary::Handle handle, RefPtr<DownstreamCompiler>& outCompiler)
{
    RefPtr<NVRTCDownstreamCompiler> compiler(new NVRTCDownstreamCompiler);

    SLANG_RETURN_ON_FAIL(compiler->init(handle));

    outCompiler = compiler;
    return SLANG_OK;
}

}
