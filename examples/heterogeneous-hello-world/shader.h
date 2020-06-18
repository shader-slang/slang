#define SLANG_PRELUDE_NAMESPACE CPPPrelude
#include "prelude/slang-cpp-prelude.h"

//using namespace CPPPrelude;

#line 13 "shader.slang"
struct UniformState
{

#line 4
    CPPPrelude::RWStructuredBuffer<float> ioBuffer;



};

extern"C" void computeMain(CPPPrelude::ComputeVaryingInput* varyingInput, CPPPrelude::UniformEntryPointParams* params, UniformState* uniformState);
