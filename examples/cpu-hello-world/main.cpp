// main.cpp

#include <stdio.h>

// This file implements an extremely simple example of loading and
// executing a Slang shader program on the CPU. 
//
// The comments in the file will attempt to explain concepts as
// they are introduced.
//
// Of course, in order to use the Slang API, we need to include
// its header. We have set up the build options for this project
// so that it is as simple as:
//
#include <slang.h>

#include <slang-com-ptr.h>
#include <slang-com-helper.h>

#include "../../source/core/slang-test-tool-util.h"
#include "../../source/core/slang-list.h"

// Slang namespace is used for elements support code (like core)
using namespace Slang;

#define SLANG_PRELUDE_NAMESPACE CPPPrelude
#include "../../prelude/slang-cpp-types.h"

static SlangResult _innerMain(int argc, char** argv)
{
    // First, we need to create a "session" for interacting with the Slang
    // compiler. This scopes all of our application's interactions
    // with the Slang library. At the moment, creating a session causes
    // Slang to load and validate its standard library, so this is a
    // somewhat heavy-weight operation. When possible, an application
    // should try to re-use the same session across multiple compiles.
    //
    ComPtr<slang::IGlobalSession> slangSession(spCreateSession(NULL));

    TestToolUtil::setSessionDefaultPrelude(argv[0], slangSession);

    // A compile request represents a single invocation of the compiler,
    // to process some inputs and produce outputs (or errors).
    //
    SlangCompileRequest* slangRequest = spCreateCompileRequest(slangSession);

    // We would like to request a CPU code that can be executed directly on the host 'HOST_CALLABLE' 
    int targetIndex = spAddCodeGenTarget(slangRequest, SLANG_HOST_CALLABLE);

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
    
    // Next we will specify the entry points we'd like to compile.
    // It is often convenient to put more than one entry point in the same file,
    // and the Slang API makes it convenient to use a single run of the compiler
    // to compile all entry points.
    //
    // For each entry point, we need to specify the name of a function, the
    // translation unit in which that function can be found, and the stage
    // that we need to compile for (e.g., vertex, fragment, geometry, ...).
    
    const char entryPointName[] = "computeMain";
    int computeIndex = spAddEntryPoint(slangRequest, translationUnitIndex, entryPointName,  SLANG_STAGE_COMPUTE);

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
    SLANG_RETURN_ON_FAIL(spGetEntryPointHostCallable(slangRequest, 0, 0, sharedLibrary.writeRef()));

    // Once we have the sharedLibrary, we no longer need the request
    // unless we want to use reflection, to for example workout how 'UniformState' and 'UniformEntryPointParams' are laid out
    // at runtime. We don't do that here - as we hard code the structures. 
    spDestroyCompileRequest(slangRequest);

    // Get the function we are going to execute 
    CPPPrelude::ComputeFunc func = (CPPPrelude::ComputeFunc)sharedLibrary->findFuncByName(entryPointName);
    if (!func)
    {
        spDestroyCompileRequest(slangRequest);
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

    UniformState uniformState;
    List<float> buffer;

    const float startBufferContents[] = { 2.0f, -10.0f, -3.0f, 5.0f };
    buffer.addRange(startBufferContents, SLANG_COUNT_OF(startBufferContents));

    uniformState.ioBuffer.data = buffer.getBuffer();
    uniformState.ioBuffer.count = buffer.getCount();

    const CPPPrelude::uint3 startGroupID = { 0, 0, 0};
    const CPPPrelude::uint3 endGroupID = { 1, 1, 1 };

    CPPPrelude::ComputeVaryingInput varyingInput;

    // In shader.slang, then entry point is attributed with `[numthreads(4, 1, 1)]` meaning each group
    // consists of 4 'thread' in x. Our input buffer is 4 wide, and we index the input array via `SV_DispatchThreadID`
    // so we only need to run a single group to execute over all of the 4 elements here.
    // The group range from { 0, 0, 0 } -> { 1, 1, 1 } means it will execute over the single group { 0, 0, 0 }.
    varyingInput.startGroupID = startGroupID;
    varyingInput.endGroupID = endGroupID;

    // We don't have any entry point parameters so that's passed as NULL
    // We need to cast our definition of the uniform state to the undefined CPPPrelude::UniformState as
    // that type is just a name to indicate what kind of thing needs to be passed in.
    func(&varyingInput, NULL, (CPPPrelude::UniformState*)&uniformState);

    // buffer holds the output (as ioBuffer in uniformState is a RWStructuredBuffer).

    // Print out the values before the computation
    printf("Before:\n");
    for (float v : startBufferContents)
    {
        printf("%f, ", v);
    }
    printf("\n");

    // Print out the values the the kernel produced
    printf("After: \n");
    for (float v : buffer)
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
