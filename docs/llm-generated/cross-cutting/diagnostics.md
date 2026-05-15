---
generated: true
model: claude-opus-4.7
generated_at: 2026-05-15T15:20:00+00:00
source_commit: e75b9a3d03659cefb39882da3adecb2eb8751e0d
watched_paths_digest: 81f47a822a99f93b8701131d76480cb13010b16d56d184d434ff385df559169e
warning: "Auto-generated. May drift from source. Do not edit by hand."
---

# Diagnostics

This document describes the diagnostic system that all pipeline stages
use to report errors, warnings, and notes. The intended reader is a
developer adding a new diagnostic, modifying error formatting, or
integrating Slang into a tool that consumes its diagnostics.

## DiagnosticSink

The central abstraction is `DiagnosticSink`, declared in
[slang-diagnostic-sink.h](../../../source/compiler-core/slang-diagnostic-sink.h).
Every front-end and back-end stage receives a sink pointer (carried on
`CompileRequestBase`, see
[../architecture/overview.md](../architecture/overview.md)) and emits
diagnostics through it.

The sink owns:

- A `Severity` filter that can suppress, downgrade, or upgrade
  diagnostics by id.
- A `SourceManager` reference (for decoding `SourceLoc` values).
- A buffered list of `Diagnostic` records, plus a writer for stream
  output.
- Per-source warning-state tracking (`SourceWarningStateTrackerBase`)
  so that pragmas / per-file overrides can adjust the severity
  enforcement on a token-by-token basis.

A `Diagnostic` is a small record:

```cpp
class Diagnostic
{
public:
    String Message;
    SourceLoc loc;
    int ErrorID;
    Severity severity;
};
```

Higher-level static metadata is held in `DiagnosticInfo`:

```cpp
struct DiagnosticInfo
{
    int id;
    Severity severity;
    char const* name;          // unique identifier
    char const* messageFormat; // printf-style format
};
```

The `DiagnosticInfo` instances are generated from the Lua tables
described below.

## Severity levels

`Severity` is declared in
[slang-diagnostic-sink.h](../../../source/compiler-core/slang-diagnostic-sink.h):

```cpp
enum class Severity
{
    Disable,
    Note,
    Warning,
    Error,
    Fatal,
    Internal
};
```

The `static_assert` block immediately following the declaration
ensures the enum values match the `SLANG_SEVERITY_*` constants
exposed by [slang.h](../../../include/slang.h), so callers using the
public API see the same numeric values.

The names rendered to the user (`getSeverityName`) are:
`ignored`, `note`, `warning`, `error`, `fatal error`,
`internal error`.

## Diagnostic definitions in Lua

Diagnostics are declared in Lua source files and turned into C++
structs / message tables at build time by `slang-fiddle`.

Two layers exist:

1. **Legacy / message-table diagnostics** declared in
   [slang-diagnostics.lua](../../../source/slang/slang-diagnostics.lua)
   and processed by
   [slang-diagnostics-helpers.lua](../../../source/slang/slang-diagnostics-helpers.lua).
   These produce `DiagnosticInfo` entries with a printf-style
   `messageFormat` and integer ids.
2. **Rich diagnostics** declared per-area under
   [diagnostics/](../../../source/slang/diagnostics/) (currently
   `type-errors.lua`). These describe multi-span errors with primary
   and secondary labels, typed parameters, and notes; the FIDDLE
   pipeline generates strongly-typed C++ diagnostic structs.

The header that consumes the generated tables is
[slang-diagnostics.h](../../../source/slang/slang-diagnostics.h).
Note its comment:

> All diagnostics are now defined in slang-diagnostics.lua and
> generated via slang-rich-diagnostics.h. The old
> slang-diagnostic-defs.h has been removed.

The rich-diagnostics consumer lives in
[slang-rich-diagnostics.h](../../../source/slang/slang-rich-diagnostics.h)
/
[slang-rich-diagnostics.cpp](../../../source/slang/slang-rich-diagnostics.cpp).
The header file is itself partially generated (note the companion
`.h.lua`).

### Anatomy of a rich diagnostic

A rich diagnostic entry, abbreviated, looks like:

```lua
diagnostic "argument_type_mismatch" {
    code = "E30019",
    severity = "error",
    flag = "type-mismatch",
    message = "cannot convert argument of type `{found}` to parameter of type `{expected}`",
    params = {
        { name = "func_name", type = "String" },
        { name = "param_name", type = "String" },
        { name = "param_index", type = "int" },
        { name = "expected", type = "Type" },
        { name = "found", type = "Type" },
    },
    primary_label = { loc = "arg_loc", message = "expected `{expected}`, found `{found}`" },
    -- secondary labels, notes, ...
}
```

The FIDDLE templates in
[slang-rich-diagnostics.h](../../../source/slang/slang-rich-diagnostics.h)
turn the table into a C++ `struct` with one member per `params`
entry. A code path that wants to emit the diagnostic constructs the
struct and calls `sink->diagnose(...)`, which chooses message text
and labels accordingly.

### Anatomy of a legacy diagnostic

Legacy entries in
[slang-diagnostics.lua](../../../source/slang/slang-diagnostics.lua)
follow the simpler pattern:

```lua
err("function return type mismatch", 30007,
    "expression type ~expression.type does not match function's return type ~returnType:Type",
    span({ loc = "expression:Expr", message = "expression type" }),
    span({ loc = "function:Decl",   message = "function return type" }))
```

`err`, `warning`, `span`, and `note` helpers are defined in
[slang-diagnostics-helpers.lua](../../../source/slang/slang-diagnostics-helpers.lua).

## Source locations and message rendering

When the sink formats a diagnostic, it uses the `SourceManager` to
decode the `SourceLoc` into `file:line:column` and to retrieve the
original source line for caret rendering. Macro-expanded locations
are rendered with their expansion stack so that errors triggered
inside a macro point at both the use site and the macro body.

The sink's writer (`StdWriters::stdError` by default, or any
`ISlangWriter`) receives the formatted text. Tools that consume
diagnostics in machine-readable form can install a JSON-emitting
writer; the JSON schema is governed by
[slang-json-diagnostic-defs.h](../../../source/compiler-core/slang-json-diagnostic-defs.h)
and
[slang-json-diagnostics.cpp](../../../source/compiler-core/slang-json-diagnostics.cpp).

## Error codes and the `name` field

Every diagnostic has a unique integer id (`code = "E30019"` or
`30007` in the examples above) and a unique `name`. Tools that need
to suppress a specific diagnostic can pass either the integer id, the
name, or the `flag` group through `overrideDiagnostic` /
`overrideDiagnostics` declared in
[slang-diagnostics.h](../../../source/slang/slang-diagnostics.h).

The user-facing diagnostic style guide is
[../../diagnostic-guidelines.md](../../diagnostic-guidelines.md);
this document does not duplicate it. The conventions document covers
how to choose error codes and how to write good messages.

## Internal-compiler errors and assertions

The macros `SLANG_INTERNAL_ERROR`, `SLANG_UNIMPLEMENTED`, and
`SLANG_DIAGNOSE_UNEXPECTED` (defined in
[slang-diagnostics.h](../../../source/slang/slang-diagnostics.h))
funnel internal-compiler errors through the same sink as ordinary
diagnostics with `Severity::Internal`. In debug builds they emit a
companion note that records the C++ source location where the macro
fired:

```cpp
(sink)->diagnoseRaw(
    Slang::Severity::Note,
    "note: internal error triggered at " __FILE__ ":" SLANG_DIAG_STRINGIFY(__LINE__) "\n");
```

The runtime behaviour of `SLANG_ASSERT` / `SLANG_RELEASE_ASSERT` is
governed by the `SLANG_ASSERT` environment variable (see
[CLAUDE.md](../../../CLAUDE.md) for the supported values:
`system`, `debugbreak`, `release-assert-only`, or unset). On Windows
the build option `SLANG_IGNORE_ABORT_MSG` further suppresses modal
abort dialogs in unattended runs. These mechanisms are independent of
the diagnostic sink but interact with it: a release-assert that fires
typically calls into the sink before terminating.

## Adding a new diagnostic

1. Choose between rich and legacy. New diagnostics with multi-span
   labels or typed parameters should be **rich** (Lua tables under
   [diagnostics/](../../../source/slang/diagnostics/)). Simple
   diagnostics may continue to use the legacy form in
   [slang-diagnostics.lua](../../../source/slang/slang-diagnostics.lua).
2. Allocate a unique integer id in the conventional range for the
   subsystem (parser, checker, lowering, IR pass, emit) — the
   conventions are in
   [../../diagnostic-guidelines.md](../../diagnostic-guidelines.md).
3. Choose a unique snake_case `name` (rich) or string id (legacy).
4. Write the message text. Use `~param` interpolation in the
   legacy form or `{param}` in the rich form. Fill in `params`,
   `primary_label`, and any secondary labels / notes (rich only).
5. Rebuild — `slang-fiddle` regenerates the consumer headers so the
   diagnostic appears in `Slang::Diagnostics::<Name>`.
6. Call `sink->diagnose(Slang::Diagnostics::<Name>{...})` from the
   site that detects the condition.
7. Add a `DIAGNOSTIC_TEST` regression test under
   [tests/](../../../tests/) (see
   [CLAUDE.md](../../../CLAUDE.md) for the test directive
   conventions).

## What is not in this document

- The diagnostic style / writing guide. See
  [../../diagnostic-guidelines.md](../../diagnostic-guidelines.md).
- The full enumeration of every diagnostic id. The authoritative
  source is
  [slang-diagnostics.lua](../../../source/slang/slang-diagnostics.lua)
  and the files under
  [diagnostics/](../../../source/slang/diagnostics/). Listing them
  here would replicate the build artefact and drift on every change.
