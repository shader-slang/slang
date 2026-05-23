// unit-test-slang-vm.cpp

#include "core/slang-blob.h"
#include "core/slang-memory-file-system.h"
#include "slang-com-ptr.h"
#include "slang.h"
#include "slang/slang-vm-bytecode.h"
#include "unit-test/slang-unit-test.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

using namespace Slang;

struct MinimalVMByteCode
{
    List<uint8_t> data;
    uint32_t functionOffsetOffset = 0;
    uint32_t functionHeaderOffset = 0;
    uint32_t stringOffsetOffset = 0;
};

template<typename T>
static void appendValue(List<uint8_t>& data, const T& value)
{
    data.addRange(reinterpret_cast<const uint8_t*>(&value), sizeof(value));
}

static void patchUInt32(List<uint8_t>& data, uint32_t offset, uint32_t value)
{
    memcpy(data.getBuffer() + offset, &value, sizeof(value));
}

static MinimalVMByteCode makeMinimalVMByteCode()
{
    MinimalVMByteCode result;

    appendValue(result.data, kSlangByteCodeFourCC);
    appendValue(result.data, kSlangByteCodeVersion);

    appendValue(result.data, kSlangByteCodeFunctionsFourCC);
    uint32_t functionSectionSizeOffset = (uint32_t)result.data.getCount();
    appendValue(result.data, uint32_t(0));

    appendValue(result.data, uint32_t(1));
    result.functionOffsetOffset = (uint32_t)result.data.getCount();
    appendValue(result.data, uint32_t(0));

    result.functionHeaderOffset = (uint32_t)result.data.getCount();
    patchUInt32(result.data, result.functionOffsetOffset, result.functionHeaderOffset);

    VMFuncHeader funcHeader = {};
    funcHeader.name.sectionId = kSlangByteCodeSectionStrings;
    funcHeader.name.offset = 0;
    funcHeader.codeSize = sizeof(VMInstHeader);
    appendValue(result.data, funcHeader);

    VMInstHeader retInst = {};
    retInst.opcode = VMOp::Ret;
    appendValue(result.data, retInst);

    uint32_t functionSectionSize =
        (uint32_t)result.data.getCount() - functionSectionSizeOffset - sizeof(uint32_t);
    patchUInt32(result.data, functionSectionSizeOffset, functionSectionSize);

    appendValue(result.data, kSlangByteCodeKernelBlobFourCC);
    appendValue(result.data, uint32_t(0));

    const char functionName[] = "main";
    appendValue(result.data, kSlangByteCodeConstantsFourCC);
    appendValue(result.data, uint32_t(sizeof(functionName)));
    appendValue(result.data, uint32_t(1));
    result.stringOffsetOffset = (uint32_t)result.data.getCount();
    appendValue(result.data, uint32_t(0));
    result.data.addRange(reinterpret_cast<const uint8_t*>(functionName), sizeof(functionName));

    return result;
}

static SlangResult validateVMModuleForTest(List<uint8_t>& data)
{
    ComPtr<slang::IBlob> moduleBlob = RawBlob::create(data.getBuffer(), data.getCount());
    ComPtr<slang::IBlob> disassemblyBlob;
    return slang_disassembleByteCode(moduleBlob, disassemblyBlob.writeRef());
}

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

SLANG_UNIT_TEST(slangVMRejectMalformedByteCodeOffsets)
{
    auto validByteCode = makeMinimalVMByteCode();
    SLANG_CHECK(validateVMModuleForTest(validByteCode.data) == SLANG_OK);

    auto invalidFunctionOffset = makeMinimalVMByteCode();
    patchUInt32(
        invalidFunctionOffset.data,
        invalidFunctionOffset.functionOffsetOffset,
        (uint32_t)invalidFunctionOffset.data.getCount() + sizeof(VMFuncHeader));
    SLANG_CHECK(SLANG_FAILED(validateVMModuleForTest(invalidFunctionOffset.data)));

    auto invalidFunctionName = makeMinimalVMByteCode();
    auto funcHeader = reinterpret_cast<VMFuncHeader*>(
        invalidFunctionName.data.getBuffer() + invalidFunctionName.functionHeaderOffset);
    funcHeader->name.offset = 1;
    SLANG_CHECK(SLANG_FAILED(validateVMModuleForTest(invalidFunctionName.data)));

    auto invalidStringOffset = makeMinimalVMByteCode();
    patchUInt32(invalidStringOffset.data, invalidStringOffset.stringOffsetOffset, UINT32_MAX);
    SLANG_CHECK(SLANG_FAILED(validateVMModuleForTest(invalidStringOffset.data)));

    auto unterminatedString = makeMinimalVMByteCode();
    auto stringDataOffset = unterminatedString.stringOffsetOffset + sizeof(uint32_t);
    unterminatedString.data[stringDataOffset + 4] = '!';
    SLANG_CHECK(SLANG_FAILED(validateVMModuleForTest(unterminatedString.data)));
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
