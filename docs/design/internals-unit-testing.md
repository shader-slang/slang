# Unit-testing compiler internals

This document describes `slang-internals-test`: what it is for, why it is a
separate executable, and how to add tests to it.

## The problem

Slang has two kinds of test today.

`.slang` tests run by `slang-test` cover language behaviour and target codegen
well, but they can only assert on what the compiler *reports or emits* —
diagnostics and generated code. They cannot look at compiler state.

`slang-unit-test` tests are C++ and can call into the library directly, but they
are hosted in a shared library that `slang-test` loads at runtime. That restricts
them to symbols exported from `libslang-compiler`. Internal declarations in
`source/slang` — IR passes, AST utilities, mangling, serialization — carry no
export annotation and are compiled with hidden visibility, so a plugin test
cannot link against `eliminateDeadCode` or `IRBuilder`. This is why existing unit
tests that want to reach compiler internals go through the public C API and
construct a full `Session` even when the thing under test is much smaller.

Some behaviour is impractical to test either way. Consider the contract of global
dead-code elimination: *an unreferenced top-level function without a
`[KeepAlive]` decoration is removed*. The frontend deliberately keeps entry
points and their callees alive, which is precisely the condition the pass
discriminates on, so a `.slang` test cannot easily construct a module where the
distinction is observable.

## The approach

`slang-internals-test` is an executable that links the compiler **statically**.
Static linkage resolves internal symbols at link time, so no export annotation
or source change is needed: `#include "slang/slang-ir-dce.h"` and call
`eliminateDeadCode` directly.

It is defined only when `SLANG_LIB_TYPE=STATIC`. Under a shared build the link
would fail by design, so the target simply does not exist there.

### Why a separate executable rather than static-linking `slang-unit-test`

`slang-unit-test` is a `MODULE` that `slang-test` loads with `dlopen`, and the
host creates compiler objects and passes them across that boundary — see
`UnitTestContext::slangGlobalSession`. If the compiler were linked statically
into both, the host and the plugin would each hold their own copy of it, and an
object created by one would be used by the other. Internal downcasts, RTTI
identity, the IR opcode tables, and allocator state would all diverge.

A separate executable avoids this by construction: one process, one copy of the
compiler, and no compiler objects crossing a boundary. It also means a static
configuration produces no shared library at all, so there is no question of the
same translation units being linked into two artifacts.

## Writing a test

Tests use `SLANG_UNIT_TEST` and `SLANG_CHECK` exactly as they do in
`slang-unit-test`; registration goes through the same list in `tools/unit-test`.
Two helpers in `internals-test-env.h` cover the common needs.

`InternalsTestEnv` owns a `Linkage` and exposes the internal handles:

```cpp
SLANG_UNIT_TEST(astVectorTypesAreDeduplicated)
{
    InternalsTestEnv env(unitTestContext);
    ASTBuilder* astBuilder = env.getASTBuilder();

    Type* floatType = astBuilder->getFloatType();
    IntVal* three = astBuilder->getIntVal(astBuilder->getIntType(), 3);

    // Structurally identical types must be the same object: much of the
    // compiler compares types by pointer.
    SLANG_CHECK(
        astBuilder->getVectorType(floatType, three) ==
        astBuilder->getVectorType(floatType, three));
}
```

Construct one environment per test. It is cheap — the expensive work, loading
the core module, happens once per process when the global session is created and
is shared with every test. Sharing an environment between tests saves nothing
measurable and makes the suite order-dependent.

`IRFixtureBuilder` builds an `IRModule` by hand, which is how a pass contract
becomes expressible:

```cpp
SLANG_UNIT_TEST(irDeadCodeEliminationRemovesUnreferencedFunction)
{
    InternalsTestEnv env(unitTestContext);
    IRFixtureBuilder builder(env.getSession());

    builder.addVoidFunction("keptFunc", /* keepAlive: */ true);
    builder.addVoidFunction("deadFunc", /* keepAlive: */ false);

    SLANG_CHECK(eliminateDeadCode(builder.getModule()));

    List<String> names = builder.getFunctionNames();
    SLANG_CHECK_ABORT(names.getCount() == 1);
    SLANG_CHECK(names[0] == "keptFunc");
}
```

Prefer asserting on names over counts. A failure then reports *which* function
behaved unexpectedly rather than only that a number was wrong.
`IRFixtureBuilder::dump()` exists for diagnosing a failure interactively; do not
write assertions against the dump text, as its format is not a stable contract.

To test against AST or IR that the frontend actually produced, use
`InternalsTestEnv::checkModuleFromSource`, which runs the frontend only:

```cpp
Module* module = env.checkModuleFromSource("myTest", "struct Point { float x; }\n");
ModuleDecl* moduleDecl = module->getModuleDecl();   // checked AST
IRModule* irModule = module->getIRModule();         // frontend-produced IR
```

Each module name must be unique within one environment, or the module cache
returns the first module and the later case silently becomes a no-op. The helper
asserts on this.

When testing a pass against frontend-produced IR, first establish what the IR
looks like at that stage. A checked module has not yet been through linking and
specialization, so it does not carry the keep-alive marking that later stages
add — running a liveness-based pass on it can remove more than expected. Confirm
the invariants of the stage you are working with before encoding them in
assertions.

## Which kind of test to write

| Testing | Use |
| --- | --- |
| Language behaviour, target codegen, diagnostics | `.slang` test under `tests/` |
| Public API behaviour, reflection, compilation requests | `slang-unit-test` |
| An IR pass contract, AST invariants, mangling, checker output | `slang-internals-test` |

`slang-internals-test` is for what the other two cannot reach. It is not a
replacement for either, and existing tests have no reason to move into it.

## Building and running

```bash
cmake --preset default -DSLANG_LIB_TYPE=STATIC
cmake --build build --config Debug --target slang-internals-test
./build/Debug/bin/slang-internals-test

# Run a subset while iterating; the argument is a substring filter.
./build/Debug/bin/slang-internals-test irDeadCode
```
