// ir-fixture.cpp
//
// Implementation of `IRFixture::compileSlangToIR` (Option A in
// `docs/design/ir-pass-unit-testing.md`).
//
// IR traversal is implemented manually using the public linked-list
// fields of `IRInst` (`m_decorationsAndChildren.first`, `next`)
// rather than the iterator helpers, because the iterator and walker
// helpers in `slang-ir.cpp` are not exported from
// `libslang-compiler.dylib`. See issue #10950.

#include "ir-fixture.h"

#include "../../include/slang-com-ptr.h"
#include "../../include/slang.h"
#include "../../source/slang/slang-ir.h"
#include "../../source/slang/slang-module.h"

namespace Slang
{

namespace
{

// One global session per test process — `IGlobalSession` is heavy
// to create and the per-session state we care about (IR module
// shape) doesn't depend on global-session config.
ComPtr<slang::IGlobalSession> _getOrCreateGlobalSession()
{
    static ComPtr<slang::IGlobalSession> s_globalSession;
    if (!s_globalSession)
    {
        slang_createGlobalSession(SLANG_API_VERSION, s_globalSession.writeRef());
    }
    return s_globalSession;
}

// Compose a fresh `ISession` with a default SPIRV target. Target
// choice doesn't affect IR-before-lowering shape, but `createSession`
// requires at least one target descriptor.
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

// Visit `inst` and every transitive child (every entry of its
// combined decorations-and-children list, recursively). `visitor`
// is called for each visited instruction; if it returns true the
// walk short-circuits. Implemented with an explicit work stack to
// avoid stack overflow on deep IR trees and to avoid relying on
// any out-of-line walker symbol.
template<typename Fn>
bool _walk(IRInst* root, Fn&& visitor)
{
    if (!root)
        return false;
    List<IRInst*> work;
    work.add(root);
    while (work.getCount())
    {
        IRInst* cur = work.getLast();
        work.removeLast();
        if (visitor(cur))
            return true;
        // Walk the linked list of decorations + children using the
        // public `first` field and `next` pointer. Equivalent to
        // `cur->getChildren()` but uses no out-of-line accessor.
        for (IRInst* sub = cur->m_decorationsAndChildren.first; sub; sub = sub->next)
        {
            work.add(sub);
        }
    }
    return false;
}

} // namespace

IRFixture::IRFixture() = default;
IRFixture::~IRFixture() = default;
IRFixture::IRFixture(IRFixture&&) noexcept = default;
IRFixture& IRFixture::operator=(IRFixture&&) noexcept = default;

bool IRFixture::isValid() const
{
    return m_irModule != nullptr;
}

Index IRFixture::countInsts(IROpCode op) const
{
    if (!m_irModule)
        return 0;
    Index n = 0;
    // Start the walk at the module's root inst — every global
    // instruction lives under `getModuleInst()` (an inline accessor
    // that returns a public field).
    _walk(
        m_irModule->getModuleInst(),
        [&](IRInst* cur)
        {
            if ((int)cur->getOp() == op)
                ++n;
            return false; // never short-circuit
        });
    return n;
}

IRInst* IRFixture::findFirstInst(IROpCode op) const
{
    if (!m_irModule)
        return nullptr;
    IRInst* found = nullptr;
    _walk(
        m_irModule->getModuleInst(),
        [&](IRInst* cur)
        {
            if ((int)cur->getOp() == op)
            {
                found = cur;
                return true;
            }
            return false;
        });
    return found;
}

IRFixture compileSlangToIR(const char* source, const char* entryPointName)
{
    IRFixture f;

    auto globalSession = _getOrCreateGlobalSession();
    if (!globalSession)
    {
        f.m_errorMessage = "failed to create global session";
        return f;
    }

    auto session = _createTestSession(globalSession);
    if (!session)
    {
        f.m_errorMessage = "failed to create session";
        return f;
    }
    f.m_session = session;

    ComPtr<slang::IBlob> diagnostics;
    f.m_iModule = session->loadModuleFromSourceString(
        "ir_fixture_module",
        nullptr,
        source,
        diagnostics.writeRef());

    if (diagnostics)
    {
        f.m_errorMessage = String((const char*)diagnostics->getBufferPointer());
    }

    if (!f.m_iModule)
        return f;

    // (`entryPointName` is reserved for future use — currently the
    // module-level compile already runs the IR-pre-lowering passes
    // we want, regardless of entry-point selection.)
    (void)entryPointName;

    // Cast the public IModule wrapper to the internal Module class
    // and reach for the IRModule via the inline accessor.
    f.m_slangModule = static_cast<Module*>(f.m_iModule.get());
    f.m_irModule = f.m_slangModule->getIRModule();
    return f;
}

} // namespace Slang
