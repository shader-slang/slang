// main.cpp

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "../../source/core/secure-crt.h"
#include <slang.h>

int main(
    int argc,
    char** argv)
{
    // TODO: parse arguments

    assert(argc >= 2);
    char const* inputPath = argv[1];

    // Slurp in the input file, so that we can compile and run it
    FILE* inputFile;
    fopen_s(&inputFile, inputPath, "rb");
    assert(inputFile);

    fseek(inputFile, 0, SEEK_END);
    size_t inputSize = ftell(inputFile);
    fseek(inputFile, 0, SEEK_SET);

    char* inputText = (char*) malloc(inputSize + 1);
    fread(inputText, inputSize, 1, inputFile);
    inputText[inputSize] = 0;
    fclose(inputFile);

    // TODO: scan through the text to find comments,
    // that instruct us how to generate input and
    // consume output when running the test.

    //

    SlangSession* session = spCreateSession(nullptr);
    SlangCompileRequest* request = spCreateCompileRequest(session);

    spSetCompileFlags(
        request,
        SLANG_COMPILE_FLAG_USE_IR);

    spSetOutputContainerFormat(
        request,
        SLANG_CONTAINER_FORMAT_SLANG_MODULE);

    int translationUnitIndex = spAddTranslationUnit(
        request,
        SLANG_SOURCE_LANGUAGE_SLANG,
        nullptr);

    spAddTranslationUnitSourceString(
        request,
        translationUnitIndex,
        inputPath,
        inputText);

    int entryPointIndex = spAddEntryPoint(
        request,
        translationUnitIndex,
        "main",
        spFindProfile(session, "cs_5_0"));

    if( spCompile(request) != 0 )
    {
        char const* output = spGetDiagnosticOutput(request);
        fputs(output, stderr);
        exit(1);
    }

    // Things compiled, so now we need to run them...

    // Extract the bytecode
    size_t bytecodeSize = 0;
    void const* bytecode = spGetCompileRequestCode(request, &bytecodeSize);

    // Now we need to create an execution context to go and run the bytecode we got

    SlangVM* vm = SlangVM_create();

    SlangVMModule* vmModule = SlangVMModule_load(
        vm,
        bytecode,
        bytecodeSize);

    SlangVMFunc* vmFunc = (SlangVMFunc*)SlangVMModule_findGlobalSymbolPtr(
        vmModule,
        "main");

    int32_t*& inputArg = *(int32_t**)SlangVMModule_findGlobalSymbolPtr(
        vmModule,
        "input");

    int32_t*& outputArg = *(int32_t**)SlangVMModule_findGlobalSymbolPtr(
        vmModule,
        "output");

    SlangVMThread* vmThread = SlangVMThread_create(
        vm);

    int32_t inputData[8] = { 0, 1, 2, 3, 4, 5, 6, 7 };
    int32_t outputData[8] = { 0 };

    inputArg = inputData;
    outputArg = outputData;

    // TODO: set arguments based on specification from the user...
    for (uint32_t threadID = 0; threadID < 8; ++threadID)
    {
#if 0
        fprintf(stderr, "\n\nthreadID = %u\n\n", threadID);
        fflush(stderr);
#endif

        SlangVMThread_beginCall(vmThread, vmFunc);

        SlangVMThread_setArg(
            vmThread,
            0,
            &threadID,
            sizeof(threadID));

        SlangVMThread_resume(vmThread);
    }

    for (uint32_t ii = 0; ii < 8; ++ii)
    {
        fprintf(stdout, "outputData[%u] = %d\n", ii, outputData[ii]);
    }

    spDestroyCompileRequest(request);
    spDestroySession(session);

    return 0;
}
