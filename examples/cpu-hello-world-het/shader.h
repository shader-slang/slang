#define SLANG_PRELUDE_NAMESPACE CPPPrelude
#include "prelude/slang-cpp-prelude.h"

using namespace CPPPrelude;

#line 13 "shader.slang"
struct UniformState
{

#line 4
    RWStructuredBuffer<float> ioBuffer;



};

extern"C" void computeMain(ComputeVaryingInput* varyingInput, UniformEntryPointParams* params, UniformState* uniformState);
