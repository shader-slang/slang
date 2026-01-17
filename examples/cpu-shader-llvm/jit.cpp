// This sample demonstrates how to compile a Slang shader during runtime and
// run it on the CPU via JIT compilation.
#include "examples/example-base/example-base.h"
#include "slang.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

// This is used to find the path to the shader source. You do not need this in
// your own code.
static const ExampleResources resourceBase("cpu-shader-llvm");

// All uniform parameters given to the entry point are collected and stored
// here.
struct EntryPointParams
{
    int32_t maxIters;
};

// "MyConstantParams" is just a struct on the Slang side, see `shader.slang`.
struct MyConstantParams
{
    int32_t imageSize[2];
};

// All global parameters like global uniforms and buffers are collected into
// this struct.
struct GlobalParams
{
    // RWStructuredBuffers are encoded as a data pointer and a size.
    uint8_t* outputColorsData;
    size_t outputColorsCount;

    // ConstantBuffers are only pointers to the data.
    MyConstantParams* constants;
};

// Compute shader entry point always have a similar signature. The
// `EntryPointParams` and `GlobalParams` arguments will exist even if they are
// empty; you can treat them as `void*` and just pass `nullptr` in that case.
typedef void (*ComputeShaderFunc)(
    uint32_t groupID[3],
    EntryPointParams* entryPointParams,
    GlobalParams* globalParams);

int exampleMain(int argc, char** argv)
{
    // Initialize the global session as usual.
    Slang::ComPtr<slang::IGlobalSession> globalSession;
    SlangGlobalSessionDesc desc = {};
    if (slang::createGlobalSession(&desc, globalSession.writeRef()) != SLANG_OK)
    {
        fprintf(stderr, "Failed to open global session!\n");
        return -1;
    }

    // This option is currently needed to use LLVM directly instead of emitting
    // C++ and invoking Clang.
    slang::CompilerOptionEntry options[] = {
        {slang::CompilerOptionName::EmitCPUMethod, {{}, SLANG_EMIT_CPU_VIA_LLVM}}};

    slang::TargetDesc target = {};
    // For JIT execution, the target must be SLANG_SHADER_HOST_CALLABLE or
    // SLANG_HOST_HOST_CALLABLE; you'll want the former for running compute
    // shaders. The latter is for calling arbitrary Slang functions that aren't
    // shader entry points.
    target.format = SLANG_SHADER_HOST_CALLABLE;
    target.compilerOptionEntries = options;
    target.compilerOptionEntryCount = std::size(options);

    slang::SessionDesc sessionDesc;
    sessionDesc.targets = &target;
    sessionDesc.targetCount = 1;

    Slang::ComPtr<slang::ISession> session;
    if (globalSession->createSession(sessionDesc, session.writeRef()))
    {
        fprintf(stderr, "Failed to open JIT session!\n");
        return -1;
    }

    ComPtr<slang::IBlob> diagnosticBlob;
    Slang::String path = resourceBase.resolveResource("shader.slang");
    slang::IModule* module = session->loadModule(path.getBuffer(), diagnosticBlob.writeRef());
    if (!module)
    {
        diagnoseIfNeeded(diagnosticBlob);
        return -1;
    }

    ComPtr<slang::IEntryPoint> entryPoint;
    module->findEntryPointByName("renderMandelbrotFractal", entryPoint.writeRef());

    slang::IComponentType* components[] = {module, entryPoint};
    Slang::ComPtr<slang::IComponentType> program;
    if (session->createCompositeComponentType(
            components,
            std::size(components),
            program.writeRef(),
            diagnosticBlob.writeRef()) != SLANG_OK)
    {
        diagnoseIfNeeded(diagnosticBlob);
        return -1;
    }

    // Now, we can finally fetch a handle that allows us to access the compiled
    // program's functions.
    ComPtr<ISlangSharedLibrary> sharedLibrary;
    if (program->getEntryPointHostCallable(
            0,
            0,
            sharedLibrary.writeRef(),
            diagnosticBlob.writeRef()) != SLANG_OK)
    {
        diagnoseIfNeeded(diagnosticBlob);
        return -1;
    }

    // Grab the work group / "thread group" function. This runs a whole group at
    // once.
    auto renderMandelbrotFractal_Group =
        (ComputeShaderFunc)sharedLibrary->findFuncByName("renderMandelbrotFractal_Group");
    if (!renderMandelbrotFractal_Group)
    {
        fprintf(stderr, "Failed to find entry point!\n");
        return -1;
    }

    // Let's render an image!
    int imageWidth = 512;
    int imageHeight = 512;
    std::vector<uint8_t> imageData(imageWidth * imageHeight * 4);

    MyConstantParams constants;
    constants.imageSize[0] = imageWidth;
    constants.imageSize[1] = imageHeight;

    GlobalParams globals;
    globals.outputColorsData = imageData.data();
    // divide by 4 because the shader side used uint8_t4.
    globals.outputColorsCount = imageData.size() / 4;
    globals.constants = &constants;

    EntryPointParams entryPointParams;
    entryPointParams.maxIters = 256;

    // We could safely multithread the workgroup calls, e.g. with OpenMP:
    // #pragma omp parallel for collapse(2)
    // The thread group size is 8x8 here.
    for (uint32_t x = 0; x < imageWidth / 8; ++x)
    {
        for (uint32_t y = 0; y < imageHeight / 8; ++y)
        {
            uint32_t groupID[3] = {x, y, 0};
            renderMandelbrotFractal_Group(groupID, &entryPointParams, &globals);
        }
    }

    const char* filename = "cpu-shader-llvm.png";
    if (!stbi_write_png(filename, imageWidth, imageHeight, 4, imageData.data(), 4 * imageWidth))
    {
        log("Failed to write %s!\n", filename);
        return 1;
    }

    return 0;
}
