// slang-glslang.cpp
#include "slang-glslang.h"


#include "StandAlone/ResourceLimits.h"
#include "StandAlone/Worklist.h"
#include "glslang/Include/ShHandle.h"
#include "glslang/Public/ShaderLang.h"
#include "SPIRV/GlslangToSpv.h"
#include "SPIRV/GLSL.std.450.h"
#include "SPIRV/doc.h"
#include "SPIRV/disassemble.h"

#include "OGLCompilersDLL/InitializeDll.h"

#include "../../slang.h"

#include "spirv-tools/optimizer.hpp"
#include "spirv-tools/libspirv.h"

#if 0
#include <cstring>
#include <cstdlib>
#include <cctype>
#include <cmath>
#include <array>
#include <memory>
#include <thread>
#endif

#ifdef _WIN32
#include <Windows.h>
#endif

#include <memory>
#include <sstream>

// This is a wrapper to allow us to run the `glslang` compiler
// in a controlled fashion.

#define UNLIMITED 9999

static TBuiltInResource _calcBuiltinResources()
{
    // NOTE! This is a bit of a hack - to set all the fields to true/UNLIMITED.
    // Care must be taken if new variables are introduced, the default may not be appropriate.
    
    // We are relying on limits being after the other fields. 
    SLANG_COMPILE_TIME_ASSERT(SLANG_OFFSET_OF(TBuiltInResource, limits) > 0);
    // We are relying on maxLights being the first parameter, and all values will have the same type
    SLANG_COMPILE_TIME_ASSERT(SLANG_OFFSET_OF(TBuiltInResource, maxLights) == 0);
    
    TBuiltInResource resource;
    // Set up all the integer values.
    {
        
        auto* dst = &resource.maxLights;
        const size_t count = SLANG_OFFSET_OF(TBuiltInResource, limits) / sizeof(*dst);
        for (size_t i = 0; i < count; ++i)
        {
            dst[i] = UNLIMITED;
        }
    }

    // In the sea of variables there is a min value
    resource.minProgramTexelOffset = -UNLIMITED;

    // Set up the bools
    {
        TLimits* limits = &resource.limits;
        bool* dst = (bool*)limits;

        const size_t count = sizeof(TLimits) / sizeof(bool);
        for (size_t i = 0; i < count; ++i)
        {
            dst[i] = true;
        }
    }
    return resource;
}

static TBuiltInResource gResources = _calcBuiltinResources();

static void dump(
    void const*         data,
    size_t              size,
    glslang_OutputFunc  outputFunc,
    void*               outputUserData,
    FILE*               fallbackStream)
{
    if( outputFunc )
    {
        outputFunc(data, size, outputUserData);
    }
    else
    {
        fwrite(data, 1, size, fallbackStream);

        // also output it for debug purposes
        std::string str((char const*)data, size);
    #ifdef _WIN32
        OutputDebugStringA(str.c_str());
    #else
        fprintf(stderr, "%s\n", str.c_str());;
    #endif
    }
}

static void dumpDiagnostics(
    const glslang_CompileRequest_1_1& request,
    std::string const&      log)
{
    dump(log.c_str(), log.length(), request.diagnosticFunc, request.diagnosticUserData, stderr);
}

// Apply the SPIRV-Tools optimizer to generated SPIR-V based on the desired optimization level
// TODO: add flag for optimizing SPIR-V size as well
static void glslang_optimizeSPIRV(std::vector<unsigned int>& spirv, spv_target_env targetEnv, unsigned optimizationLevel, unsigned debugInfoType)
{
    spvtools::Optimizer optimizer(targetEnv);
    optimizer.SetMessageConsumer(
        [](spv_message_level_t level, const char *source, const spv_position_t &position, const char *message) {
        auto &out = std::cerr;
        switch (level)
        {
        case SPV_MSG_FATAL:
        case SPV_MSG_INTERNAL_ERROR:
        case SPV_MSG_ERROR:
            out << "error: ";
            break;
        case SPV_MSG_WARNING:
            out << "warning: ";
            break;
        case SPV_MSG_INFO:
        case SPV_MSG_DEBUG:
            out << "info: ";
            break;
        default:
            break;
        }
        if (source)
        {
            out << source << ":";
        }
        out << position.line << ":" << position.column << ":" << position.index << ":";
        if (message)
        {
            out << " " << message;
        }
        out << std::endl;
    });

    // If debug info is being generated, propagate
    // line information into all SPIR-V instructions. This avoids loss of
    // information when instructions are deleted or moved. Later, remove
    // redundant information to minimize final SPRIR-V size.
    if (debugInfoType != SLANG_DEBUG_INFO_LEVEL_NONE)
    {
        optimizer.RegisterPass(spvtools::CreatePropagateLineInfoPass());
    }

    // TODO confirm which passes we want to invoke for each level
    switch (optimizationLevel)
    {
    case SLANG_OPTIMIZATION_LEVEL_NONE:
        // Don't register any passes if our optimization level is none
        break;
    case SLANG_OPTIMIZATION_LEVEL_DEFAULT:
        // Use a minimal set of performance settings
        // If we run CreateInlineExhaustivePass, We need to run CreateMergeReturnPass first. 
        optimizer.RegisterPass(spvtools::CreateMergeReturnPass());
        optimizer.RegisterPass(spvtools::CreateInlineExhaustivePass());
        optimizer.RegisterPass(spvtools::CreateAggressiveDCEPass());
        optimizer.RegisterPass(spvtools::CreatePrivateToLocalPass());
        optimizer.RegisterPass(spvtools::CreateScalarReplacementPass(100));
        optimizer.RegisterPass(spvtools::CreateLocalAccessChainConvertPass());
        optimizer.RegisterPass(spvtools::CreateAggressiveDCEPass());
        break;
    case SLANG_OPTIMIZATION_LEVEL_HIGH:
    case SLANG_OPTIMIZATION_LEVEL_MAXIMAL:
        // Use the same passes when specifying the "-O" flag in spirv-opt
        optimizer.RegisterPass(spvtools::CreateWrapOpKillPass());
        optimizer.RegisterPass(spvtools::CreateDeadBranchElimPass());
        optimizer.RegisterPass(spvtools::CreateMergeReturnPass());
        optimizer.RegisterPass(spvtools::CreateInlineExhaustivePass());
        optimizer.RegisterPass(spvtools::CreateAggressiveDCEPass());
        optimizer.RegisterPass(spvtools::CreatePrivateToLocalPass());
        optimizer.RegisterPass(spvtools::CreateLocalSingleBlockLoadStoreElimPass());
        optimizer.RegisterPass(spvtools::CreateLocalSingleStoreElimPass());
        optimizer.RegisterPass(spvtools::CreateAggressiveDCEPass());
        optimizer.RegisterPass(spvtools::CreateScalarReplacementPass());
        optimizer.RegisterPass(spvtools::CreateLocalAccessChainConvertPass());
        optimizer.RegisterPass(spvtools::CreateLocalSingleBlockLoadStoreElimPass());
        optimizer.RegisterPass(spvtools::CreateLocalSingleStoreElimPass());
        optimizer.RegisterPass(spvtools::CreateAggressiveDCEPass());
        optimizer.RegisterPass(spvtools::CreateLocalMultiStoreElimPass());
        optimizer.RegisterPass(spvtools::CreateAggressiveDCEPass());
        optimizer.RegisterPass(spvtools::CreateCCPPass());
        optimizer.RegisterPass(spvtools::CreateAggressiveDCEPass());
        optimizer.RegisterPass(spvtools::CreateRedundancyEliminationPass());
        optimizer.RegisterPass(spvtools::CreateCombineAccessChainsPass());
        optimizer.RegisterPass(spvtools::CreateSimplificationPass());
        optimizer.RegisterPass(spvtools::CreateVectorDCEPass());
        optimizer.RegisterPass(spvtools::CreateDeadInsertElimPass());
        optimizer.RegisterPass(spvtools::CreateDeadBranchElimPass());
        optimizer.RegisterPass(spvtools::CreateSimplificationPass());
        optimizer.RegisterPass(spvtools::CreateIfConversionPass());
        optimizer.RegisterPass(spvtools::CreateCopyPropagateArraysPass());
        optimizer.RegisterPass(spvtools::CreateReduceLoadSizePass());
        optimizer.RegisterPass(spvtools::CreateAggressiveDCEPass());
        optimizer.RegisterPass(spvtools::CreateBlockMergePass());
        optimizer.RegisterPass(spvtools::CreateRedundancyEliminationPass());
        optimizer.RegisterPass(spvtools::CreateDeadBranchElimPass());
        optimizer.RegisterPass(spvtools::CreateBlockMergePass());
        optimizer.RegisterPass(spvtools::CreateSimplificationPass());
        break;
    }

    if (debugInfoType != SLANG_DEBUG_INFO_LEVEL_NONE)
    {
        optimizer.RegisterPass(spvtools::CreateRedundantLineInfoElimPass());
    }

    spvtools::OptimizerOptions spvOptOptions;
    spvOptOptions.set_run_validator(false); // Don't run the validator by default
    optimizer.Run(spirv.data(), spirv.size(), &spirv, spvOptOptions);
}


static glslang::EShTargetLanguageVersion _makeTargetLanguageVersion(int majorVersion, int minorVersion)
{
    return glslang::EShTargetLanguageVersion((uint32_t(majorVersion) << 16) | (uint32_t(minorVersion) << 8));
}

static glsl_SPIRVVersion _toSPIRVVersion(glslang::EShTargetLanguageVersion version)
{
    glsl_SPIRVVersion ver;
    ver.patch = 0;
    ver.major = uint8_t(uint32_t(version) >> 16);
    ver.minor = uint8_t(uint32_t(version) >> 8);
    return ver;
}

// For working out the targets based on SPIR-V target strings

namespace { // anonymous

struct SPRIVTargetInfo
{
    const char* name;
    spv_target_env targetEnv;
};

} // anonymous

static const SPRIVTargetInfo kSpirvTargetInfos[] =
{
    {"1.0",         SPV_ENV_UNIVERSAL_1_0},
    {"vk1.0",       SPV_ENV_VULKAN_1_0},
    {"1.1",         SPV_ENV_UNIVERSAL_1_1}, 
    {"cl2.1",       SPV_ENV_OPENCL_2_1},
    {"cl2.2",       SPV_ENV_OPENCL_2_2}, 
    {"gl4.0",       SPV_ENV_OPENGL_4_0},
    {"gl4.1",       SPV_ENV_OPENGL_4_1},
    {"gl4.2",       SPV_ENV_OPENGL_4_2},
    {"gl4.3",       SPV_ENV_OPENGL_4_3},
    {"gl4.5",       SPV_ENV_OPENGL_4_5},
    {"1.2",         SPV_ENV_UNIVERSAL_1_2}, 
    {"cl1.2",       SPV_ENV_OPENCL_1_2}, 
    {"cl_emb1.2",   SPV_ENV_OPENCL_EMBEDDED_1_2},
    {"cl2.0",       SPV_ENV_OPENCL_2_0},
    {"cl_emb2.0",   SPV_ENV_OPENCL_EMBEDDED_2_0},
    {"cl_emb2.1",   SPV_ENV_OPENCL_EMBEDDED_2_1},
    {"cl_emb2.2",   SPV_ENV_OPENCL_EMBEDDED_2_2},
    {"1.3",         SPV_ENV_UNIVERSAL_1_3},
    {"vk1.1",       SPV_ENV_VULKAN_1_1},
    {"web_gpu1.0",  SPV_ENV_WEBGPU_0},
    {"1.4",         SPV_ENV_UNIVERSAL_1_4},
    {"vk1.1_spirv1.4", SPV_ENV_VULKAN_1_1_SPIRV_1_4},
    {"1.5",         SPV_ENV_UNIVERSAL_1_5},             
};

static int _findTargetIndex(const char* name)
{
    const int count = int(sizeof(kSpirvTargetInfos) / sizeof(kSpirvTargetInfos[0]));
    for (int i = 0; i < count; ++i)
    {
        const SPRIVTargetInfo& info = kSpirvTargetInfos[i];

        if (::strcmp(info.name, name) == 0)
        {
            return i;
        }
    }
    return -1;
}

static spv_target_env _getUniversalTargetEnv(glslang::EShTargetLanguageVersion inVersion)
{
    glsl_SPIRVVersion spirvVersion = _toSPIRVVersion(inVersion);
    uint32_t ver = (uint32_t(spirvVersion.major) << 8) | spirvVersion.minor;

    switch (ver)
    {
        case 0x100:     return SPV_ENV_UNIVERSAL_1_0;
        case 0x101:     return SPV_ENV_UNIVERSAL_1_1;
        case 0x102:     return SPV_ENV_UNIVERSAL_1_2;
        case 0x103:     return SPV_ENV_UNIVERSAL_1_3;
        case 0x104:     return SPV_ENV_UNIVERSAL_1_4;
        case 0x105:     return SPV_ENV_UNIVERSAL_1_5;
        default:
        {            
            if (ver > 0x105)
            {
                // This is the highest we known for now..., so try that
                return SPV_ENV_UNIVERSAL_1_5;
            }
            break;
        }
    }
    // Just use the default...
    return SPV_ENV_UNIVERSAL_1_2;
}

static int glslang_compileGLSLToSPIRV(const glslang_CompileRequest_1_1& request)
{
    // Check that the encoding matches
    assert(glslang::EShTargetSpv_1_4 == _makeTargetLanguageVersion(1, 4));

    EShLanguage glslangStage;
    switch( request.slangStage )
    {
#define CASE(SP, GL) case SLANG_STAGE_##SP: glslangStage = EShLang##GL; break
    CASE(VERTEX,    Vertex);
    CASE(FRAGMENT,  Fragment);
    CASE(GEOMETRY,  Geometry);
    CASE(HULL,      TessControl);
    CASE(DOMAIN,    TessEvaluation);
    CASE(COMPUTE,   Compute);

    CASE(RAY_GENERATION,    RayGenNV);
    CASE(INTERSECTION,      IntersectNV);
    CASE(ANY_HIT,           AnyHitNV);
    CASE(CLOSEST_HIT,       ClosestHitNV);
    CASE(MISS,              MissNV);
    CASE(CALLABLE,          CallableNV);

#undef CASE

    default:
        dumpDiagnostics(request, "internal error: stage unsupported by glslang\n");
        return 1;
    }


    spv_target_env targetEnv = SPV_ENV_UNIVERSAL_1_2;
    glslang::EShTargetLanguageVersion targetLanguage = glslang::EShTargetLanguageVersion(0);

    int spirvTargetIndex = -1;
    if (request.spirvTargetName)
    {
        spirvTargetIndex = _findTargetIndex(request.spirvTargetName);
        if (spirvTargetIndex < 0)
        {
            dumpDiagnostics(request, "warning: unknown SPIR-V version\n");
        }
        else
        {
            targetEnv = kSpirvTargetInfos[spirvTargetIndex].targetEnv;
        }
    }

    // If a version is specified, and no target language is specified, set to universal version of that SPIR-V version
    if (request.spirvVersion.major != 0 && targetLanguage == glslang::EShTargetLanguageVersion(0))
    {
        targetLanguage = _makeTargetLanguageVersion(request.spirvVersion.major, request.spirvVersion.minor);
    }

    // If we don't have a target, but do have a language, use that to determine a universal target
    if (spirvTargetIndex < 0 && targetLanguage != glslang::EShTargetLanguageVersion(0))
    {
        // We can just use the appropriate universal based on the target language
        targetEnv = _getUniversalTargetEnv(targetLanguage);
    }
    
    // TODO: compute glslang stage to use

    glslang::TShader* shader = new glslang::TShader(glslangStage);
    auto shaderPtr = std::unique_ptr<glslang::TShader>(shader);

    // Only set the target language if one is determined
    if (targetLanguage != glslang::EShTargetLanguageVersion(0))
    {
        shader->setEnvTarget(glslang::EShTargetSpv, targetLanguage);
    }

    glslang::TProgram* program = new glslang::TProgram();
    auto programPtr = std::unique_ptr<glslang::TProgram>(program);

    char const* sourceText = (char const*)request.inputBegin;
    char const* sourceTextEnd = (char const*)request.inputEnd;

    int sourceTextLength = (int)(sourceTextEnd - sourceText);

    shader->setPreamble("#extension GL_GOOGLE_cpp_style_line_directive : require\n");

    shader->setStringsWithLengthsAndNames(
        &sourceText,
        &sourceTextLength,
        &request.sourcePath,
        1);

    EShMessages messages = EShMessages(EShMsgSpvRules | EShMsgVulkanRules);
     
    if( !shader->parse(&gResources, 110, false, messages) )
    {
        dumpDiagnostics(request, shader->getInfoLog());
        return 1;
    }

    program->addShader(shader);

    if( !program->link(messages) )
    {
        dumpDiagnostics(request, program->getInfoLog());
        return 1;
    }

    if( !program->mapIO() )
    {
        dumpDiagnostics(request, program->getInfoLog());
        return 1;
    }

    for(int stage = 0; stage < EShLangCount; ++stage)
    {
        auto stageIntermediate = program->getIntermediate((EShLanguage)stage);
        if(!stageIntermediate)
            continue;

        std::vector<unsigned int> spirv;
        std::string warningsErrors;
        spv::SpvBuildLogger logger;
        glslang::GlslangToSpv(*stageIntermediate, spirv, &logger);

        if (request.optimizationLevel != SLANG_OPTIMIZATION_LEVEL_NONE)
        {
            glslang_optimizeSPIRV(spirv, targetEnv, request.optimizationLevel, request.debugInfoType);
        }

        dumpDiagnostics(request, logger.getAllMessages());

        dump(spirv.data(), spirv.size() * sizeof(unsigned int), request.outputFunc, request.outputUserData, stdout);
    }

    return 0;
}

static int glslang_dissassembleSPIRV(const glslang_CompileRequest_1_1& request)
{
    typedef unsigned int SPIRVWord;

    SPIRVWord const* spirvBegin = (SPIRVWord const*)request.inputBegin;
    SPIRVWord const* spirvEnd   = (SPIRVWord const*)request.inputEnd;

    std::vector<SPIRVWord> spirv(spirvBegin, spirvEnd);

    std::stringstream spirvAsmStream;
    spv::Disassemble(spirvAsmStream, spirv);
    std::string result = spirvAsmStream.str();
    dump(result.c_str(), result.length(), request.outputFunc, request.outputUserData, stdout);

    return 0;
}

// We need a per process initialization
class ProcessInitializer
{
public:
    ProcessInitializer()
    {
        m_isInitialized = false;
    }

    bool init()
    {
        std::lock_guard<std::mutex> guard(m_mutex);
        if (!m_isInitialized)
        {
            if (!glslang::InitializeProcess())
            {
                return false;
            }
            m_isInitialized = true;
        }
        return true;
    }

    ~ProcessInitializer()
    {
        // We *assume* will only be called once dll is detatched and that will be on a single thread
        if (m_isInitialized)
        {
            glslang::FinalizeProcess();
        }
    }

    std::mutex m_mutex;
    bool m_isInitialized = false;
};

static int _compile(const glslang_CompileRequest_1_1& request)
{
    int result = 0;
    switch (request.action)
    {
        default:
            result = 1;
            break;

        case GLSLANG_ACTION_COMPILE_GLSL_TO_SPIRV:
            result = glslang_compileGLSLToSPIRV(request);
            break;

        case GLSLANG_ACTION_DISSASSEMBLE_SPIRV:
            result = glslang_dissassembleSPIRV(request);
            break;
    }

    return result;
}

extern "C"
#ifdef _MSC_VER
_declspec(dllexport)
#else
__attribute__((__visibility__("default")))
#endif
int glslang_compile_1_1(glslang_CompileRequest_1_1* inRequest)
{
    static ProcessInitializer g_processInitializer;
    if (!g_processInitializer.init())
    {
        // Failed
        return 1;
    }
    if (!glslang::InitThread())
    {
        // Failed
        return 1;
    }

    // If it's the right size just use it
    if (inRequest->sizeInBytes == sizeof(glslang_CompileRequest_1_1))
    {
        return _compile(*inRequest);
    }
    else
    {
        // NOTE! It could be larger, but here we'll assume thats ok, and copy and use.

        // Try to ensure some binary compatibility, by using sizeInBytes member, and copying

        glslang_CompileRequest_1_1 request;
        
        // Copy into request
        const size_t copySize = (inRequest->sizeInBytes > sizeof(request)) ? sizeof(request) : inRequest->sizeInBytes;
        ::memcpy(&request, inRequest, copySize);
        // Zero any remaining members
        memset(((uint8_t*)&request) + copySize, 0, sizeof(request) - copySize);

        return _compile(request);
    }
}

extern "C"
#ifdef _MSC_VER
_declspec(dllexport)
#else
__attribute__((__visibility__("default")))
#endif
int glslang_compile(glslang_CompileRequest_1_0* inRequest)
{
    glslang_CompileRequest_1_1 request;
    memset(&request, 0, sizeof(request));
    request.sizeInBytes = sizeof(request);
    request.set(*inRequest);
    return glslang_compile_1_1(&request);
}

#if 0
static std::mutex g_globalMutex;

namespace glslang {

void InitGlobalLock()
{
    
}

void GetGlobalLock()
{
    g_globalMutex.lock();
}

void ReleaseGlobalLock()
{
    g_globalMutex.unlock();
}

} // namespace glslang
#endif
