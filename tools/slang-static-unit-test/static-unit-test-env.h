// static-unit-test-env.h
//
// Helpers for unit tests that call non-exported `source/slang` entry points
// directly.
//
// These tests run in the `slang-static-unit-test` executable rather than the
// `slang-unit-test` plugin, because reaching non-exported symbols requires linking
// the compiler statically (`SLANG_LIB_TYPE=STATIC`). See
// `docs/design/static-linked-unit-testing.md`.

#ifndef SLANG_TOOLS_STATIC_UNIT_TEST_ENV_H
#define SLANG_TOOLS_STATIC_UNIT_TEST_ENV_H

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
/// Constructing a `StaticUnitTestEnv` is cheap (on the order of microseconds)
/// because the expensive work — loading the core module — happens once when the
/// process creates its global session, and that session is shared with every
/// test through `UnitTestContext::slangGlobalSession`. Prefer one environment
/// per test over sharing a single one: the isolation is essentially free, and
/// sharing mutable compiler state between tests is what makes a suite
/// order-dependent.
class StaticUnitTestEnv
{
public:
    /// Create a session (and therefore a `Linkage`) from the global session
    /// that the test harness passes in.
    explicit StaticUnitTestEnv(UnitTestContext* context);

    /// Return the internal `Session` — the process-wide object that owns IR
    /// allocation — which is what `IRModule::create` needs.
    ///
    /// Note that this is *not* the `slang::ISession` this environment created.
    /// Slang uses "session" for both: `Linkage` implements the public
    /// `ISession` and is per-environment, while `Session` is the global one
    /// shared by every environment in the process. This forwards to
    /// `Linkage::getSessionImpl()`, and is named after it for that reason. The
    /// per-test isolation this class provides is isolation of the `Linkage`;
    /// the `Session` behind it is deliberately shared, because loading the core
    /// module into one per test is what would make the suite slow.
    Session* getSessionImpl() const;

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
    ///
    /// The returned pointer is borrowed, not owned: the `Linkage`'s module cache owns
    /// the module, so a caller must not release it, and it stays valid only for as
    /// long as this environment does.
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

    /// Add a top-level `GlobalParam` of type `float`, with a name hint.
    ///
    /// `IRDeadCodeEliminationOptions::keepGlobalParamsAlive` — on by default — keeps
    /// global parameters even when nothing refers to them, so an unreferenced one is the
    /// smallest fixture that tells the two settings of that flag apart.
    IRGlobalParam* addGlobalParam(const char* name);

    /// Add a top-level `void()` function carrying `[Export]`, and nothing else that
    /// would keep it alive. `IRDeadCodeEliminationOptions::keepExportsAlive` is what
    /// decides its fate, so this is the smallest fixture that tells that flag apart.
    IRFunc* addExportedVoidFunction(const char* name);

    /// Add a top-level `void()` function carrying an (empty) layout decoration, and
    /// nothing else that would keep it alive. The counterpart of
    /// `addExportedVoidFunction` for `keepLayoutsAlive`.
    IRFunc* addVoidFunctionWithLayout(const char* name);

    /// Add a *live* `kIROp_WeakUse` whose single operand is `target`, and return it.
    ///
    /// `WeakUse` is hoistable, so it lands at module scope rather than inside any
    /// function body; it is decorated `[KeepAlive]` here so that it is a liveness root
    /// in its own right. That is what makes it useful as a fixture: the referent's
    /// fate then depends only on whether the operand is classified weak. A weak
    /// operand must not keep its referent alive -- if the classification were dropped,
    /// `target` would be marked live and survive.
    IRInst* addLiveWeakUseOf(IRFunc* target);

    /// Add a two-block `void()` function whose second block takes a parameter that
    /// nothing reads, passed as a branch argument from the first. DCE removes such a
    /// parameter and then reruns its work list -- the one path in the pass that
    /// iterates more than once.
    IRFunc* addVoidFunctionWithUnusedBlockParam(const char* name, bool keepAlive);

    /// Add a top-level struct with one unreferenced field, marked
    /// `[OptimizableType]` so `trimOptimizableTypes` will consider it.
    IRStructType* addOptimizableStructWithUnusedField(const char* name);

    IRModule* getModule() const { return m_module.get(); }

    /// Count the direct children of the module inst whose opcode is `op`.
    ///
    /// This is every module-level instruction -- types, global variables and witness
    /// tables as well as functions -- not only functions, so an opcode that also occurs
    /// among those will be counted there too.
    Int countGlobalInsts(IROp op) const;

    /// Return the name hints of the top-level functions currently in the module.
    /// Prefer asserting on these over a bare count: a failure then reports which
    /// function unexpectedly survived or vanished, rather than only that a number was
    /// wrong.
    ///
    /// A function with no `IRNameHintDecoration` is omitted rather than reported under
    /// a placeholder, so this matches `countGlobalInsts(kIROp_Func)` only while every
    /// function in the fixture is named. `addVoidFunction` and `addVoidFunctionCalling`
    /// always add a name hint; a fixture that builds a function by hand may not.
    List<String> getFunctionNames() const;

    /// Disassemble the module. Intended for diagnosing a failing test, not for
    /// writing assertions against — the dump format is not a stable contract.
    String dump() const;

private:
    /// Create a `void()` top-level function named `name` and open an entry block
    /// for its body, leaving the builder inserting into that block. Paired with
    /// `endVoidFunction`, which terminates it.
    IRFunc* beginVoidFunction(const char* name);

    /// Terminate the block opened by `beginVoidFunction` and apply `keepAlive`.
    void endVoidFunction(IRFunc* func, bool keepAlive);

    // Declaration order matters: the constructor initializes `m_builder` from
    // `m_module.get()`, and members are initialized in declaration order rather
    // than in the order the initializer list happens to name them. Declaring
    // `m_builder` first would bind it to an uninitialized `RefPtr`, with no
    // compiler diagnostic.
    RefPtr<IRModule> m_module;
    IRBuilder m_builder;
};

} // namespace Slang

#endif // SLANG_TOOLS_STATIC_UNIT_TEST_ENV_H
