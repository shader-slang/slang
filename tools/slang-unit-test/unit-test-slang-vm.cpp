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
    uint32_t functionSectionSizeOffset = 0;
    uint32_t functionOffsetOffset = 0;
    uint32_t functionHeaderOffset = 0;
    uint32_t constantBlobSizeOffset = 0;
    uint32_t stringCountOffset = 0;
    uint32_t stringOffsetOffset = 0;
};

static const char kMinimalFunctionName[] = "main";
static const char kAlternateFunctionName[] = "tail";

template<typename T>
static void appendVMTestValue(List<uint8_t>& data, const T& value)
{
    data.addRange((const uint8_t*)&value, sizeof(value));
}

static uint32_t reserveVMTestUInt32(List<uint8_t>& data)
{
    uint32_t zero = 0;
    auto offset = (uint32_t)data.getCount();
    appendVMTestValue(data, zero);
    return offset;
}

static void writeVMTestUInt32At(List<uint8_t>& data, uint32_t offset, uint32_t value)
{
    memcpy(data.getBuffer() + offset, &value, sizeof(value));
}

static VMFuncHeader* getFunctionHeaderForTest(MinimalVMByteCode& byteCode)
{
    return reinterpret_cast<VMFuncHeader*>(
        byteCode.data.getBuffer() + byteCode.functionHeaderOffset);
}

static MinimalVMByteCode makeMinimalVMByteCode()
{
    MinimalVMByteCode result;

    appendVMTestValue(result.data, kSlangByteCodeFourCC);
    appendVMTestValue(result.data, kSlangByteCodeVersion);

    appendVMTestValue(result.data, kSlangByteCodeFunctionsFourCC);
    result.functionSectionSizeOffset = (uint32_t)result.data.getCount();
    appendVMTestValue(result.data, uint32_t(0));

    appendVMTestValue(result.data, uint32_t(1));
    result.functionOffsetOffset = (uint32_t)result.data.getCount();
    appendVMTestValue(result.data, uint32_t(0));

    result.functionHeaderOffset = (uint32_t)result.data.getCount();
    writeVMTestUInt32At(result.data, result.functionOffsetOffset, result.functionHeaderOffset);

    VMFuncHeader funcHeader = {};
    funcHeader.name.sectionId = kSlangByteCodeSectionStrings;
    funcHeader.name.offset = 0;
    funcHeader.codeSize = sizeof(VMInstHeader);
    appendVMTestValue(result.data, funcHeader);

    VMInstHeader retInst = {};
    retInst.opcode = VMOp::Ret;
    appendVMTestValue(result.data, retInst);

    uint32_t functionSectionSize =
        (uint32_t)result.data.getCount() - result.functionSectionSizeOffset - sizeof(uint32_t);
    writeVMTestUInt32At(result.data, result.functionSectionSizeOffset, functionSectionSize);

    appendVMTestValue(result.data, kSlangByteCodeKernelBlobFourCC);
    appendVMTestValue(result.data, uint32_t(0));

    appendVMTestValue(result.data, kSlangByteCodeConstantsFourCC);
    result.constantBlobSizeOffset = (uint32_t)result.data.getCount();
    appendVMTestValue(result.data, uint32_t(sizeof(kMinimalFunctionName)));
    result.stringCountOffset = (uint32_t)result.data.getCount();
    appendVMTestValue(result.data, uint32_t(1));
    result.stringOffsetOffset = (uint32_t)result.data.getCount();
    appendVMTestValue(result.data, uint32_t(0));
    result.data.addRange(
        reinterpret_cast<const uint8_t*>(kMinimalFunctionName),
        sizeof(kMinimalFunctionName));

    return result;
}

static SlangResult validateVMModuleForTest(List<uint8_t>& data)
{
    ComPtr<slang::IBlob> moduleBlob = RawBlob::create(data.getBuffer(), data.getCount());
    ComPtr<slang::IBlob> disassemblyBlob;
    return slang_disassembleByteCode(moduleBlob, disassemblyBlob.writeRef());
}

static VMOperand makeVMTestOperand(uint32_t sectionId, uint32_t offset, uint32_t size)
{
    VMOperand operand = {};
    operand.sectionId = sectionId;
    operand.offset = offset;
    operand.size = size;
    operand.setType(slang::OperandDataType::General);
    return operand;
}

static void appendVMTestInst(
    List<uint8_t>& code,
    VMOp op,
    uint32_t opcodeExtension,
    ArrayView<VMOperand> operands)
{
    VMInstHeader inst = {};
    inst.opcode = op;
    inst.opcodeExtension = opcodeExtension;
    inst.operandCount = (uint32_t)operands.getCount();
    appendVMTestValue(code, inst);
    for (auto operand : operands)
        appendVMTestValue(code, operand);
}

static ComPtr<slang::IBlob> createVMTestBlob(
    List<uint8_t>& instCode,
    uint32_t workingSetSize,
    uint32_t returnValueSize,
    List<uint8_t>& constants)
{
    List<uint8_t> data;
    appendVMTestValue(data, kSlangByteCodeFourCC);
    appendVMTestValue(data, kSlangByteCodeVersion);

    appendVMTestValue(data, kSlangByteCodeFunctionsFourCC);
    auto functionSectionSizeOffset = reserveVMTestUInt32(data);
    auto functionSectionStart = (uint32_t)data.getCount();
    uint32_t functionCount = 1;
    appendVMTestValue(data, functionCount);
    auto functionOffsetOffset = reserveVMTestUInt32(data);

    auto functionOffset = (uint32_t)data.getCount();
    writeVMTestUInt32At(data, functionOffsetOffset, functionOffset);

    VMFuncHeader funcHeader = {};
    funcHeader.name.sectionId = kSlangByteCodeSectionStrings;
    funcHeader.name.offset = 0;
    funcHeader.workingSetSizeInBytes = workingSetSize;
    funcHeader.codeSize = (uint32_t)instCode.getCount();
    funcHeader.returnValueSizeInBytes = returnValueSize;
    appendVMTestValue(data, funcHeader);
    data.addRange(instCode.getBuffer(), instCode.getCount());

    auto functionSectionSize = (uint32_t)data.getCount() - functionSectionStart;
    writeVMTestUInt32At(data, functionSectionSizeOffset, functionSectionSize);

    appendVMTestValue(data, kSlangByteCodeKernelBlobFourCC);
    uint32_t kernelBlobSize = 0;
    appendVMTestValue(data, kernelBlobSize);

    appendVMTestValue(data, kSlangByteCodeConstantsFourCC);
    auto constantBlobSize = (uint32_t)constants.getCount();
    appendVMTestValue(data, constantBlobSize);
    uint32_t stringCount = 1;
    uint32_t mainStringOffset = 0;
    appendVMTestValue(data, stringCount);
    appendVMTestValue(data, mainStringOffset);
    data.addRange(constants.getBuffer(), constants.getCount());

    ComPtr<slang::IBlob> blob;
    blob.attach(slang_createBlob(data.getBuffer(), data.getCount()));
    return blob;
}

static void appendVMTestMainString(List<uint8_t>& constants)
{
    char mainName[] = "main";
    constants.addRange((uint8_t*)mainName, sizeof(mainName));
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

    auto validLastFunctionNameIndex = makeMinimalVMByteCode();
    uint32_t alternateNameOffset = sizeof(kMinimalFunctionName);
    auto stringDataOffset = validLastFunctionNameIndex.stringOffsetOffset + sizeof(uint32_t);
    validLastFunctionNameIndex.data.insertRange(
        stringDataOffset,
        reinterpret_cast<const uint8_t*>(&alternateNameOffset),
        sizeof(alternateNameOffset));
    validLastFunctionNameIndex.data.addRange(
        reinterpret_cast<const uint8_t*>(kAlternateFunctionName),
        sizeof(kAlternateFunctionName));
    writeVMTestUInt32At(
        validLastFunctionNameIndex.data,
        validLastFunctionNameIndex.constantBlobSizeOffset,
        uint32_t(sizeof(kMinimalFunctionName) + sizeof(kAlternateFunctionName)));
    writeVMTestUInt32At(
        validLastFunctionNameIndex.data,
        validLastFunctionNameIndex.stringCountOffset,
        2);
    getFunctionHeaderForTest(validLastFunctionNameIndex)->name.offset = 1;
    SLANG_CHECK(validateVMModuleForTest(validLastFunctionNameIndex.data) == SLANG_OK);

    auto invalidFunctionSectionSize = makeMinimalVMByteCode();
    writeVMTestUInt32At(
        invalidFunctionSectionSize.data,
        invalidFunctionSectionSize.functionSectionSizeOffset,
        (uint32_t)invalidFunctionSectionSize.data.getCount());
    SLANG_CHECK(SLANG_FAILED(validateVMModuleForTest(invalidFunctionSectionSize.data)));

    auto invalidFunctionOffsetTableSize = makeMinimalVMByteCode();
    writeVMTestUInt32At(
        invalidFunctionOffsetTableSize.data,
        invalidFunctionOffsetTableSize.functionSectionSizeOffset,
        uint32_t(sizeof(uint32_t)));
    SLANG_CHECK(SLANG_FAILED(validateVMModuleForTest(invalidFunctionOffsetTableSize.data)));

    auto invalidFunctionOffset = makeMinimalVMByteCode();
    writeVMTestUInt32At(
        invalidFunctionOffset.data,
        invalidFunctionOffset.functionOffsetOffset,
        (uint32_t)invalidFunctionOffset.data.getCount() + sizeof(VMFuncHeader));
    SLANG_CHECK(SLANG_FAILED(validateVMModuleForTest(invalidFunctionOffset.data)));

    auto invalidParameterTableSize = makeMinimalVMByteCode();
    getFunctionHeaderForTest(invalidParameterTableSize)->parameterCount =
        (uint32_t)invalidParameterTableSize.data.getCount();
    SLANG_CHECK(SLANG_FAILED(validateVMModuleForTest(invalidParameterTableSize.data)));

    auto invalidFunctionCodeSize = makeMinimalVMByteCode();
    getFunctionHeaderForTest(invalidFunctionCodeSize)->codeSize =
        (uint32_t)invalidFunctionCodeSize.data.getCount();
    SLANG_CHECK(SLANG_FAILED(validateVMModuleForTest(invalidFunctionCodeSize.data)));

    auto invalidFunctionName = makeMinimalVMByteCode();
    getFunctionHeaderForTest(invalidFunctionName)->name.offset = 1;
    SLANG_CHECK(SLANG_FAILED(validateVMModuleForTest(invalidFunctionName.data)));

    auto invalidStringOffsetTableSize = makeMinimalVMByteCode();
    writeVMTestUInt32At(
        invalidStringOffsetTableSize.data,
        invalidStringOffsetTableSize.stringCountOffset,
        (uint32_t)invalidStringOffsetTableSize.data.getCount());
    SLANG_CHECK(SLANG_FAILED(validateVMModuleForTest(invalidStringOffsetTableSize.data)));

    auto invalidConstantBlobSize = makeMinimalVMByteCode();
    writeVMTestUInt32At(
        invalidConstantBlobSize.data,
        invalidConstantBlobSize.constantBlobSizeOffset,
        (uint32_t)invalidConstantBlobSize.data.getCount());
    SLANG_CHECK(SLANG_FAILED(validateVMModuleForTest(invalidConstantBlobSize.data)));

    auto invalidStringOffset = makeMinimalVMByteCode();
    writeVMTestUInt32At(
        invalidStringOffset.data,
        invalidStringOffset.stringOffsetOffset,
        uint32_t(sizeof(kMinimalFunctionName)));
    SLANG_CHECK(SLANG_FAILED(validateVMModuleForTest(invalidStringOffset.data)));

    auto unterminatedString = makeMinimalVMByteCode();
    stringDataOffset = unterminatedString.stringOffsetOffset + sizeof(uint32_t);
    unterminatedString.data[stringDataOffset + 4] = '!';
    SLANG_CHECK(SLANG_FAILED(validateVMModuleForTest(unterminatedString.data)));
}

SLANG_UNIT_TEST(slangVMRejectsOutOfBoundsOperand)
{
    List<uint8_t> constants;
    appendVMTestMainString(constants);

    List<VMOperand> operands;
    operands.add(makeVMTestOperand(kSlangByteCodeSectionWorkingSet, 8, sizeof(uint32_t)));
    operands.add(makeVMTestOperand(kSlangByteCodeSectionConstants, 0, sizeof(uint32_t)));

    List<uint8_t> instCode;
    appendVMTestInst(instCode, VMOp::Copy, sizeof(uint32_t), operands.getArrayView());

    auto blob = createVMTestBlob(instCode, 8, 0, constants);
    ComPtr<slang::IByteCodeRunner> runner;
    slang::ByteCodeRunnerDesc runnerDesc = {};
    SLANG_CHECK(slang_createByteCodeRunner(&runnerDesc, runner.writeRef()) == SLANG_OK);
    SLANG_CHECK(SLANG_FAILED(runner->loadModule(blob)));
}

SLANG_UNIT_TEST(slangVMRejectsOversizedCopy)
{
    List<uint8_t> constants;
    appendVMTestMainString(constants);
    uint32_t value = 0;
    auto valueOffset = (uint32_t)constants.getCount();
    appendVMTestValue(constants, value);

    List<VMOperand> operands;
    operands.add(makeVMTestOperand(kSlangByteCodeSectionWorkingSet, 0, sizeof(uint32_t)));
    operands.add(makeVMTestOperand(kSlangByteCodeSectionConstants, valueOffset, sizeof(uint32_t)));

    List<uint8_t> instCode;
    appendVMTestInst(instCode, VMOp::Copy, 16, operands.getArrayView());

    auto blob = createVMTestBlob(instCode, 8, 0, constants);
    ComPtr<slang::IByteCodeRunner> runner;
    slang::ByteCodeRunnerDesc runnerDesc = {};
    SLANG_CHECK(slang_createByteCodeRunner(&runnerDesc, runner.writeRef()) == SLANG_OK);
    SLANG_CHECK(runner->loadModule(blob) == SLANG_OK);
    SLANG_CHECK(runner->selectFunctionByIndex(0) == SLANG_OK);
    SLANG_CHECK(SLANG_FAILED(runner->execute(nullptr, 0)));
}

SLANG_UNIT_TEST(slangVMRejectsWorkingSetPointerEscape)
{
    List<uint8_t> constants;
    appendVMTestMainString(constants);
    while (constants.getCount() % sizeof(uint32_t))
        constants.add(0);
    uint32_t elementIndex = 16;
    auto indexOffset = (uint32_t)constants.getCount();
    appendVMTestValue(constants, elementIndex);

    List<uint8_t> instCode;

    List<VMOperand> getWorkingSetPtrOperands;
    getWorkingSetPtrOperands.add(
        makeVMTestOperand(kSlangByteCodeSectionWorkingSet, 0, sizeof(void*)));
    appendVMTestInst(instCode, VMOp::GetWorkingSetPtr, 0, getWorkingSetPtrOperands.getArrayView());

    List<VMOperand> getElementPtrOperands;
    getElementPtrOperands.add(makeVMTestOperand(kSlangByteCodeSectionWorkingSet, 8, sizeof(void*)));
    getElementPtrOperands.add(makeVMTestOperand(kSlangByteCodeSectionWorkingSet, 0, sizeof(void*)));
    getElementPtrOperands.add(
        makeVMTestOperand(kSlangByteCodeSectionConstants, indexOffset, sizeof(uint32_t)));
    appendVMTestInst(
        instCode,
        VMOp::GetElementPtr,
        sizeof(uint32_t),
        getElementPtrOperands.getArrayView());

    auto blob = createVMTestBlob(instCode, 24, 0, constants);
    ComPtr<slang::IByteCodeRunner> runner;
    slang::ByteCodeRunnerDesc runnerDesc = {};
    SLANG_CHECK(slang_createByteCodeRunner(&runnerDesc, runner.writeRef()) == SLANG_OK);
    SLANG_CHECK(runner->loadModule(blob) == SLANG_OK);
    SLANG_CHECK(runner->selectFunctionByIndex(0) == SLANG_OK);
    SLANG_CHECK(SLANG_FAILED(runner->execute(nullptr, 0)));
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
