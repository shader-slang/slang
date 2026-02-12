# Diagnostic System Conversion Guide

This document describes how to convert diagnostics from the old system (`slang-diagnostic-defs.h`) to the new Lua-based system (`slang-diagnostics.lua`).

## Essential Reading

Before working on diagnostic conversions, familiarize yourself with these key files:

### Core Definition Files

- **`source/slang/slang-diagnostics.lua`** - The main file where new diagnostics are defined. Read the comments at the top for syntax reference.
- **`source/slang/slang-diagnostics-helpers.lua`** - Helper functions that process diagnostic definitions (`span()`, `note()`, `variadic_span()`, etc.). Essential for understanding how interpolants and locations are parsed.
- **`source/slang/slang-diagnostic-defs.h`** - The old macro-based definitions. Search here to find the original diagnostic you're converting.

### Code Generation (FIDDLE System)

- **`source/slang/slang-rich-diagnostics.h`** - Header containing FIDDLE templates that generate C++ structs from Lua definitions.
- **`source/slang/slang-rich-diagnostics.cpp`** - Implementation containing FIDDLE templates that generate `toGenericDiagnostic()` methods.
- **`source/slang/slang-rich-diagnostics.h.lua`** - Lua module that bridges between `slang-diagnostics.lua` and FIDDLE templates. Contains type mappings (`cpp_type_map`).
- **`build/source/slang/fiddle/slang-rich-diagnostics.h.fiddle`** - Generated output (after build). Review this only if you need to verify your structs are generated correctly.

### Testing

- **`tools/slang-test/slang-test-main.cpp`** - Search for "diag=" to understand how diagnostic testing works.
- **`tools/slang-test/diagnostic-annotation-util.h`** - read the documentation here

### Diagnostic Sink and Rendering

- **`source/compiler-core/slang-diagnostic-sink.h`** - The `DiagnosticSink` class and `diagnose()` overloads.
- **`source/compiler-core/slang-rich-diagnostics-render.h`** - `GenericDiagnostic`, `DiagnosticSpan`, `DiagnosticNote` structures.

### Type System Reference

- **`source/slang/slang-ast-support-types.h`** - Definitions for `QualType`, `TypeExp`, and `printDiagnosticArg()` overloads.

## Summary of Diagnostic Definition Styles

### Old System (slang-diagnostic-defs.h)

```cpp
DIAGNOSTIC(code, Severity, camelCaseName, "message with $0 $1 interpolants")
```

- **Code**: A numeric ID (-1 for notes, positive integers for others)
- **Severity**: `Note`, `Warning`, `Error`, `Fatal`
- **Name**: camelCase identifier
- **Message**: String with `$0`, `$1`, etc. for interpolants
- Notes (code -1) are emitted immediately after another diagnostic

### New System (slang-diagnostics.lua)

```lua
err(
    "space case name",
    code,
    "general message describing the problem",
    span { loc = "param_name:Type", message = "specific message with ~interpolants:Type" }
)
```

- **Name**: space-separated words (not camelCase)
- **Code**: Same numeric ID
- **Diagnostic message**: A general, standalone description of the problem
- **Span message**: The specific message (originally from the old definition)
- **Interpolants**: `~name:Type` syntax with named parameters
- **Location**: Derived from the first span's `loc` parameter

## Interpolant Syntax

In the new system, interpolants use the `~name:Type` syntax:

- `~param` - String parameter (default)
- `~param:Type` - Typed parameter
- `~param.member` - Member access (e.g., `~expr.type`, `~decl.name`)
- `~param:Type.member` - Inline type with member

**Supported types:**

- `Type` - AST type pointer (`Type*`), converted via `typeToPrintableString()`
- `QualType` - Qualified type (`QualType`), converted via `qualTypeToPrintableString()`.
- `Decl` - Declaration pointer (`Decl*`), uses `getName()` when interpolated directly
- `Expr` - Expression pointer (`Expr*`)
- `Stmt` - Statement pointer (`Stmt*`)
- `Val` - Value pointer (`Val*`)
- `Name` - Name pointer (`Name*`), converted via `nameToPrintableString()`
- `int` - Integer value
- `string` - String value (default)

Parameters are automatically deduplicated. Decl types automatically use `.name` when interpolated directly.

**IMPORTANT: Never construct strings manually from types or expressions to pass to diagnostics.** Always pass the `Type*`, `Expr*`, `QualType`, etc. directly and let the diagnostic system handle the conversion. This preserves type information for richer diagnostic rendering.

## Location Syntax

- `"location"` - Plain SourceLoc variable (this is the default - do NOT specify `:SourceLoc` explicitly)
- `"location:Type"` - Typed location (Decl->getNameLoc(), Expr->loc, etc.)

Typed locations can be used as parameters in interpolations.

**Note:** If your location is a plain `SourceLoc`, just use `"location"` without any type suffix. The system defaults to `SourceLoc` for untyped locations.

it doesn't have to be called location, it can also be one of the other interpolants if the context makes it clear that that location is being used.

---

## Example: Error + Note Pattern

---

## Conversion Examples

### Example 1: `typeMismatch`

**Old:**

```cpp
DIAGNOSTIC(30019, Error, typeMismatch, "expected an expression of type '$0', got '$1'")
```

**Old emission (likely):**

```cpp
getSink()->diagnose(expr, Diagnostics::typeMismatch, expectedType, actualType);
```

**New:**

```lua
err(
    "type mismatch",
    30019,
    "type mismatch in expression",
    span { loc = "expr:Expr", message = "expected an expression of type '~expected_type:Type', got '~actual_type:QualType'" }
)
```

Note: This diagnostic uses both `Type*` and `QualType` because different callers may have different type representations. The old system accepted strings, but the new system preserves type information.

**New emission:**

```cpp
getSink()->diagnose(Diagnostics::TypeMismatch{.expected_type = expectedType, .actual_type = actualType, .expr = expr});
```

---

### Example 2: `macroRedefinition` (with note)

**Old:**

```cpp
DIAGNOSTIC(15400, Warning, macroRedefinition, "redefinition of macro '$0'")
DIAGNOSTIC(-1, Note, seePreviousDefinitionOf, "see previous definition of '$0'")
```

(don't delete the note since it might be used elsewhere)

**Old emission (from slang-preprocessor.cpp:3809-3810):**

```cpp
sink->diagnose(nameToken.loc, Diagnostics::macroRedefinition, name);
sink->diagnose(oldMacro->getLoc(), Diagnostics::seePreviousDefinitionOf, name);
```

**New:**

```lua
-- uses notes
warning(
    "macro redefinition",
    15400,
    "macro '~name:Name' is being redefined",
    span { loc = "location", message = "redefinition of macro '~name:Name'" },
    note { message = "see previous definition of '~name'", span { loc = "original_location" } }
)
```

Note: Locations without a type default to `SourceLoc`, so just use `"location"` instead of `"location:SourceLoc"`.

**New emission:**

```cpp
getSink()->diagnose(Diagnostics::MacroRedefinition{.name = name, .location = nameToken.loc, .original_location = oldMacro->getLoc()});
```

---

### Example 3: `invalidSwizzleExpr`

**Old:**

```cpp
DIAGNOSTIC(30052, Error, invalidSwizzleExpr, "invalid swizzle pattern '$0' on type '$1'")
```

**Old emission (hypothetical):**

```cpp
getSink()->diagnose(expr, Diagnostics::invalidSwizzleExpr, swizzlePattern, baseType);
```

**New:**

```lua
err(
    "invalid swizzle expr",
    30052,
    "invalid swizzle expression",
    span { loc = "expr:Expr", message = "invalid swizzle pattern '~pattern' on type '~type:Type'" }
)
```

**New emission:**

```cpp
getSink()->diagnose(Diagnostics::InvalidSwizzleExpr{.pattern = swizzlePattern, .type = baseType, .expr = expr});
```

---

### Example 4: `expectedPrefixOperator` (with note)

**Old:**

```cpp
DIAGNOSTIC(39999, Error, expectedPrefixOperator, "function called as prefix operator was not declared `__prefix`")
DIAGNOSTIC(-1, Note, seeDefinitionOf, "see definition of '$0'")
```

**Old emission (from slang-check-overload.cpp:232-233):**

```cpp
getSink()->diagnose(context.loc, Diagnostics::expectedPrefixOperator);
getSink()->diagnose(decl, Diagnostics::seeDefinitionOf, decl->getName());
```

**New:**

```lua
-- uses notes
err(
    "expected prefix operator",
    39999,
    "function called as prefix operator was not declared `__prefix`",
    span { loc = "call_loc", message = "function called as prefix operator was not declared `__prefix`" },
    note { message = "see definition of '~decl'", span { loc = "decl:Decl" } }
)
```

Note: `call_loc` without a type suffix defaults to `SourceLoc`.

**New emission:**

```cpp
getSink()->diagnose(Diagnostics::ExpectedPrefixOperator{.call_loc = context.loc, .decl = decl});
```

---

### Example 5: `tooManyInitializers`

**Old:**

```cpp
DIAGNOSTIC(30500, Error, tooManyInitializers, "too many initializers (expected $0, got $1)")
```

**Old emission (hypothetical):**

```cpp
getSink()->diagnose(initList, Diagnostics::tooManyInitializers, expected, got);
```

**New:**

```lua
err(
    "too many initializers",
    30500,
    "too many initializers in initializer list",
    span { loc = "init_list:Expr", message = "too many initializers (expected ~expected:int, got ~got:int)" }
)
```

**New emission:**

```cpp
getSink()->diagnose(Diagnostics::TooManyInitializers{.expected = expected, .got = got, .init_list = initList});
```

---

### Example 6: `cannotConvertArrayOfSmallerToLargerSize`

**Old:**

```cpp
DIAGNOSTIC(
    30024,
    Error,
    cannotConvertArrayOfSmallerToLargerSize,
    "Cannot convert array of size $0 to array of size $1 as this would truncate data")
```

**New:**

```lua
err(
    "cannot convert array of smaller to larger size",
    30024,
    "array size mismatch prevents conversion",
    span { loc = "expr:Expr", message = "Cannot convert array of size ~source_size:int to array of size ~target_size:int as this would truncate data" }
)
```

---

## Key Differences Summarized

| Aspect           | Old System                              | New System                                                    |
| ---------------- | --------------------------------------- | ------------------------------------------------------------- |
| **Name format**  | camelCase                               | space case                                                    |
| **Interpolants** | `$0`, `$1`, ...                         | `~name:Type`                                                  |
| **Location**     | First argument in `diagnose()`          | `loc` in primary `span`                                       |
| **Message**      | Single message string                   | Split: diagnostic message (general) + span message (specific) |
| **Notes**        | Separate DIAGNOSTIC call, emitted after | Inline `note { }` attached to main diagnostic                 |
| **Type safety**  | None                                    | Generated C++ structs with typed members                      |

## Notes Handling

When diagnostics in the old system emit a diagnostic and then immediately emit some notes, add a comment `-- uses notes` above the diagnostic in the lua file. This marks them for later review and proper integration.

The pattern to look for in C++ code:

```cpp
getSink()->diagnose(loc1, Diagnostics::someError, ...);
getSink()->diagnose(loc2, Diagnostics::seeDefinitionOf, ...);  // Note follows immediately
```

Becomes:

```lua
-- uses notes
err(
    "some error",
    code,
    "general description",
    span { loc = "...", message = "..." },
    note { message = "see definition of '~decl'", span { loc = "decl:Decl" } }
)
```

---

## Updating Diagnostic Tests

After converting a diagnostic to the new Lua-based system, you must update the corresponding tests. The test system uses diagnostic annotations to verify compiler output.

### Step 1: Find Tests for the Diagnostic

Search for tests that trigger the diagnostic by its error code:

```bash
grep -r "30019" tests/   # For error code 30019
```

### Step 2: Run the Test to See Failures

```bash
./build/Debug/bin/slang-test tests/path/to/test.slang
```

If the test fails due to diagnostic changes, you'll see the mismatch.

### Step 3: Update Test Annotations

Add `diag=PREFIX` to the `DIAGNOSTIC_TEST:SIMPLE(...)` line to enable diagnostic annotation checking:

```slang
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):
```

If the test file has multiple test configurations (e.g., different targets), use unique prefixes:

```slang
//DIAGNOSTIC_TEST:SIMPLE(diag=SPIRV):-target spirv
//DIAGNOSTIC_TEST:SIMPLE(diag=HLSL):-target hlsl
```

**Important:** Choose a prefix that doesn't conflict with existing filecheck prefixes in the same file.

### Step 4: Add Diagnostic Annotations

Annotations indicate expected diagnostics using carets (`^`) to mark the column:

```slang
void test() {
    int x = "hello";
//CHECK     ^^^^^^^ expected an expression of type
}
```

**Annotation Syntax:**

- `//PREFIX   ^^^^ message substring` - Position-based check (carets align with source column)
- `//PREFIX message substring` - Simple substring check (any location)

**Block comments** for early columns or multiple diagnostics on same line:

```slang
if (x == y);
/*CHECK
^ condition warning
          ^ empty statement
*/
```

### Step 5: Run slang-test for Automatic Annotations

When you run `slang-test` with `diag=PREFIX` enabled, it will tell you exactly what annotations to add:

```bash
./build/Debug/bin/slang-test tests/path/to/test.slang
```

The output shows expected annotations. Review them to ensure they make sense, then paste them into the test file.

### Step 6: Verify the Test Passes

```bash
./build/Debug/bin/slang-test tests/path/to/test.slang
```

### Exhaustive vs Non-Exhaustive Mode

By default, tests are **exhaustive**: all diagnostics must have annotations. Use `non-exhaustive` to only check annotated diagnostics:

```slang
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):
```

Prefer exhaustive mode (default) to catch unexpected diagnostics.

### Example: Updating a Test

**Before (old style with expected output):**

```slang
//TEST:SIMPLE(filecheck=CHECK):

void test() {
    int x = "hello";
}

// CHECK: error 30019: expected an expression
```

**After (new annotation style):**

```slang
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

void test() {
    int x = "hello";
//CHECK     ^^^^^^^ expected an expression of type
}
```

### Matching Against Severity and Error Codes

Annotations can match:

- Message text: `//CHECK ^ expected an expression`
- Severity: `//CHECK ^ error`
- Error code: `//CHECK ^ 30019`
- Combined: `//CHECK ^ error 30019`

This is useful for checking specific error codes without the full message.

---

## Complete Workflow Summary

### Phase 1: Define the Diagnostic

1. **Add diagnostic definition** to `slang-diagnostics.lua` using the new format
2. **Build the project** to generate C++ structs in `build/source/slang/fiddle/slang-rich-diagnostics.h.fiddle`
3. **Verify build succeeds** - the generated structs are now available

### Phase 2: Update Emission Code (Required for Full Transition)

The diagnostic won't actually be used until the C++ code is updated to emit it using the new struct API:

**Old emission (still works during transition):**

```cpp
getSink()->diagnose(expr, Diagnostics::typeMismatch, expectedType, actualType);
```

**New emission (after full conversion):**

```cpp
getSink()->diagnose(Diagnostics::TypeMismatch{
    .expected_type = expectedType,
    .actual_type = actualType,
    .expr = expr
});
```

Until the emission code is updated, the old `DIAGNOSTIC()` definition in `slang-diagnostic-defs.h` continues to be used and tests will pass unchanged.

### Phase 3: Update Tests (After Emission Code Changes)

Once the C++ emission code is updated, tests may need to be updated:

1. **Search for tests** using `grep -r "ERROR_CODE" tests/`
2. **Run tests** with `./build/Debug/bin/slang-test`
3. **If tests fail**, update them to use the new `diag=PREFIX` annotation style
4. **Add annotations** using caret syntax based on slang-test output
5. **Verify tests pass**

### Important Notes

- During transition, both old and new diagnostic systems coexist
- Tests only need updating when the actual message text or format changes
- The new system generates type-safe C++ structs that ensure correct parameters
- Rich diagnostics with span information will provide better IDE integration

---

## Output Format Changes

When you convert a diagnostic to use the new struct-based API, the output format changes from the old style to the rich diagnostic style:

**Old output format:**

```
tests/file.slang(8): error 30201: function 'foo' already has a body
int foo(){ /* redefinition */ }
    ^~~
tests/file.slang(7): note: see previous definition of 'foo'
int foo(){ /* original */ }
    ^~~
```

**New rich diagnostic format:**

```
error[E30201]: function 'foo' already has a body
  ╭╼ tests/file.slang:8:5
  │
8 │ int foo(){ /* redefinition */ }
  │     ━━━ redeclared here

note: see previous definition of 'foo'
  ╭╼ tests/file.slang:7:5
  │
7 │ int foo(){ /* original */ }
  │     ───
```

Key differences:

- Error codes now use `[EXXXXX]` format instead of just the number
- Line/column format changes from `file(line)` to `file:line:column`
- Source context is displayed with box-drawing characters
- Span messages (like "redeclared here") appear inline with the source
- Notes are visually separated and show their own source context

This rich format is always emitted when using the new struct-based `diagnose()` calls, regardless of the `-enable-experimental-rich-diagnostics` flag.

---

## Considerations When Converting

### Control Flow Preservation

When converting diagnostics, be careful to preserve the original control flow semantics. Watch for cases where errors and notes have different conditional logic:

```cpp
// Hypothetical problematic pattern - note emitted independently
if (someCondition)
{
    getSink()->diagnose(loc1, Diagnostics::someError, ...);
}
// Note always emitted regardless of condition
getSink()->diagnose(loc2, Diagnostics::seeDefinitionOf, ...);
```

The new struct combines error + note, so you must ensure equivalent behavior. If the note should be emitted independently, you may need to keep the old pattern or emit a separate diagnostic.

One thing you can do is to construct the diagnostic struct separately and diagnose after the control flow, unset notes are not emitted.

### Mixed Type Usage

Some diagnostics have call sites that pass different types. For example, `typeMismatch` might receive:

- `Type*` and `QualType` (typed parameters)
- String literals like `"uint3"` or `"array"` (for built-in type names)

If a diagnostic has mixed usage, you have options:

1. Create separate diagnostics for different use cases
2. Use the most general type that covers all cases
3. Keep string-based call sites on the old system temporarily

The `TypeMismatch` struct in this codebase uses `Type*` for expected and `QualType` for actual, covering the common case. Call sites passing string literals would need to use a different approach.
