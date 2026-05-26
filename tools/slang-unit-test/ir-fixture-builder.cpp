// ir-fixture-builder.cpp
//
// Implementation of `IRFixtureBuilder` (Approach B in
// `docs/design/ir-pass-unit-testing.md`).

#include "ir-fixture-builder.h"

#include "../../include/slang-com-ptr.h"
#include "../../include/slang.h"
#include "../../source/slang/slang-ir-insts.h"
#include "../../source/slang/slang-ir.h"
#include "../../source/slang/slang-session.h"

namespace Slang
{

namespace
{

// Same global-session caching strategy as `compileSlangToIR` —
// constructing an IGlobalSession is expensive, and the per-session
// state we touch (target list, capability sets) doesn't matter
// for hand-built IR.
ComPtr<slang::IGlobalSession> _getOrCreateGlobalSession()
{
    static ComPtr<slang::IGlobalSession> s_globalSession;
    if (!s_globalSession)
    {
        slang_createGlobalSession(SLANG_API_VERSION, s_globalSession.writeRef());
    }
    return s_globalSession;
}

// `IRModule::create` requires a `Slang::Session*` (the internal
// type), not a `slang::ISession*`. The path to an internal Session
// is: ISession -> Linkage (internal) -> Session. We obtain Linkage
// by static_cast'ing the public ISession, and ask Linkage for its
// owning Session via `getSessionImpl()`.
ComPtr<slang::ISession> _createTestSession(slang::IGlobalSession* globalSession)
{
    slang::TargetDesc targetDesc = {};
    targetDesc.format = SLANG_SPIRV;
    targetDesc.profile = globalSession->findProfile("spirv_1_5");

    slang::SessionDesc sessionDesc = {};
    sessionDesc.targetCount = 1;
    sessionDesc.targets = &targetDesc;

    ComPtr<slang::ISession> session;
    globalSession->createSession(sessionDesc, session.writeRef());
    return session;
}

} // namespace

struct IRFixtureBuilder::Impl
{
    ComPtr<slang::ISession> sessionCom;
    Session* session = nullptr;
    RefPtr<IRModule> module;
    // The IRBuilder is heap-allocated so we can return an IRFunc*
    // pointing into the live module without exposing the builder
    // to the caller.
    IRBuilder builder;
    // Tracks whether the current insertion point has been "sealed"
    // by an emitReturn. Subsequent emits without a new function/
    // block are rejected via a SLANG_ASSERT.
    bool currentBlockSealed = true;
};

IRFixtureBuilder::IRFixtureBuilder()
    : m_impl(new Impl())
{
    auto globalSession = _getOrCreateGlobalSession();
    m_impl->sessionCom = _createTestSession(globalSession);
    // The internal `Slang::Session` lives behind the public
    // `slang::ISession` ComPtr. The cast goes via `Slang::Linkage`
    // because `Linkage` is the concrete subclass implementing
    // `ISession`, and `Linkage::getSessionImpl()` returns the
    // owning `Session*`.
    auto linkage = static_cast<Linkage*>(m_impl->sessionCom.get());
    m_impl->session = linkage->getSessionImpl();
    m_impl->module = IRModule::create(m_impl->session);
    m_impl->builder = IRBuilder(m_impl->module.get());
}

IRFixtureBuilder::~IRFixtureBuilder()
{
    delete m_impl;
}

IRFunc* IRFixtureBuilder::addVoidFunction(bool keepAlive)
{
    auto& b = m_impl->builder;
    b.setInsertInto(m_impl->module.get());

    // void()
    IRType* voidType = b.getVoidType();
    IRType* funcType = b.getFuncType(0, nullptr, voidType);

    // `createFunc` allocates the function and inserts it under
    // the module's module-inst (via `addGlobalValue`). It does
    // *not* set the function's type — we set it explicitly so
    // later passes that look at `getDataType()` see something
    // sensible.
    IRFunc* func = b.createFunc();
    func->setFullType(funcType);

    // Add an entry block + position the insertion point inside it.
    b.setInsertInto(func);
    b.emitBlock();

    if (keepAlive)
    {
        b.addKeepAliveDecoration(func);
    }

    m_impl->currentBlockSealed = false;
    return func;
}

void IRFixtureBuilder::emitReturnVoid()
{
    SLANG_ASSERT(!m_impl->currentBlockSealed);
    m_impl->builder.emitReturn();
    m_impl->currentBlockSealed = true;
}

IRFixture IRFixtureBuilder::build()
{
    IRFixture f;
    f.m_session = m_impl->sessionCom;
    f.m_ownedModule = m_impl->module;
    f.m_irModule = m_impl->module.get();
    // m_iModule / m_slangModule stay null — there is no public
    // Slang module wrapper for a hand-built IRModule.
    return f;
}

} // namespace Slang
