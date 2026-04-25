// unit-test-ir-fixture-builder.cpp
//
// Reference test for Approach B (`IRFixtureBuilder`). The test
// exists to demonstrate the pattern, not just to verify the
// builder API: it picks a pass (`eliminateDeadCode`) whose value
// is best demonstrated on a hand-crafted module, runs the pass,
// and asserts the resulting IR shape.
//
// Why DCE on a hand-built module:
//
//   * Global DCE removes top-level instructions that are
//     unreferenced and that don't carry a `[KeepAlive]` (or
//     equivalent) decoration. The frontend always emits at least
//     an entry-point shim that holds everything alive, which makes
//     it surprisingly hard to drive DCE on real Slang source —
//     the very thing that pins the pass's behaviour is what the
//     frontend works hard to avoid producing.
//
//   * With the builder we can place exactly two functions in a
//     module, mark one with `[KeepAlive]`, and assert the other
//     gets removed. The IR shape is the assertion: there is no
//     surrounding shader scaffolding.
//
// This is the canonical justification for the B path in the
// design doc — see `docs/design/ir-pass-unit-testing.md`.

#include "../../source/slang/slang-ir-dce.h"
#include "../../source/slang/slang-ir.h"
#include "ir-fixture-builder.h"
#include "ir-fixture.h"
#include "unit-test/slang-unit-test.h"

using namespace Slang;

SLANG_UNIT_TEST(irFixtureBuilderDCEDropsUnreferencedFunction)
{
    IRFixtureBuilder b;
    // Two top-level void functions: `kept` carries [KeepAlive],
    // `dead` does not. Neither is referenced from anywhere else.
    IRFunc* kept = b.addVoidFunction(/*keepAlive*/ true);
    b.emitReturnVoid();
    IRFunc* dead = b.addVoidFunction(/*keepAlive*/ false);
    b.emitReturnVoid();
    SLANG_CHECK(kept != nullptr);
    SLANG_CHECK(dead != nullptr);

    IRFixture f = b.build();
    SLANG_CHECK_ABORT(f.module() != nullptr);
    SLANG_CHECK(f.countInsts(kIROp_Func) == 2);

    // Run global DCE. The default options keep `[KeepAlive]`-
    // decorated insts and global params alive; everything else
    // is removed if unreferenced.
    bool changed = eliminateDeadCode(f.module());
    SLANG_CHECK(changed);

    // Exactly one IRFunc should remain — the `[KeepAlive]` one.
    // We don't compare against the `kept` pointer because DCE may
    // mutate the module-inst child list; we assert the *shape*
    // instead, which is what the pass actually contracts.
    SLANG_CHECK(f.countInsts(kIROp_Func) == 1);
    IRInst* survivor = f.findFirstInst(kIROp_Func);
    SLANG_CHECK_ABORT(survivor != nullptr);
    // The survivor should carry the KeepAlive decoration.
    SLANG_CHECK(f.countInsts(kIROp_KeepAliveDecoration) >= 1);
}
