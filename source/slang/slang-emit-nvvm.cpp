#include "slang-emit-nvvm.h"

#include "compiler-core/slang-artifact-impl.h"
#include "compiler-core/slang-artifact-util.h"
#include "slang-code-gen.h"
#include "slang-diagnostics.h"
#include "slang-ir-insts.h"

namespace Slang
{
namespace
{

struct ScopedNVVMModule
{
    const NVVMIRBuilder* builder = nullptr;
    SlangNVVMModuleHandle_1 module = nullptr;

    ~ScopedNVVMModule()
    {
        if (builder && module)
            builder->destroyModule(module);
    }
};

SlangResult _diagnoseUnsupportedIR(
    CodeGenContext* codeGenContext,
    const UnownedStringSlice& construct)
{
    codeGenContext->getSink()->diagnose(
        Diagnostics::NvvmUnsupportedIr{.construct = String(construct)});
    return SLANG_E_NOT_IMPLEMENTED;
}

SlangResult _requireBuilderOperation(
    CodeGenContext* codeGenContext,
    const char* operation,
    SlangResult result)
{
    if (SLANG_SUCCEEDED(result))
        return result;

    codeGenContext->getSink()->diagnose(Diagnostics::NvvmIrBuilderOperationFailed{
        .operation = String(operation),
        .resultCode = result,
    });
    return result;
}

bool _isSelectedEntryPoint(const LinkedIR& linkedIR, IRInst* globalInst)
{
    for (auto entryPoint : linkedIR.entryPoints)
    {
        if (entryPoint == globalInst)
            return true;
    }
    return false;
}

} // namespace

SlangResult validateNVVMMinimalComputeIR(CodeGenContext* codeGenContext, const LinkedIR& linkedIR)
{
    if (!linkedIR.module || linkedIR.entryPoints.getCount() != 1)
        return _diagnoseUnsupportedIR(codeGenContext, toSlice("entry-point count"));

    IRFunc* entryPoint = linkedIR.entryPoints[0];
    if (!entryPoint || !entryPoint->isDefinition())
        return _diagnoseUnsupportedIR(codeGenContext, toSlice("entry-point definition"));

    auto entryPointDecoration = entryPoint->findDecoration<IREntryPointDecoration>();
    if (!entryPointDecoration)
        return _diagnoseUnsupportedIR(codeGenContext, toSlice("entry-point decoration"));
    if (entryPointDecoration->getProfile().getStage() != Stage::Compute)
        return _diagnoseUnsupportedIR(codeGenContext, toSlice("entry-point stage"));
    if (!entryPointDecoration->getName()->getStringSlice().getLength())
        return _diagnoseUnsupportedIR(codeGenContext, toSlice("entry-point name"));
    if (!as<IRVoidType>(entryPoint->getResultType()))
        return _diagnoseUnsupportedIR(codeGenContext, toSlice("entry-point result type"));
    if (entryPoint->getParamCount() != 0)
        return _diagnoseUnsupportedIR(codeGenContext, toSlice("entry-point parameter"));

    IRBlock* entryBlock = nullptr;
    Index blockCount = 0;
    for (auto block : entryPoint->getBlocks())
    {
        entryBlock = block;
        ++blockCount;
    }
    if (blockCount != 1)
        return _diagnoseUnsupportedIR(codeGenContext, toSlice("basic-block count"));
    if (entryBlock->getFirstParam())
        return _diagnoseUnsupportedIR(codeGenContext, toSlice("basic-block parameter"));

    IRTerminatorInst* terminator = entryBlock->getTerminator();
    IRInst* firstOrdinaryInst = entryBlock->getFirstOrdinaryInst();
    if (firstOrdinaryInst != terminator)
    {
        return _diagnoseUnsupportedIR(
            codeGenContext,
            firstOrdinaryInst ? UnownedStringSlice(getIROpInfo(firstOrdinaryInst->getOp()).name)
                              : toSlice("missing terminator"));
    }

    auto returnInst = as<IRReturn>(terminator);
    if (!returnInst)
    {
        return _diagnoseUnsupportedIR(
            codeGenContext,
            terminator ? UnownedStringSlice(getIROpInfo(terminator->getOp()).name)
                       : toSlice("missing return"));
    }
    if (!returnInst->getVal() || returnInst->getVal()->getOp() != kIROp_VoidLit)
        return _diagnoseUnsupportedIR(codeGenContext, toSlice("return value"));

    // Linking can retain module-scope types, layouts, capabilities, and constants needed to spell
    // the selected function. They are hoistable representation nodes, not omitted storage or code.
    // Reject every other semantic global so this emitter cannot silently drop a parameter, helper,
    // initializer, or exported function while fabricating an apparently valid empty kernel.
    for (auto globalInst : linkedIR.module->getGlobalInsts())
    {
        if (as<IRDecoration>(globalInst) || as<IRConstant>(globalInst) ||
            _isSelectedEntryPoint(linkedIR, globalInst) ||
            getIROpInfo(globalInst->getOp()).isHoistable())
        {
            continue;
        }
        return _diagnoseUnsupportedIR(
            codeGenContext,
            UnownedStringSlice(getIROpInfo(globalInst->getOp()).name));
    }

    return SLANG_OK;
}

SlangResult emitNVVMIRFromLinkedIR(
    CodeGenContext* codeGenContext,
    const LinkedIR& linkedIR,
    const NVVMIRBuilder& builder,
    ComPtr<IArtifact>& outArtifact)
{
    outArtifact.setNull();
    SLANG_RELEASE_ASSERT(linkedIR.entryPoints.getCount() == 1);

    IRFunc* entryPoint = linkedIR.entryPoints[0];
    auto entryPointDecoration = entryPoint->findDecoration<IREntryPointDecoration>();
    SLANG_RELEASE_ASSERT(entryPointDecoration);

    ScopedNVVMModule moduleScope;
    moduleScope.builder = &builder;
    SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
        codeGenContext,
        "module creation",
        builder.createModule(toSlice("slang-direct-nvvm"), moduleScope.module)));

    SlangNVVMTypeHandle_1 voidType = nullptr;
    SlangNVVMTypeHandle_1 functionType = nullptr;
    SlangNVVMValueHandle_1 function = nullptr;
    SlangNVVMBlockHandle_1 entryBlock = nullptr;
    SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
        codeGenContext,
        "void type",
        builder.getVoidType(moduleScope.module, voidType)));
    SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
        codeGenContext,
        "function type",
        builder.getFunctionType(moduleScope.module, voidType, nullptr, 0, functionType)));
    SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
        codeGenContext,
        "function declaration",
        builder.declareFunction(
            moduleScope.module,
            functionType,
            entryPointDecoration->getName()->getStringSlice(),
            function)));
    SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
        codeGenContext,
        "basic-block creation",
        builder.createBlock(moduleScope.module, function, toSlice("entry"), entryBlock)));
    SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
        codeGenContext,
        "insertion-block selection",
        builder.setInsertBlock(moduleScope.module, entryBlock)));
    SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
        codeGenContext,
        "void return",
        builder.emitReturnVoid(moduleScope.module)));
    SLANG_RETURN_ON_FAIL(_requireBuilderOperation(
        codeGenContext,
        "kernel annotation",
        builder.markFunctionAsKernel(moduleScope.module, function)));

    if (!builder.supportsSerializationDiagnostics())
    {
        return _requireBuilderOperation(
            codeGenContext,
            "verified bitcode serialization",
            SLANG_E_NOT_AVAILABLE);
    }

    ComPtr<ISlangBlob> bitcode;
    String verifierDiagnostics;
    SlangResult serializationResult = builder.serializeModule(
        moduleScope.module,
        SLANG_NVVM_SERIALIZATION_FORMAT_BITCODE,
        bitcode,
        verifierDiagnostics);
    if (SLANG_FAILED(serializationResult))
    {
        _requireBuilderOperation(
            codeGenContext,
            "verified bitcode serialization",
            serializationResult);
        if (verifierDiagnostics.getLength())
        {
            codeGenContext->getSink()->diagnoseRaw(
                Severity::Note,
                verifierDiagnostics.getUnownedSlice());
        }
        return serializationResult;
    }
    if (verifierDiagnostics.getLength())
    {
        codeGenContext->getSink()->diagnoseRaw(
            Severity::Note,
            verifierDiagnostics.getUnownedSlice());
    }
    if (!bitcode || !bitcode->getBufferSize())
    {
        return _requireBuilderOperation(
            codeGenContext,
            "verified bitcode serialization",
            SLANG_FAIL);
    }

    auto artifact = ArtifactUtil::createArtifact(ArtifactDesc::make(
        ArtifactKind::ObjectCode,
        ArtifactPayload::LLVMIR,
        ArtifactStyle::Kernel));
    artifact->addRepresentationUnknown(bitcode);
    ArtifactUtil::addAssociated(artifact, linkedIR.metadata);
    outArtifact = artifact;
    return SLANG_OK;
}

} // namespace Slang
