# Lua-Based Diagnostic System

## Overview

Slang diagnostics are defined in `source/slang/slang-diagnostics.lua`. This file is processed at build time to generate C++ structs in `slang-rich-diagnostics.h` and `slang-rich-diagnostics.cpp`.

## Defining Diagnostics

### Basic Syntax

```lua
err("diagnostic-name", 1234, "message with ~param interpolation",
    span { loc = "location:Type", message = "span label" })

warning("diagnostic-name", 5678, "message",
    span { loc = "location" })
```

Available severities are: `err`, `warning`, `internal`, `fatal`, `standalone_note`

Diagnostic names must be kebab-case: lowercase letters, numbers, and hyphens only. No underscores, no uppercase, no spaces.

### Parameters

Interpolate parameters with `~`:
- `~param` - String parameter
- `~param:Type` - Typed parameter (Type, Decl, Expr, Stmt, Val, Name, int)

### Spans

```lua
-- Named arguments
span { loc = "location:Type", message = "label text" }
```

Location types:
- Plain `"location"` - SourceLoc
- Typed `"location:Expr"` - extracts loc from Expr*, Decl*, etc.

The string `location` here refers to what name the member of the generated diagnostic struct will be given

### Notes

Notes appear after the main diagnostic with additional context:

```lua
err("redefinition", 30201, "function '~function' already has a body",
    span { loc = "function:Decl", message = "redeclared here" },
    note { message = "see previous definition", span { loc = "original:Decl", messages = "this message is attached to the original span" } })
```

Notes require at least one span. First span is primary, additional spans are secondary.

### Variadic Elements

For diagnostics with lists of similar items we can use `variadic_span` or `variadic_note`:

```lua
err("ambiguous-overload", 39999, "ambiguous call to '~name'",
    span { loc = "expr:Expr" },
    variadic_note { cpp_name = "Candidate", message = "candidate: ~signature",
        span { loc = "candidate:Decl" } })
```

Generates a nested struct and `List<Candidate> candidates` member.

### Locationless Diagnostics

Omit the span for diagnostics without source locations:

```lua
err("cannot-deduce-source-language", 12, "can't deduce language for '~path'")
```

## Using Diagnostics in C++

Generated structs live in `Slang::Diagnostics` namespace:

```cpp
#include "slang-rich-diagnostics.h"

sink->diagnose(Diagnostics::UnknownProfile{
    .profile = profileName,
    .location = loc
});

sink->diagnose(Diagnostics::MacroRedefinition{
    .name = macroName,
    .location = newLoc,
    .originalLocation = prevLoc
});
```

The struct's `toGenericDiagnostic()` method builds the full diagnostic with all spans and notes.

## Adding a New Diagnostic

1. Add definition to `source/slang/slang-diagnostics.lua`
2. Rebuild (regenerates `.fiddle` files)
3. Use the generated struct via `sink->diagnose(Diagnostics::YourDiagnostic{...})`

---

# Diagnostic Annotation Tests

## Overview

Test files can verify expected diagnostics using inline annotations. The test runner parses machine-readable diagnostic output and matches it against annotations.

## Enabling Diagnostic Tests

```slang
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target spirv
```

Options:
- `diag=PREFIX` - Use PREFIX for annotation comments (e.g., `CHECK`)
- `non-exhaustive` - Only check annotated diagnostics (default: all diagnostics must be annotated)

## Annotation Syntax

### Position-Based (with carets)

Carets align with source columns on the preceding non-annotation line:

```slang
int foo = undefined;
//CHECK:  ^^^^^^^^^ undeclared identifier
//CHECK:           ^ another error on this line
```

Use block comments for early columns.

```slang
if (x == y);
/*CHECK:
^ don't use if here
          ^ empty statement
*/
```

### Simple Substring (no carets)

Matches anywhere in output:

```slang
//CHECK: unused variable
```

### Matching Fields

Annotations can match:
- Message text
- Severity: `warning`, `error`
- Error code: `E20101`
- Combined: `warning E20101`

```slang
if (some + bad_addition)
//CHECK: ^ warning
//CHECK: ^ E20101
//CHECK: ^ bad_addition
```

## Exhaustive vs Non-Exhaustive

Default (exhaustive): Test fails if any diagnostic lacks an annotation.

Non-exhaustive (`non-exhaustive` option): Only checks that annotations match; extra diagnostics are ignored.

Use non-exhaustive when:
- Testing specific diagnostics while ignoring cascading errors
- Incremental test development
