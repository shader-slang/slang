#include "slang-tint.h"
#include "tint/tint.h"

#if defined(_MSC_VER)
#define SLANG_SHARED_LIB_EXPORT __declspec(dllexport)
#else
#define SLANG_SHARED_LIB_EXPORT __attribute__((__visibility__("default")))
#endif

static bool compile(std::vector<uint32_t> const& spirvCode, std::string& output)
{
    using namespace tint;

    // TODO:
    // Once crbug.com/tint/1907 is resolved, we can call the function currently
    // called spirv::reader::ReadIR to transform via Tint IR, which will be the preferred path,
    // but this function is still WIP at the moment.
    spirv::reader::Options spirvReadOptions {};
    spirvReadOptions.allowed_features = wgsl::AllowedFeatures::Everything();
    Program program {spirv::reader::Read(spirvCode, spirvReadOptions)};
    if(program.Diagnostics().ContainsErrors())
    {
        std::string failure {program.Diagnostics().Str()};
        printf("slang-tint: SPIR-V to Tint program: %s\n", failure.c_str());
        return false;
    }

    wgsl::writer::Options wgslWriteOptions {};
    Result<wgsl::writer::Output> wgslOutputResult {
        wgsl::writer::Generate(program, wgslWriteOptions)
    };
    if(wgslOutputResult != Success)
    {
        std::string failure {wgslOutputResult.Failure().reason.Str()};
        printf("slang-tint: Tint program to WGSL: %s\n", failure.c_str());
        return false;
    }
    wgsl::writer::Output & wgslOutput {wgslOutputResult.Get()};

    output = std::move(wgslOutput.wgsl);
    return true;
}

extern "C"
SLANG_SHARED_LIB_EXPORT
int tint_compile(tint_CompileRequest* request)
{
    if(request == nullptr)
        return 1;

    std::vector<uint32_t> spirvCode(request->spirvCode, request->spirvCode + request->spirvCodeLength);
    std::string wgslCode;
    if(!compile(spirvCode, wgslCode))
        return 1;

    size_t const resultBufferSize {wgslCode.size() + size_t{1}};
    char *const resultBuffer {static_cast<char*>(::malloc(resultBufferSize))};
    if(resultBuffer == nullptr)
        return 1;
    ::memcpy(resultBuffer, wgslCode.c_str(), wgslCode.size());
    resultBuffer[wgslCode.size()] = '\0';

    request->resultBuffer = resultBuffer;
    request->resultBufferSize = resultBufferSize;
    return 0;
}
