// slang-glslang.cpp
#include "slang-glslang.h"


#include "StandAlone/ResourceLimits.h"
#include "StandAlone/Worklist.h"
#include "glslang/Include/ShHandle.h"
#include "glslang/Include/revision.h"
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

static TBuiltInResource gResources =
{
    UNLIMITED, UNLIMITED, UNLIMITED, UNLIMITED, UNLIMITED, UNLIMITED, UNLIMITED, UNLIMITED, UNLIMITED, UNLIMITED, 
    UNLIMITED, UNLIMITED, UNLIMITED, UNLIMITED, UNLIMITED, UNLIMITED, UNLIMITED,-UNLIMITED, UNLIMITED, UNLIMITED, 
    UNLIMITED, UNLIMITED, UNLIMITED, UNLIMITED, UNLIMITED, UNLIMITED, UNLIMITED, UNLIMITED, UNLIMITED, UNLIMITED, 
    UNLIMITED, UNLIMITED, UNLIMITED, UNLIMITED, UNLIMITED, UNLIMITED, UNLIMITED, UNLIMITED, UNLIMITED, UNLIMITED, 
    UNLIMITED, UNLIMITED, UNLIMITED, UNLIMITED, UNLIMITED, UNLIMITED, UNLIMITED, UNLIMITED, UNLIMITED, UNLIMITED, 
    UNLIMITED, UNLIMITED, UNLIMITED, UNLIMITED, UNLIMITED, UNLIMITED, UNLIMITED, UNLIMITED, UNLIMITED, UNLIMITED, 
    UNLIMITED, UNLIMITED, UNLIMITED, UNLIMITED, UNLIMITED, UNLIMITED, UNLIMITED, UNLIMITED, UNLIMITED, UNLIMITED, 
    UNLIMITED, UNLIMITED, UNLIMITED, UNLIMITED, UNLIMITED, UNLIMITED, UNLIMITED, UNLIMITED, UNLIMITED, UNLIMITED, 
    UNLIMITED, UNLIMITED, UNLIMITED, UNLIMITED, UNLIMITED, UNLIMITED, UNLIMITED, UNLIMITED, UNLIMITED, UNLIMITED, 
    UNLIMITED, UNLIMITED,

    { true, true, true, true, true, true, true, true, true, }
};

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
    glslang_CompileRequest* request,
    std::string const&      log)
{
    dump(log.c_str(), log.length(), request->diagnosticFunc, request->diagnosticUserData, stderr);
}

// Apply the SPIRV-Tools optimizer to generated SPIR-V based on the desired optimization level
// TODO: add flag for optimizing SPIR-V size as well
static void glslang_optimizeSPIRV(std::vector<unsigned int>& spirv, unsigned optimizationLevel, unsigned debugInfoType)
{
    spv_target_env target_env = SPV_ENV_UNIVERSAL_1_2;

    spvtools::Optimizer optimizer(target_env);
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

static int glslang_compileGLSLToSPIRV(glslang_CompileRequest* request)
{
    EShLanguage glslangStage;
    switch( request->slangStage )
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

    // TODO: compute glslang stage to use

    glslang::TShader* shader = new glslang::TShader(glslangStage);
    auto shaderPtr = std::unique_ptr<glslang::TShader>(shader);

    glslang::TProgram* program = new glslang::TProgram();
    auto programPtr = std::unique_ptr<glslang::TProgram>(program);

    char const* sourceText = (char const*)request->inputBegin;
    char const* sourceTextEnd = (char const*)request->inputEnd;

    int sourceTextLength = (int)(sourceTextEnd - sourceText);

    shader->setPreamble("#extension GL_GOOGLE_cpp_style_line_directive : require\n");

    shader->setStringsWithLengthsAndNames(
        &sourceText,
        &sourceTextLength,
        &request->sourcePath,
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

        if (request->optimizationLevel != SLANG_OPTIMIZATION_LEVEL_NONE)
        {
            glslang_optimizeSPIRV(spirv, request->optimizationLevel, request->debugInfoType);
        }

        dumpDiagnostics(request, logger.getAllMessages());

        dump(spirv.data(), spirv.size() * sizeof(unsigned int), request->outputFunc, request->outputUserData, stdout);
    }

    return 0;
}

static int glslang_dissassembleSPIRV(glslang_CompileRequest* request)
{
    typedef unsigned int SPIRVWord;

    SPIRVWord const* spirvBegin = (SPIRVWord const*)request->inputBegin;
    SPIRVWord const* spirvEnd   = (SPIRVWord const*)request->inputEnd;

    std::vector<SPIRVWord> spirv(spirvBegin, spirvEnd);

    std::stringstream spirvAsmStream;
    spv::Disassemble(spirvAsmStream, spirv);
    std::string result = spirvAsmStream.str();
    dump(result.c_str(), result.length(), request->outputFunc, request->outputUserData, stdout);

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

extern "C"
#ifdef _MSC_VER
_declspec(dllexport)
#else
__attribute__((__visibility__("default")))
#endif
int glslang_compile(glslang_CompileRequest* request)
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

    int result = 0;
    switch(request->action)
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
