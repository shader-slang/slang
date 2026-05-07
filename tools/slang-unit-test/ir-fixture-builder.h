// ir-fixture-builder.h
//
// Approach B from `docs/design/ir-pass-unit-testing.md`:
// build an IRModule directly via `IRBuilder` so tests can construct
// IR shapes the frontend would never emit (orphan blocks,
// hand-crafted CFGs, intentionally-dead instructions, etc.).
//
// Symbols on `IRBuilder` and the small set of pass entry points
// the tests need are decorated with `SLANG_INTERNAL_TEST_API` so
// they are reachable from the test plugin. See #10950.

#ifndef SLANG_TOOLS_UNIT_TEST_IR_FIXTURE_BUILDER_H
#define SLANG_TOOLS_UNIT_TEST_IR_FIXTURE_BUILDER_H

#include "../../source/core/slang-basic.h"
#include "ir-fixture.h"

namespace Slang
{

class Session;
struct IRBuilder;
struct IRType;
struct IRFunc;
struct IRBlock;

/// Fluent builder that constructs an `IRModule` directly via
/// `IRBuilder`. Designed for tests that want to exercise a pass
/// on a hand-crafted IR shape.
///
/// Lifetime: the builder owns the underlying `IRBuilder`/`IRModule`
/// until `build()` is called, at which point ownership of the
/// module is transferred to the returned `IRFixture`.
///
/// The DSL is intentionally tiny — only what the first reference
/// test in `unit-test-ir-fixture-builder.cpp` actually exercises.
/// Add primitives as new tests need them; do not pre-emptively
/// expand the surface.
class IRFixtureBuilder
{
public:
    IRFixtureBuilder();
    ~IRFixtureBuilder();

    IRFixtureBuilder(IRFixtureBuilder const&) = delete;
    IRFixtureBuilder& operator=(IRFixtureBuilder const&) = delete;

    /// Add a top-level void-returning function and position the
    /// internal insertion point at a fresh entry block in it.
    /// Subsequent emit calls add instructions to that block.
    /// `keepAlive=true` attaches a `[KeepAlive]` decoration so DCE
    /// won't delete the function.
    IRFunc* addVoidFunction(bool keepAlive);

    /// Emit `return;` (void) into the current block. After this
    /// the current block is sealed; further emit calls without a
    /// new function/block will fail (no auto-fallback).
    void emitReturnVoid();

    /// Finish building and hand off the module to an `IRFixture`.
    /// After this call the builder is exhausted; reusing it is a
    /// programming error.
    IRFixture build();

private:
    struct Impl;
    Impl* m_impl;
};

} // namespace Slang

#endif // SLANG_TOOLS_UNIT_TEST_IR_FIXTURE_BUILDER_H
