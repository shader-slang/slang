// static-unit-test-env.cpp

#include "static-unit-test-env.h"

#include "slang/slang-ast-builder.h"
#include "slang/slang-module.h"
#include "slang/slang-session.h"

namespace Slang
{

StaticUnitTestEnv::StaticUnitTestEnv(UnitTestContext* context)
{
    // The target choice is arbitrary for tests that never generate code, but a
    // session needs at least one target to be well-formed.
    slang::TargetDesc targetDesc = {};
    targetDesc.format = SLANG_SPIRV;
    targetDesc.profile = context->slangGlobalSession->findProfile("spirv_1_5");

    slang::SessionDesc sessionDesc = {};
    sessionDesc.targetCount = 1;
    sessionDesc.targets = &targetDesc;

    context->slangGlobalSession->createSession(sessionDesc, m_session.writeRef());
    SLANG_RELEASE_ASSERT(m_session);

    // `Linkage` is the concrete internal class implementing the public
    // `ISession`, and it is the route from public API to internal compiler
    // state: `Linkage::getSessionImpl()` yields the `Session` that owns IR
    // allocation, and `Linkage::getASTBuilder()` yields the AST builder.
    m_linkage = static_cast<Linkage*>(m_session.get());
}

Session* StaticUnitTestEnv::getSessionImpl() const
{
    return m_linkage->getSessionImpl();
}

ASTBuilder* StaticUnitTestEnv::getASTBuilder() const
{
    return m_linkage->getASTBuilder();
}

Module* StaticUnitTestEnv::checkModuleFromSource(
    const char* moduleName,
    const char* source,
    String* outDiagnostics)
{
    // Loading two modules under one name would silently return the cached first
    // module, turning the second case into a no-op that still passes. Fail
    // loudly instead.
    for (auto const& used : m_usedModuleNames)
    {
        SLANG_RELEASE_ASSERT(used != moduleName);
    }
    m_usedModuleNames.add(moduleName);

    // `loadModuleFromSourceString` takes a path separately from the module name, and
    // uses it for the source locations in diagnostics. These sources are in memory and
    // have no file, so synthesize one from the name rather than passing the bare name:
    // `outDiagnostics` is how tests read diagnostics back, and "myTest.slang(3): error"
    // reads as a location while "myTest(3): error" reads as nothing in particular.
    StringBuilder pathBuilder;
    pathBuilder << moduleName << ".slang";
    const String path = pathBuilder.produceString();

    ComPtr<slang::IBlob> diagnostics;
    slang::IModule* module = m_session->loadModuleFromSourceString(
        moduleName,
        path.getBuffer(),
        source,
        diagnostics.writeRef());

    if (outDiagnostics && diagnostics)
    {
        *outDiagnostics = String((const char*)diagnostics->getBufferPointer());
    }

    // `Module` is the concrete internal implementation of the public `slang::IModule`,
    // the same relationship as `Linkage` to `ISession` in the constructor above, so this
    // downcast is sound for any `IModule` this linkage produced. The pointer is borrowed:
    // the linkage's module cache owns it, so this environment must not release it.
    return static_cast<Module*>(module);
}

IRFixtureBuilder::IRFixtureBuilder(Session* session)
    : m_module(IRModule::create(session)), m_builder(m_module.get())
{
}

IRFunc* IRFixtureBuilder::beginVoidFunction(const char* name)
{
    m_builder.setInsertInto(m_module.get());

    IRFunc* func = m_builder.createFunc();
    func->setFullType(m_builder.getFuncType(0, nullptr, m_builder.getVoidType()));
    m_builder.addNameHintDecoration(func, UnownedStringSlice(name));

    // A function needs an entry block with a terminator to be well-formed. The block
    // is opened here and terminated by `endVoidFunction`, so a caller can emit a body
    // in between.
    m_builder.setInsertInto(func);
    m_builder.emitBlock();

    return func;
}

void IRFixtureBuilder::endVoidFunction(IRFunc* func, bool keepAlive)
{
    m_builder.emitReturn();

    if (keepAlive)
    {
        m_builder.addKeepAliveDecoration(func);
    }
}

IRFunc* IRFixtureBuilder::addVoidFunction(const char* name, bool keepAlive)
{
    IRFunc* func = beginVoidFunction(name);
    endVoidFunction(func, keepAlive);
    return func;
}

IRFunc* IRFixtureBuilder::addVoidFunctionCalling(const char* name, bool keepAlive, IRFunc* callee)
{
    // The call emitted below is only well-formed if `callee` is a `void()` function
    // belonging to this module. Getting that wrong produces malformed IR that fails
    // somewhere inside a later pass, which reads as a bug in the pass under test rather
    // than a bad fixture, so check it here where the caller's mistake is still visible.
    SLANG_RELEASE_ASSERT(callee);
    SLANG_RELEASE_ASSERT(callee->getParent() == m_module->getModuleInst());
    auto calleeType = as<IRFuncType>(callee->getFullType());
    SLANG_RELEASE_ASSERT(calleeType);
    SLANG_RELEASE_ASSERT(calleeType->getParamCount() == 0);
    SLANG_RELEASE_ASSERT(calleeType->getResultType()->getOp() == kIROp_VoidType);

    IRFunc* func = beginVoidFunction(name);
    m_builder.emitCallInst(m_builder.getVoidType(), callee, 0, nullptr);
    endVoidFunction(func, keepAlive);
    return func;
}

IRFunc* IRFixtureBuilder::addExportedVoidFunction(const char* name)
{
    IRFunc* func = beginVoidFunction(name);
    endVoidFunction(func, /* keepAlive: */ false);
    m_builder.addExportDecoration(func, UnownedStringSlice(name));
    return func;
}

IRFunc* IRFixtureBuilder::addVoidFunctionWithLayout(const char* name)
{
    IRFunc* func = beginVoidFunction(name);
    endVoidFunction(func, /* keepAlive: */ false);

    // An empty layout is enough: `keepLayoutsAlive` tests only for the presence of an
    // `IRLayoutDecoration`, not for anything the layout says.
    IRTypeLayout::Builder layoutBuilder(&m_builder);
    m_builder.addLayoutDecoration(func, layoutBuilder.build());

    return func;
}

IRInst* IRFixtureBuilder::addLiveWeakUseOf(IRFunc* target)
{
    SLANG_RELEASE_ASSERT(target);
    SLANG_RELEASE_ASSERT(target->getParent() == m_module->getModuleInst());

    m_builder.setInsertInto(m_module.get());

    // Hoistable, so this is placed at module scope regardless of the insert location.
    IRInst* weakUse = m_builder.getWeakUse(target);

    // Without this the `WeakUse` is itself dead, and `target` would be removed for
    // that reason alone -- which would make any test built on it pass whether or not
    // weak operands are honoured. Keeping the `WeakUse` alive is what isolates the
    // behaviour under test.
    m_builder.addKeepAliveDecoration(weakUse);

    return weakUse;
}

IRFunc* IRFixtureBuilder::addVoidFunctionWithUnusedBlockParam(const char* name, bool keepAlive)
{
    m_builder.setInsertInto(m_module.get());

    IRFunc* func = m_builder.createFunc();
    func->setFullType(m_builder.getFuncType(0, nullptr, m_builder.getVoidType()));
    m_builder.addNameHintDecoration(func, UnownedStringSlice(name));

    m_builder.setInsertInto(func);
    IRBlock* entryBlock = m_builder.emitBlock();
    IRBlock* secondBlock = m_builder.emitBlock();

    // The parameter is never read, so DCE removes it and then reruns -- which also
    // requires rewriting the branch in the entry block that supplies its argument.
    m_builder.setInsertInto(secondBlock);
    m_builder.emitParam(m_builder.getIntType());
    m_builder.emitReturn();

    m_builder.setInsertInto(entryBlock);
    IRInst* arg = m_builder.getIntValue(m_builder.getIntType(), 7);
    m_builder.emitBranch(secondBlock, 1, &arg);

    if (keepAlive)
    {
        m_builder.addKeepAliveDecoration(func);
    }

    return func;
}

IRStructType* IRFixtureBuilder::addOptimizableStructWithUnusedField(const char* name)
{
    m_builder.setInsertInto(m_module.get());

    IRStructType* structType = m_builder.createStructType();
    m_builder.addNameHintDecoration(structType, UnownedStringSlice(name));

    IRStructKey* fieldKey = m_builder.createStructKey();
    m_builder.addNameHintDecoration(fieldKey, UnownedStringSlice("unusedField"));
    m_builder.createStructField(structType, fieldKey, m_builder.getIntType());

    // Without this decoration `trimOptimizableTypes` skips the type entirely, so it is
    // what makes the pass consider the struct at all.
    m_builder.addDecoration(structType, kIROp_OptimizableTypeDecoration);

    return structType;
}

List<IRInst*> IRFixtureBuilder::addStraightLineMarkerRun(const char* name, Index markerCount)
{
    IRFunc* func = beginVoidFunction(name);

    List<IRInst*> markers;
    for (Index i = 0; i < markerCount; ++i)
        markers.add(m_builder.emitIncrementCoverageCounter());

    endVoidFunction(func, /* keepAlive: */ false);
    return markers;
}

List<IRInst*> IRFixtureBuilder::addMarkerRunAroundCall(const char* name, IRFunc* callee)
{
    // The call emitted below is only well-formed for a `void()` callee in this
    // module, the same precondition `addVoidFunctionCalling` checks; validate it
    // here where the caller's mistake is still visible rather than let a later
    // pass trip over the malformed IR.
    SLANG_RELEASE_ASSERT(callee);
    SLANG_RELEASE_ASSERT(callee->getParent() == m_module->getModuleInst());
    auto calleeType = as<IRFuncType>(callee->getFullType());
    SLANG_RELEASE_ASSERT(calleeType);
    SLANG_RELEASE_ASSERT(calleeType->getParamCount() == 0);
    SLANG_RELEASE_ASSERT(calleeType->getResultType()->getOp() == kIROp_VoidType);

    IRFunc* func = beginVoidFunction(name);

    List<IRInst*> markers;
    markers.add(m_builder.emitIncrementCoverageCounter());
    m_builder.emitCallInst(m_builder.getVoidType(), callee, 0, nullptr);
    markers.add(m_builder.emitIncrementCoverageCounter());

    endVoidFunction(func, /* keepAlive: */ false);
    return markers;
}

List<IRInst*> IRFixtureBuilder::addMarkerRunSplitByAbort(const char* name)
{
    IRFunc* func = beginVoidFunction(name);

    List<IRInst*> markers;
    markers.add(m_builder.emitIncrementCoverageCounter());
    // `Abort` carries a format operand; its contents are irrelevant to the exit
    // analysis, which keys only off the opcode, so any string will do.
    IRInst* format = m_builder.getStringValue(UnownedStringSlice("coverage-unit-test"));
    m_builder.emitIntrinsicInst(m_builder.getVoidType(), kIROp_Abort, 1, &format);
    markers.add(m_builder.emitIncrementCoverageCounter());

    endVoidFunction(func, /* keepAlive: */ false);
    return markers;
}

IRFunc* IRFixtureBuilder::addFunctionEndingInGenericAsm(const char* name)
{
    IRFunc* func = beginVoidFunction(name);
    // `GenericAsm` is itself a terminator, so it stands in for the return
    // `endVoidFunction` would otherwise add.
    m_builder.emitGenericAsm(UnownedStringSlice("coverage-unit-test"));
    return func;
}

void IRFixtureBuilder::addMutuallyRecursiveFunctions(
    const char* aName,
    const char* bName,
    IRFunc*& outA,
    IRFunc*& outB)
{
    // Open both functions' entry blocks before emitting either body, since each
    // body has to name the other.
    outA = beginVoidFunction(aName);
    outB = beginVoidFunction(bName);

    m_builder.setInsertInto(outA->getFirstBlock());
    m_builder.emitCallInst(m_builder.getVoidType(), outB, 0, nullptr);
    m_builder.emitReturn();

    m_builder.setInsertInto(outB->getFirstBlock());
    m_builder.emitCallInst(m_builder.getVoidType(), outA, 0, nullptr);
    m_builder.emitReturn();
}

IRGlobalParam* IRFixtureBuilder::addGlobalParam(const char* name)
{
    m_builder.setInsertInto(m_module.get());

    IRGlobalParam* param = m_builder.createGlobalParam(m_builder.getFloatType());
    m_builder.addNameHintDecoration(param, UnownedStringSlice(name));

    return param;
}

Int IRFixtureBuilder::countGlobalInsts(IROp op) const
{
    Int count = 0;
    for (auto inst : m_module->getGlobalInsts())
    {
        if (inst->getOp() == op)
            count++;
    }
    return count;
}

List<String> IRFixtureBuilder::getFunctionNames() const
{
    List<String> names;
    for (auto inst : m_module->getGlobalInsts())
    {
        if (inst->getOp() != kIROp_Func)
            continue;
        if (auto nameHint = inst->findDecoration<IRNameHintDecoration>())
            names.add(String(nameHint->getName()));
    }
    return names;
}

String IRFixtureBuilder::dump() const
{
    String result;
    m_module->getModuleInst()->dump(result);
    return result;
}

} // namespace Slang
