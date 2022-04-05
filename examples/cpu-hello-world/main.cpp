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

struct UniformState;

static SlangResult _innerMain(int argc, char** argv)
{
    // First, we need to create a "session" for interacting with the Slang
    // compiler. This scopes all of our application's interactions
    // with the Slang library. At the moment, creating a session causes
    // Slang to load and validate its standard library, so this is a
    // somewhat heavy-weight operation. When possible, an application
    // should try to re-use the same session across multiple compiles.
    //
    // NOTE that we use attach instead of setting via assignment, as assignment will increase
    // the refcount. spCreateSession returns a IGlobalSession with a refcount of 1.
    ComPtr<slang::IGlobalSession> slangSession;
    slangSession.attach(spCreateSession(NULL));

    // As touched on earlier, in order to generate the final executable code,
    // the slang code is converted into C++, and that C++ needs a 'prelude' which
    // is just definitions that the generated code needed to work correctly.
    // There is a simple default definition of a prelude provided in the prelude
    // directory called 'slang-cpp-prelude.h'.
    // 
    // We need to tell slang either the contents of the prelude, or suitable include/s
    // that will work. The actual API call to set the prelude is `setPrelude`
    // and this just sets for a specific language a bit of text placed before generated code.
    //
    // Most downstream C++ compilers work on files. In that case slang may generate temporary
    // files that contain the generated code. Typically the generated files  will not be in the
    // same directory as the original source so handling includes becomes awkward. The mechanism used here
    // is for the prelude code to be an *absolute* path to the 'slang-cpp-prelude.h' - which means
    // this will work wherever the generated code is, and allows accessing other files via relative paths.
    //
    // Look at the source to TestToolUtil::setSessionDefaultPreludeFromExePath to see what's involed. 
    TestToolUtil::setSessionDefaultPreludeFromExePath(argv[0], slangSession);

    // A compile request represents a single invocation of the compiler,
    // to process some inputs and produce outputs (or errors).
    //
    SlangCompileRequest* slangRequest = spCreateCompileRequest(slangSession);

    // We would like to request a CPU code that can be executed directly on the host -
    // which is the 'SLANG_HOST_CALLABLE' target. 
    // If we wanted a just a shared library/dll, we could have used SLANG_SHARED_LIBRARY.
    int targetIndex = spAddCodeGenTarget(slangRequest, SLANG_SHADER_HOST_CALLABLE);

    // Set the target flag to indicate that we want to compile all the entrypoints in the
    // slang shader file into a library.
    spSetTargetFlags(slangRequest, targetIndex, SLANG_TARGET_FLAG_GENERATE_WHOLE_PROGRAM);

    // A compile request can include one or more "translation units," which more or
    // less amount to individual source files (think `.c` files, not the `.h` files they
    // might include).
    //
    // For this example, our code will all be in the Slang language. The user may
    // also specify HLSL input here, but that currently doesn't affect the compiler's
    // behavior much.
    //
    int translationUnitIndex = spAddTranslationUnit(slangRequest, SLANG_SOURCE_LANGUAGE_SLANG, nullptr);

    // We will load source code for our translation unit from the file `shader.slang`.
    // There are also variations of this API for adding source code from application-provided buffers.
    //
    spAddTranslationUnitSourceFile(slangRequest, translationUnitIndex, "shader.slang");

    // Once all of the input options for the compiler have been specified,
    // we can invoke `spCompile` to run the compiler and see if any errors
    // were detected.
    //
    const SlangResult compileRes = spCompile(slangRequest);

    // Even if there were no errors that forced compilation to fail, the
    // compiler may have produced "diagnostic" output such as warnings.
    // We will go ahead and print that output here.
    //
    if(auto diagnostics = spGetDiagnosticOutput(slangRequest))
    {
        printf("%s", diagnostics);
    }

    // If compilation failed, there is no point in continuing any further.
    if(SLANG_FAILED(compileRes))
    {
        spDestroyCompileRequest(slangRequest);
        return compileRes;
    }

    // Get the 'shared library' (note that this doesn't necessarily have to be implemented as a shared library
    // it's just an interface to executable code).
    ComPtr<ISlangSharedLibrary> sharedLibrary;
    SLANG_RETURN_ON_FAIL(spGetTargetHostCallable(slangRequest, 0, sharedLibrary.writeRef()));

    // Once we have the sharedLibrary, we no longer need the request
    // unless we want to use reflection, to for example workout how 'UniformState' and 'UniformEntryPointParams' are laid out
    // at runtime. We don't do that here - as we hard code the structures. 
    spDestroyCompileRequest(slangRequest);

    // Get the function we are going to execute 
    const char entryPointName[] = "computeMain";
    CPPPrelude::ComputeFunc func = (CPPPrelude::ComputeFunc)sharedLibrary->findFuncByName(entryPointName);
    if (!func)
    {
        return SLANG_FAIL;
    }

    // Define the uniform state structure that is *specific* to our shader defined in shader.slang
    // That the layout of the structure can be determined through reflection, or can be inferred from
    // the original slang source. Look at the documentation in docs/cpu-target.md which describes
    // how different resources map.
    // The order of the resources is in the order that they are defined in the source. 
    struct UniformState
    {
        CPPPrelude::RWStructuredBuffer<float> ioBuffer;
    };

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
    func(&varyingInput, NULL, &uniformState);

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
