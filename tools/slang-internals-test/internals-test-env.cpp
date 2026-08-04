// internals-test-env.cpp

#include "internals-test-env.h"

#include "slang/slang-ast-builder.h"
#include "slang/slang-module.h"
#include "slang/slang-session.h"

namespace Slang
{

InternalsTestEnv::InternalsTestEnv(UnitTestContext* context)
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

Session* InternalsTestEnv::getSession() const
{
    return m_linkage->getSessionImpl();
}

ASTBuilder* InternalsTestEnv::getASTBuilder() const
{
    return m_linkage->getASTBuilder();
}

Module* InternalsTestEnv::checkModuleFromSource(
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
    slang::IModule* module =
        m_session->loadModuleFromSourceString(moduleName, moduleName, source, diagnostics.writeRef());

    if (outDiagnostics && diagnostics)
    {
        *outDiagnostics = String((const char*)diagnostics->getBufferPointer());
    }

    return static_cast<Module*>(module);
}

IRFixtureBuilder::IRFixtureBuilder(Session* session)
    : m_module(IRModule::create(session)), m_builder(m_module.get())
{
}

IRFunc* IRFixtureBuilder::addVoidFunction(const char* name, bool keepAlive)
{
    m_builder.setInsertInto(m_module.get());

    IRFunc* func = m_builder.createFunc();
    func->setFullType(m_builder.getFuncType(0, nullptr, m_builder.getVoidType()));
    m_builder.addNameHintDecoration(func, UnownedStringSlice(name));

    // A function needs an entry block with a terminator to be well-formed.
    m_builder.setInsertInto(func);
    m_builder.emitBlock();
    m_builder.emitReturn();

    if (keepAlive)
    {
        m_builder.addKeepAliveDecoration(func);
    }

    return func;
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
