// unit-test-slang-vm.cpp

#include "core/slang-memory-file-system.h"
#include "slang-com-ptr.h"
#include "slang.h"
#include "unit-test/slang-unit-test.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

using namespace Slang;

SLANG_UNIT_TEST(slangVM)
{
    const char* testSource = R"(
        int one() { return 1; }
        int sum(int x)
        {
            int result = 0;
            for (int i = 0; i <= x; i++)
            {
                result += i;
            }
            return result + one();
        }
        [shader("dispatch")]
        int dispatchMain(uniform int2 v, out int c)
        {
            int a = v.x;
            int b = v.y;
            int tmp = 0;
            if (a > 0)
                tmp = a + b;
            else
                tmp = b - a;
            tmp += sum(b);
            c = tmp;
            return 100;
        }
    )";

    // Create Slang session and compile code.
    ComPtr<slang::IBlob> code;
    String disasmText;
    {
        ComPtr<slang::IGlobalSession> globalSession;
        SLANG_CHECK(
            slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
        slang::TargetDesc targetDesc = {};
        targetDesc.format = SLANG_HOST_VM;
        slang::SessionDesc sessionDesc = {};
        sessionDesc.targetCount = 1;
        sessionDesc.targets = &targetDesc;
        sessionDesc.compilerOptionEntryCount = 0;

        ComPtr<slang::ISession> session;
        SLANG_CHECK(globalSession->createSession(sessionDesc, session.writeRef()) == SLANG_OK);

        ComPtr<slang::IBlob> diagnosticBlob;
        auto module = session->loadModuleFromSourceString(
            "test",
            "test.slang",
            testSource,
            diagnosticBlob.writeRef());
        SLANG_CHECK(module != nullptr);

        ComPtr<slang::IComponentType> linkedProgram;
        module->link(linkedProgram.writeRef());


        linkedProgram->getTargetCode(0, code.writeRef(), diagnosticBlob.writeRef());

        SLANG_CHECK(code->getBufferSize() > 0);

        ComPtr<slang::IBlob> disasmBlob;
        SLANG_CHECK(slang_disassembleByteCode(code, disasmBlob.writeRef()) == SLANG_OK);
        disasmText = (const char*)disasmBlob->getBufferPointer();
        SLANG_CHECK(disasmText.indexOf("ret") != -1);
    }

    // Create a byte code runner and interpret the code.
    ComPtr<slang::IByteCodeRunner> runner;
    slang::ByteCodeRunnerDesc runnerDesc = {};
    SLANG_CHECK(slang_createByteCodeRunner(&runnerDesc, runner.writeRef()) == SLANG_OK);
    SLANG_CHECK(runner->loadModule(code) == SLANG_OK);
    SLANG_CHECK(runner->selectFunctionByIndex(0) == SLANG_OK);
    struct Params
    {
        int a;
        int b;
        int* result;
    };
    int result = 0;
    Params params = {1, 2, &result};
    SLANG_CHECK(runner->execute(&params, sizeof(params)) == SLANG_OK);
    SLANG_CHECK(result == 7);

    size_t returnValSize = 0;
    int* returnVal = (int*)runner->getReturnValue(&returnValSize);
    SLANG_CHECK(returnValSize == sizeof(int));
    SLANG_CHECK(*returnVal == 100);
}

struct ExtCallState
{
    int callCount = 0;
    const char* callee = nullptr;
    const char* arg0 = nullptr;
};

static void vmExtPingHandler(slang::IByteCodeRunner*, slang::VMExecInstHeader* inst, void* userData)
{
    auto* state = (ExtCallState*)userData;
    state->callCount++;

    state->callee = *(const char**)inst->getOperand(1).getPtr();
    state->arg0 = *(const char**)inst->getOperand(2).getPtr();

    int result = 22;
    SLANG_CHECK(inst->getOperand(0).size == sizeof(result));
    memcpy(inst->getOperand(0).getPtr(), &result, sizeof(result));
}

SLANG_UNIT_TEST(slangVMRegisterExtCall)
{
    const char* testSource = R"(
        __target_intrinsic(dispatch, "vmExtPing")
        int vmExtPing(String msg);

        [shader("dispatch")]
        int dispatchMain()
        {
            return vmExtPing("hello-from-vm");
        }
    )";

    ComPtr<slang::IBlob> code;
    {
        ComPtr<slang::IGlobalSession> globalSession;
        SLANG_CHECK(
            slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);

        slang::TargetDesc targetDesc = {};
        targetDesc.format = SLANG_HOST_VM;

        slang::SessionDesc sessionDesc = {};
        sessionDesc.targetCount = 1;
        sessionDesc.targets = &targetDesc;

        ComPtr<slang::ISession> session;
        SLANG_CHECK(globalSession->createSession(sessionDesc, session.writeRef()) == SLANG_OK);

        ComPtr<slang::IBlob> diagnosticBlob;
        auto module = session->loadModuleFromSourceString(
            "ext-call-test",
            "ext-call-test.slang",
            testSource,
            diagnosticBlob.writeRef());
        SLANG_CHECK(module != nullptr);

        ComPtr<slang::IComponentType> linkedProgram;
        SLANG_CHECK(module->link(linkedProgram.writeRef(), diagnosticBlob.writeRef()) == SLANG_OK);
        SLANG_CHECK(
            linkedProgram->getTargetCode(0, code.writeRef(), diagnosticBlob.writeRef()) ==
            SLANG_OK);

        SLANG_CHECK(code != nullptr);
        SLANG_CHECK(code->getBufferSize() > 0);
    }

    // Module won't load if there are unresolved references.
    {
        ComPtr<slang::IByteCodeRunner> runner;
        slang::ByteCodeRunnerDesc runnerDesc = {};
        SLANG_CHECK(slang_createByteCodeRunner(&runnerDesc, runner.writeRef()) == SLANG_OK);
        SLANG_CHECK(SLANG_FAILED(runner->loadModule(code)));
    }

    {
        ComPtr<slang::IByteCodeRunner> runner;
        slang::ByteCodeRunnerDesc runnerDesc = {};
        SLANG_CHECK(slang_createByteCodeRunner(&runnerDesc, runner.writeRef()) == SLANG_OK);

        ExtCallState state;
        runner->setExtInstHandlerUserData(&state);
        SLANG_CHECK(runner->registerExtCall("vmExtPing", vmExtPingHandler) == SLANG_OK);

        SLANG_CHECK(runner->loadModule(code) == SLANG_OK);

        int funcIndex = runner->findFunctionByName("dispatchMain");
        SLANG_CHECK(funcIndex >= 0);
        SLANG_CHECK(runner->selectFunctionByIndex((uint32_t)funcIndex) == SLANG_OK);
        SLANG_CHECK(runner->execute(nullptr, 0) == SLANG_OK);

        SLANG_CHECK(state.callCount == 1);
        SLANG_CHECK(state.callee && strcmp(state.callee, "vmExtPing") == 0);
        SLANG_CHECK(state.arg0 && strcmp(state.arg0, "hello-from-vm") == 0);

        size_t returnValueSize = 0;
        int* returnValue = (int*)runner->getReturnValue(&returnValueSize);
        SLANG_CHECK(returnValueSize == sizeof(int));
        SLANG_CHECK(*returnValue == 22);
    }
}
