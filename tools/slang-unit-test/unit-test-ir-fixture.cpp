// unit-test-ir-fixture.cpp
//
// Self-tests for the IR fixture itself. Verifies that the
// `compileSlangToIR` helper from `ir-fixture.h` produces a usable
// `IRModule*` for representative inputs and that the helper
// surface (countInsts, findFirstInst, error reporting) behaves
// as documented in `docs/design/ir-pass-unit-testing.md`.
//
// These tests are the foundation for IR-pass unit tests in
// follow-up files: every pass test starts by compiling a snippet
// through this helper, so it must work for the obvious shapes.

// `slang-ir.h` already pulls in the `kIROp_*` enum and the IRInst
// definition we need for `getOp()`. We do not include
// `slang-ir-insts.h` because that drags in the AST/capability
// chain unnecessarily and the tests don't reference any concrete
// `IR*` instruction subtype directly.
#include "../../source/slang/slang-ir.h"
#include "ir-fixture.h"
#include "unit-test/slang-unit-test.h"

using namespace Slang;

// Smoke test: a trivial library function compiles to a non-empty
// IRModule with one IRFunc. This is the hello-world of the
// fixture; if this fails everything else is moot.
SLANG_UNIT_TEST(irFixtureCompilesTrivialFunction)
{
    auto f = compileSlangToIR(R"(
        int identity(int x) { return x; }
    )");
    SLANG_CHECK_ABORT(f.module() != nullptr);
    SLANG_CHECK(f.errorMessage().getLength() == 0);
    SLANG_CHECK(f.isValid());
    // At least one IRFunc somewhere in the module.
    SLANG_CHECK(f.countInsts(kIROp_Func) >= 1);
}

// `findFirstInst` should locate the first instruction of a given
// opcode in document order. We compile two functions and check that
// `findFirstInst(kIROp_Func)` returns a non-null IRFunc, and that
// it is one of the functions visible via the recursive walk.
SLANG_UNIT_TEST(irFixtureFindFirstInstReturnsAFunction)
{
    auto f = compileSlangToIR(R"(
        int alpha(int x) { return x + 1; }
        int beta (int x) { return x * 2; }
    )");
    SLANG_CHECK_ABORT(f.module() != nullptr);

    Index funcCount = f.countInsts(kIROp_Func);
    SLANG_CHECK(funcCount >= 2);

    IRInst* first = f.findFirstInst(kIROp_Func);
    SLANG_CHECK(first != nullptr);
    SLANG_CHECK(first->getOp() == kIROp_Func);

    // An opcode value that is out of range cannot match any
    // instruction; findFirstInst must return nullptr rather than
    // returning a stale pointer.
    SLANG_CHECK(f.findFirstInst((IROpCode)-1) == nullptr);
}

// `countInsts` must traverse the full instruction tree (recursing
// into function bodies), not just direct children of the module.
// We test this by compiling a function with multiple `IRReturn`
// instructions in its body and asserting the count >= 2.
SLANG_UNIT_TEST(irFixtureCountInstsWalksRecursively)
{
    auto f = compileSlangToIR(R"(
        int branchy(int x) {
            if (x > 0) return 1;
            return 0;
        }
    )");
    SLANG_CHECK_ABORT(f.module() != nullptr);

    // The two `return` statements lower to two distinct IRReturn
    // instructions inside the function body. countInsts must reach
    // them through the function's blocks.
    Index returns = f.countInsts(kIROp_Return);
    SLANG_CHECK(returns >= 2);
}

// Compile error: malformed source must populate `errorMessage()`
// and leave `module()` null. Tests should be able to surface the
// diagnostic for debugging.
SLANG_UNIT_TEST(irFixtureSurfacesCompileErrors)
{
    auto f = compileSlangToIR(R"(
        // Missing close brace + undeclared identifier.
        int broken( {
            return undefined_variable;
    )");
    SLANG_CHECK(f.module() == nullptr);
    SLANG_CHECK(!f.isValid());
    // The diagnostic should be non-empty and mention something
    // recognisable about a parse error.
    SLANG_CHECK(f.errorMessage().getLength() > 0);
}

// Spike for #10950: confirms that an internal symbol annotated with
// SLANG_INTERNAL_TEST_API actually links from the test plugin. We
// reach for `IRModule::create` because it is the entry point Option
// B's IRFixtureBuilder will need anyway. If this test fails to link,
// the visibility plumbing is wrong and the rest of the B work is
// blocked.
SLANG_UNIT_TEST(irFixtureSlangInternalTestApiVisibility)
{
    auto f = compileSlangToIR("int identity(int x) { return x; }");
    SLANG_CHECK_ABORT(f.module() != nullptr);
    auto session = f.module()->getSession();
    SLANG_CHECK(session != nullptr);
    auto fresh = IRModule::create(session);
    SLANG_CHECK(fresh != nullptr);
    SLANG_CHECK(fresh->getModuleInst() != nullptr);
    // The fresh module is independent from the compiled one.
    SLANG_CHECK(fresh.get() != f.module());
}

// Repeated calls to compileSlangToIR within one test process must
// each produce an independent fixture. This pins the contract that
// the cached global session is shared but each compile is fresh.
SLANG_UNIT_TEST(irFixtureCompilesAreIndependent)
{
    auto a = compileSlangToIR("int a(int x) { return x; }");
    auto b = compileSlangToIR("int b(int x) { return x + 1; }");
    SLANG_CHECK_ABORT(a.module() != nullptr);
    SLANG_CHECK_ABORT(b.module() != nullptr);
    SLANG_CHECK(a.module() != b.module());
    // Each module independently contains at least one IRFunc.
    SLANG_CHECK(a.countInsts(kIROp_Func) >= 1);
    SLANG_CHECK(b.countInsts(kIROp_Func) >= 1);
    SLANG_CHECK(a.findFirstInst(kIROp_Func) != b.findFirstInst(kIROp_Func));
}
