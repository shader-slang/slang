// internals-test-env.h
//
// Helpers for unit tests that exercise `source/slang` internals directly.
//
// These tests run in the `slang-internals-test` executable rather than the
// `slang-unit-test` plugin, because reaching internal symbols requires linking
// the compiler statically (`SLANG_LIB_TYPE=STATIC`). See
// `docs/design/internals-unit-testing.md`.

#ifndef SLANG_TOOLS_INTERNALS_TEST_ENV_H
#define SLANG_TOOLS_INTERNALS_TEST_ENV_H

#include "core/slang-list.h"
#include "core/slang-string.h"
#include "slang-com-ptr.h"
#include "slang.h"
#include "slang/slang-ir-insts.h"
#include "slang/slang-ir.h"
#include "unit-test/slang-unit-test.h"

namespace Slang
{

class ASTBuilder;
class Linkage;
class Module;
class Session;

/// Per-test environment holding a `Linkage` and the internal compiler handles
/// tests need. Construct one per test.
///
/// Constructing an `InternalsTestEnv` is cheap (on the order of microseconds)
/// because the expensive work — loading the core module — happens once when the
/// process creates its global session, and that session is shared with every
/// test through `UnitTestContext::slangGlobalSession`. Prefer one environment
/// per test over sharing a single one: the isolation is essentially free, and
/// sharing mutable compiler state between tests is what makes a suite
/// order-dependent.
class InternalsTestEnv
{
public:
    /// Create a session (and therefore a `Linkage`) from the global session
    /// that the test harness passes in.
    explicit InternalsTestEnv(UnitTestContext* context);

    /// Return the internal `Session`, which IR construction needs in order to
    /// create an `IRModule`.
    Session* getSession() const;

    /// Return the `ASTBuilder` owned by this environment's `Linkage`, used to
    /// construct types and other AST values directly.
    ASTBuilder* getASTBuilder() const;

    /// Parse and semantically check `source`, returning the internal `Module`
    /// so a test can inspect the checked AST or the IR the frontend produced.
    /// This runs the frontend only; no target code is generated.
    ///
    /// `moduleName` must be unique within a single environment. Loading two
    /// modules under the same name returns the first one from the module cache,
    /// which would silently make the second test case a no-op.
    ///
    /// Returns null if the source failed to compile; pass `outDiagnostics` to
    /// recover the compiler's message in that case.
    Module* checkModuleFromSource(
        const char* moduleName,
        const char* source,
        String* outDiagnostics = nullptr);

private:
    ComPtr<slang::ISession> m_session;
    Linkage* m_linkage = nullptr;
    List<String> m_usedModuleNames;
};

/// Builder for hand-crafted `IRModule`s.
///
/// IR passes are best tested on shapes the frontend would never emit. Global
/// dead-code elimination, for example, removes unreferenced top-level
/// instructions that carry no `[KeepAlive]` decoration — but a frontend-produced
/// module gives no way to express "this one survives, that one does not",
/// because keep-alive marking happens later in the pipeline. Building the module
/// by hand makes the pass's contract directly expressible: place two functions,
/// decorate exactly one, and assert which survives.
class IRFixtureBuilder
{
public:
    explicit IRFixtureBuilder(Session* session);

    /// Add a top-level function of type `void()` containing a single block that
    /// immediately returns, and give it a name hint so assertions can refer to
    /// it by name. When `keepAlive` is true the function is decorated with
    /// `[KeepAlive]`, which marks it as a root for liveness-based passes.
    IRFunc* addVoidFunction(const char* name, bool keepAlive);

    /// Add a top-level function of type `void()` whose body calls `callee` and
    /// then returns. Used to build the reachability half of a liveness
    /// contract: a callee with no decoration of its own should survive as long
    /// as something live refers to it.
    IRFunc* addVoidFunctionCalling(const char* name, bool keepAlive, IRFunc* callee);

    IRModule* getModule() const { return m_module.get(); }

    /// Count top-level instructions with the given opcode.
    Int countGlobalInsts(IROp op) const;

    /// Return the name hints of the top-level functions currently in the
    /// module. Prefer asserting on these over a bare count: a failure then
    /// reports which function unexpectedly survived or vanished, rather than
    /// only that a number was wrong.
    List<String> getFunctionNames() const;

    /// Disassemble the module. Intended for diagnosing a failing test, not for
    /// writing assertions against — the dump format is not a stable contract.
    String dump() const;

private:
    // Declaration order matters: the constructor initializes `m_builder` from
    // `m_module.get()`, and members are initialized in declaration order rather
    // than in the order the initializer list happens to name them. Declaring
    // `m_builder` first would bind it to an uninitialized `RefPtr`, with no
    // compiler diagnostic.
    RefPtr<IRModule> m_module;
    IRBuilder m_builder;
};

} // namespace Slang

#endif // SLANG_TOOLS_INTERNALS_TEST_ENV_H
