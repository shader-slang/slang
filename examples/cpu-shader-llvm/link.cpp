// This sample demonstrates how to run a Slang shader that has been built into
// object code and linked into the executable.
#include <cstdint>
#include <cstdio>
#include <vector>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
using namespace std;

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

// The name of the Slang compute shader's entry point was
// renderMandelbrotFractal; _Group is appended to indicate that this function
// runs a full workgroup instead of a single work item / "thread".
extern "C" void renderMandelbrotFractal_Group(
    uint32_t groupID[3],
    EntryPointParams* entryPointParams,
    GlobalParams* globalParams);

int main(int argc, char** argv)
{
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
    //#pragma omp parallel for collapse(2)
    // The thread group size is 8x8 here.
    for (uint32_t x = 0; x < imageWidth/8; ++x)
    for (uint32_t y = 0; y < imageHeight/8; ++y)
    {
        uint32_t groupID[3] = {x, y, 0};
        renderMandelbrotFractal_Group(groupID, &entryPointParams, &globals);
    }

    const char* filename = "cpu-shader-llvm.png";
    if (!stbi_write_png(filename, imageWidth, imageHeight, 4, imageData.data(), 4 * imageWidth))
    {
        fprintf(stderr, "Failed to write %s!", filename);
        return 1;
    }

    return 0;
}
