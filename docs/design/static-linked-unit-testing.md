# Statically Linked Unit Testing

`slang-static-unit-test` is an executable that links the compiler statically so
that unit tests can call non-exported `source/slang` entry points directly. This
document specifies its scope, the conditions under which it is built, and how to
add tests to it.

## Scope

Slang has three kinds of test. Two of them cannot reach compiler internals.

`.slang` tests run by `slang-test` cover language behaviour and target codegen
well, but they can only assert on what the compiler *reports or emits* —
diagnostics and generated code. They cannot look at compiler state.

`slang-unit-test` tests are C++ and can call into the library directly, but they
are hosted in a shared library that `slang-test` loads at runtime. That restricts
them to symbols exported from `libslang-compiler`. Non-exported declarations in
`source/slang` — IR passes, AST utilities, mangling, serialization — carry no
export annotation and are compiled with hidden visibility, so a plugin test
cannot link against `eliminateDeadCode` or `IRBuilder`. This is why existing unit
tests that want to reach those entry points go through the public C API and
construct a full `Session` even when the thing under test is much smaller.

Some contracts are impractical to test either way. Global dead-code elimination
removes an unreferenced top-level function that carries no `[KeepAlive]`
decoration. The frontend keeps entry points and their transitive callees alive,
which is the condition the pass discriminates on, so a `.slang` test cannot
readily construct a module in which the distinction is observable.

## Design

The target links the compiler statically. Static linkage resolves non-exported
symbols at link time, so no export annotation or change to `source/slang` is
required: a test includes `slang/slang-ir-dce.h` and calls `eliminateDeadCode`
directly.

It is defined only when `SLANG_LIB_TYPE=STATIC`, with `SLANG_ENABLE_TESTS` and
`SLANG_ENABLE_SLANG_RHI` both on. Under a shared build the link would fail by
design, so the target simply does not exist there. The RHI condition is not a
dependency of these tests -- they never touch the RHI -- it is inherited from
the `unit-test` object library they share with `slang-unit-test`, whose own
guard requires it.

### Separate executable

These tests are a separate executable rather than a statically linked
`slang-unit-test`, because the plugin arrangement is incompatible with static
linkage.

`slang-unit-test` is a `MODULE` that `slang-test` loads with `dlopen`. The host
creates compiler objects and passes them across that boundary — see
`UnitTestContext::slangGlobalSession`. Linking the compiler statically into both
would give the host and the plugin one copy each, so an object created by one
would be consumed by the other, and internal downcasts, RTTI identity, the IR
opcode tables and allocator state would all diverge.

A separate executable holds one process, one copy of the compiler, and no
compiler objects crossing a boundary. A static configuration also produces no
shared library, so the same translation units are never linked into two
artifacts.

The test registry is a function-local static in `_getTestModule()`
(`tools/unit-test/slang-unit-test.cpp`), giving each linked image its own. The
two suites therefore register independently, and a test name may exist in both
without conflict; a duplicate name within a single binary is a link error, as
for any other symbol.

## Writing a test

Tests use `SLANG_UNIT_TEST` and `SLANG_CHECK` exactly as they do in
`slang-unit-test`; registration goes through the same list in `tools/unit-test`.
Two helpers in `static-unit-test-env.h` cover the common needs.

`StaticUnitTestEnv` owns a `Linkage` and exposes the internal handles:

```cpp
SLANG_UNIT_TEST(astVectorTypesAreDeduplicated)
{
    StaticUnitTestEnv env(unitTestContext);
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
    StaticUnitTestEnv env(unitTestContext);
    IRFixtureBuilder builder(env.getSessionImpl());

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

The builder covers the shapes the existing tests needed. Each exists to make one
contract observable, so the list doubles as a map of what is already testable:

| Method | Shape it builds |
| --- | --- |
| `addVoidFunction` | a top-level `void()` function, optionally `[KeepAlive]` |
| `addVoidFunctionCalling` | the same, with a call to a given callee, for reachability |
| `addVoidFunctionWithUnusedBlockParam` | two blocks, the second taking a parameter nothing reads |
| `addExportedVoidFunction` | a function carrying `[Export]` and nothing else |
| `addVoidFunctionWithLayout` | a function carrying an empty layout decoration |
| `addGlobalParam` | an unreferenced `GlobalParam` |
| `addLiveWeakUseOf` | a `[KeepAlive]` `WeakUse` whose operand is a given function |
| `addOptimizableStructWithUnusedField` | an `[OptimizableType]` struct with one unread field |

`keepAlive` is a parameter only where both settings are meaningful. The export,
layout and global-parameter fixtures exist precisely to be unreferenced and
otherwise undecorated, so that whether they survive turns on the option under
test rather than on a decoration the fixture added.

To test against AST or IR that the frontend actually produced, use
`StaticUnitTestEnv::checkModuleFromSource`, which runs the frontend only:

```cpp
Module* module = env.checkModuleFromSource("myTest", "struct Point { float x; }\n");
SLANG_CHECK_ABORT(module != nullptr);               // null if the source failed to compile
ModuleDecl* moduleDecl = module->getModuleDecl();   // checked AST
IRModule* irModule = module->getIRModule();         // frontend-produced IR
```

`checkModuleFromSource` returns null when the source fails to compile, so the
result is checked before use. Pass `outDiagnostics` to recover the compiler's
message in that case.

Each module name must be unique within one environment, or the module cache
returns the first module and the later case silently becomes a no-op. The helper
asserts on this.

When testing a pass against frontend-produced IR, first establish what the IR
looks like at that stage. A checked module has not yet been through linking and
specialization, so it does not carry the keep-alive marking that later stages
add — running a liveness-based pass on it can remove more than expected. Confirm
the invariants of the stage you are working with before encoding them in
assertions.

## Choosing a test kind

| Testing | Use |
| --- | --- |
| Language behaviour, target codegen, diagnostics | `.slang` test under `tests/` |
| Public API behaviour, reflection, compilation requests | `slang-unit-test` |
| An IR pass contract, AST invariants, mangling, checker output | `slang-static-unit-test` |

`slang-static-unit-test` covers what the other two cannot reach. It complements
them rather than replacing either: a test that can be written against the public
API belongs in `slang-unit-test`, which additionally exercises the shared library
as it ships.

## Building and running

```bash
cmake --preset default -DSLANG_LIB_TYPE=STATIC
cmake --build build --config Debug --target slang-static-unit-test
./build/Debug/bin/slang-static-unit-test

# Run a subset while iterating; the argument is a substring filter.
./build/Debug/bin/slang-static-unit-test irDeadCode
```
