// main.cpp

#include <stdio.h>

// This file implements an extremely simple example of loading and
// executing a Slang shader program on the CPU. 
//
// More information about generation C++ or CPU code can be found in docs/cpu-target.md
//
// NOTE! This test will only run on a system correctly where slang can find a suitable
// C++ compiler - such as clang/gcc/visual studio
//
// The comments in the file will attempt to explain concepts as
// they are introduced.
//
// Of course, in order to use the Slang API, we need to include
// its header. We have set up the build options for this project
// so that it is as simple as:
#include <slang.h>

// Allows use of ComPtr - which we can use to scope any 'com-like' pointers easily
#include <slang-com-ptr.h>
// Provides macros for handling SlangResult values easily 
#include <slang-com-helper.h>

// This includes a useful small function for setting up the prelude (described more further below).
#include "../../source/core/slang-test-tool-util.h"

// Slang namespace is used for elements support code (like core) which we use here
// for ComPtr<> and TestToolUtil
using namespace Slang;

// Slang source is converted into C++ code which is compiled by a backend compiler.
// That process uses a 'prelude' which defines types and functions that are needed
// for everything else to work.
// 
// We include the prelude here, so we can directly use the types as were used by the
// compiled code. It is not necessary to include the prelude, as long as memory is
// laid out in the manner that the generated slang code expects. 
#define SLANG_PRELUDE_NAMESPACE CPPPrelude
#include "../../prelude/slang-cpp-types.h"
#include "shader.h"

static SlangResult _innerMain(int argc, char** argv)
{
    // the uniformState will be passed as a pointer to the CPU code 
    UniformState uniformState;

    // The contents of the buffer are modified, so we'll copy it
    const float startBufferContents[] = { 2.0f, -10.0f, -3.0f, 5.0f };
    float bufferContents[SLANG_COUNT_OF(startBufferContents)];
    memcpy(bufferContents, startBufferContents, sizeof(startBufferContents));

    // Set up the ioBuffer such that it uses bufferContents. It is important to set the .count
    // such that bounds checking can be performed in the kernel.  
    uniformState.ioBuffer.data = bufferContents;
    uniformState.ioBuffer.count = SLANG_COUNT_OF(bufferContents);

    // In shader.slang, then entry point is attributed with `[numthreads(4, 1, 1)]` meaning each group
    // consists of 4 'thread' in x. Our input buffer is 4 wide, and we index the input array via `SV_DispatchThreadID`
    // so we only need to run a single group to execute over all of the 4 elements here.
    // The group range from { 0, 0, 0 } -> { 1, 1, 1 } means it will execute over the single group { 0, 0, 0 }.

    const CPPPrelude::uint3 startGroupID = { 0, 0, 0};
    const CPPPrelude::uint3 endGroupID = { 1, 1, 1 };

    CPPPrelude::ComputeVaryingInput varyingInput;
    varyingInput.startGroupID = startGroupID;
    varyingInput.endGroupID = endGroupID;

    // We don't have any entry point parameters so that's passed as NULL
    // We need to cast our definition of the uniform state to the undefined CPPPrelude::UniformState as
    // that type is just a name to indicate what kind of thing needs to be passed in.
    computeMain(&varyingInput, NULL, (UniformState*)&uniformState);

    // bufferContents holds the output

    // Print out the values before the computation
    printf("Before:\n");
    for (float v : startBufferContents)
    {
        printf("%f, ", v);
    }
    printf("\n");

    // Print out the values the the kernel produced
    printf("After: \n");
    for (float v : bufferContents)
    {
        printf("%f, ", v);
    }
    printf("\n");

    return SLANG_OK;
}

int main(int argc, char** argv)
{
    return SLANG_SUCCEEDED(_innerMain(argc, argv)) ? 0 : -1;
}
