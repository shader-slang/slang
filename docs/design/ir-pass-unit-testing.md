# IR Pass Unit Testing

A fixture-style harness for writing unit tests against individual
IR passes, as an alternative to the `slang-test` end-to-end
shader tests.

This document describes what the harness supports and how to use
it. It complements `docs/design/ir.md` (the IR's design) and
`docs/design/ir-instruction-definition.md` (how new IR
instructions are added).

## Why

End-to-end `.slang` shader tests are the only way to test IR
passes today. They couple every test to the full Slang frontend,
take tens of milliseconds per minimal compile, force tests to
construct elaborate Slang source for passes that only care about
a 5-instruction shape, and read as Slang code rather than as IR
preconditions. The fixture in this document removes those
constraints for tests that don't need an end-to-end compile.

## Constructing an IRFixture

A test starts by getting an `IRFixture` — a move-only handle that
owns an `IRModule*` plus the session that produced it. There are
two constructors.

### From a Slang source string

```cpp
#include "ir-fixture.h"

SLANG_UNIT_TEST(myTest)
{
    auto f = compileSlangToIR(R"(
        int identity(int x) { return x; }
    )");
    SLANG_CHECK_ABORT(f.module() != nullptr);
    SLANG_CHECK(f.errorMessage().getLength() == 0);
    SLANG_CHECK(f.countInsts(kIROp_Func) >= 1);
}
```

`compileSlangToIR` runs the source through the public Slang
frontend and returns a fixture wrapping the resulting `IRModule*`.
On compile failure `module()` is null and `errorMessage()` is
populated.

Use this constructor when the test wants realistic IR the
frontend would naturally emit.

### From a hand-built IRModule

```cpp
#include "ir-fixture-builder.h"

SLANG_UNIT_TEST(myPassTest)
{
    IRFixtureBuilder b;
    b.addVoidFunction(/*keepAlive*/ true);
    b.emitReturnVoid();
    b.addVoidFunction(/*keepAlive*/ false);
    b.emitReturnVoid();
    IRFixture f = b.build();

    eliminateDeadCode(f.module());
    SLANG_CHECK(f.countInsts(kIROp_Func) == 1);
}
```

`IRFixtureBuilder` constructs an `IRModule` directly via
`IRBuilder` and hands ownership to the returned `IRFixture` on
`build()`. The DSL is intentionally minimal — a primitive lands
when a test needs it, never speculatively.

Use this constructor when the test wants an IR shape the frontend
wouldn't naturally emit (orphan blocks, exotic phi shapes,
hand-crafted CFGs, distinguishing decoration patterns the
frontend papers over).

## Inspecting the fixture

```cpp
class IRFixture
{
public:
    /// Underlying IR module. Lives until the fixture is destructed.
    IRModule* module() const;

    /// Compiled module wrapper (compileSlangToIR only); nullptr
    /// for builder-constructed fixtures.
    Module* slangModule() const;

    /// Diagnostic text from the underlying compile, or empty.
    const String& errorMessage() const;

    /// True iff `module()` is non-null. Deeper IR-validator
    /// checks land when a test needs them.
    bool isValid() const;

    /// Count instructions of opcode `op` anywhere in the module.
    Index countInsts(IROp op) const;

    /// Return the first instruction in the module (in document
    /// order) whose opcode equals `op`, or nullptr.
    IRInst* findFirstInst(IROp op) const;
};
```

The opcode walks (`countInsts`, `findFirstInst`) traverse every
transitive child of the module's root inst using the public
`IRInstListBase::first` field and the public `IRInst::next`
pointer:

```cpp
for (IRInst* sub = cur->m_decorationsAndChildren.first;
     sub;
     sub = sub->next)
{
    work.add(sub);
}
```

This avoids the `getChildren()` and `getDecorations()` iterators,
which are inline-but-call-out-of-line in `slang-ir.cpp`.

## Running a pass against the fixture

Tests call internal pass entry points directly on
`f.module()`. The pass entry point must be reachable from the
test plugin — see "Visibility" below.

```cpp
auto f = compileSlangToIR(R"(...)");
SLANG_CHECK_ABORT(f.module() != nullptr);

bool changed = eliminateDeadCode(f.module());
SLANG_CHECK(changed);
SLANG_CHECK(f.countInsts(kIROp_LiveRangeStart) >= 2);
```

## Visibility: making internal symbols reachable

`libslang-compiler.dylib` is built with hidden symbol visibility,
so internal classes and pass entry points aren't reachable from
the test plugin by default.

A new macro `SLANG_INTERNAL_TEST_API` (defined in
`include/slang.h`) decorates internal symbols with default
visibility. It is gated on the CMake option
`SLANG_BUILD_FOR_TESTING` (default ON):

- When the option is on, the macro expands to the same
  `dllexport` / `visibility(default)` attribute used by
  `SLANG_API`. Decorated symbols become reachable from the test
  plugin.
- When the option is off (e.g. shipping builds), the macro
  expands to nothing — decorated symbols stay hidden.

The macro applies at class scope (e.g.
`struct SLANG_INTERNAL_TEST_API IRBuilder { … }` exports every
non-template member of the class) or at function scope for free
functions and statics.

Symbols decorated today:

| Symbol                   | Used by                                   |
| ------------------------ | ----------------------------------------- |
| `IRBuilder` (class)      | `IRFixtureBuilder` calls into `IRBuilder` |
| `IRUse` (class)          | `IRInst::setFullType` calls `IRUse::init` |
| `IRModule::create`       | Builder constructs a fresh module         |
| `eliminateDeadCode(...)` | Reference test invokes the pass           |

### Adding a new pass test

When a test needs a pass that isn't yet decorated, annotate the
entry point in its declaration:

```cpp
SLANG_INTERNAL_TEST_API bool eliminateDeadCode(
    IRModule* module,
    IRDeadCodeEliminationOptions const& options = {});
```

Annotate at class scope when the test needs many members of the
same class (`IRBuilder`, `IRUse`); annotate at function scope for
single free functions.

### Reusing the mechanism for other subsystems

`SLANG_INTERNAL_TEST_API` and `SLANG_BUILD_FOR_TESTING` are not
IR-specific — they are a generic visibility escape hatch. The
same three pieces stand up a fixture for any internal subsystem
(AST, mangling, serialization, …):

1. **Annotate the symbols the new fixture needs.** Class scope
   for "expose all members" (e.g.
   `class SLANG_INTERNAL_TEST_API ASTBuilder { … }`), function
   scope for individual entry points. The annotation list grows
   one entry at a time as new tests demand symbols.
2. **Make sure `slang-unit-test` can include the relevant
   headers.** Transitive includes from the subsystem's root
   header (`slang-ast-base.h`, `slang-mangle.h`, …) must resolve.
   Most paths already work because `slang-fiddle-output` and the
   capability libraries are linked in; if a new transitive
   include surfaces a missing dep, add it as a private link of
   `slang-unit-test` in `tools/CMakeLists.txt`. These deps
   propagate include directories only — they don't substitute
   for `SLANG_INTERNAL_TEST_API`.
3. **Build a fixture wrapper analogous to `IRFixture`** that
   owns the subsystem's root object and the session that
   produced it (e.g. `RefPtr<ModuleDecl>` for AST, plus a
   ComPtr to keep the underlying `Linkage` alive). Provide a
   "from source" constructor and, where it adds value, a
   "from hand-built" builder.

The same `SLANG_BUILD_FOR_TESTING` flag controls all of these.
Turning it on once exposes whatever subsystems have been
decorated.

## What's not covered

- **AST-level pass tests.** Frontend passes (semantic check, AST
  → IR lowering) can't be isolated by this fixture; they remain
  end-to-end tested.
- **Target-specific emit tests.** Backends consume IR; emit tests
  are better written as `.slang` → expected target-asm
  comparisons.
- **Pass interaction tests** (pass A → pass B → assert) are
  integration tests, not unit tests; the existing `.slang` E2E
  tests cover those.
- **Function lookup by source-level name.** Tests today identify
  functions positionally via `findFirstInst(kIROp_Func)`. Adding
  `findFuncByName` requires decorating
  `IRConstant::getStringSlice` and `IRInst::getOperands`.
- **Validator-backed `isValid()`.** Today `isValid()` only checks
  that the module pointer is non-null; calling `validateIRModule`
  needs a `DiagnosticSink` and a decoration on the validator
  entry point.

## Files

```
include/slang.h                              # SLANG_INTERNAL_TEST_API macro

source/slang/
├── slang-ir.h                               # SLANG_INTERNAL_TEST_API on IRUse, IRModule::create
├── slang-ir-insts.h                         # SLANG_INTERNAL_TEST_API on IRBuilder
└── slang-ir-dce.h                           # SLANG_INTERNAL_TEST_API on eliminateDeadCode

tools/slang-unit-test/
├── ir-fixture.{h,cpp}                       # IRFixture + compileSlangToIR
├── ir-fixture-builder.{h,cpp}               # IRFixtureBuilder
├── unit-test-ir-fixture.cpp                 # reference tests for compileSlangToIR
└── unit-test-ir-fixture-builder.cpp         # reference test for IRFixtureBuilder
```

## Build wiring

`slang-unit-test` has private link deps on `slang-fiddle-output`,
`slang-capability-defs`, and `slang-capability-lookup` so the
headers transitively pulled in by `slang-ir.h` resolve. These
propagate include directories only; symbol visibility is handled
separately by `SLANG_INTERNAL_TEST_API`.
