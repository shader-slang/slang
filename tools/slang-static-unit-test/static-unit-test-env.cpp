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

    ComPtr<slang::IBlob> diagnostics;
    slang::IModule* module = m_session->loadModuleFromSourceString(
        moduleName,
        moduleName,
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
