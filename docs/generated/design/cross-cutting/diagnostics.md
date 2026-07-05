---
generated: true
model: claude-opus-4.8
generated_at: 2026-06-29T13:27:59Z
source_commit: c21ead2690b5b9fa4a582f6b51a4cd5fb34d29d8
watched_paths_digest: d89a8a5a54b52ca27bf1790e4d64b99d371b4099edbd608a872c47d632d6eb05
warning: "Auto-generated. May drift from source. Do not edit by hand."
---

# Diagnostics

This document describes the diagnostic system that all pipeline stages
use to report errors, warnings, and notes. The intended reader is a
developer adding a new diagnostic, modifying error formatting, or
integrating Slang into a tool that consumes its diagnostics.

## DiagnosticSink

The central abstraction is `DiagnosticSink`, declared in
[slang-diagnostic-sink.h](../../../../source/compiler-core/slang-diagnostic-sink.h).
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
[slang-diagnostic-sink.h](../../../../source/compiler-core/slang-diagnostic-sink.h):

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
exposed by [slang.h](../../../../include/slang.h), so callers using the
public API see the same numeric values.

The names rendered to the user (`getSeverityName`) are:
`ignored`, `note`, `warning`, `error`, `fatal error`,
`internal error`.

## Diagnostic definitions in Lua

Diagnostics are declared in Lua source files and turned into C++
structs / message tables at build time by `slang-fiddle`.

Two layers exist:

1. **Legacy / message-table diagnostics** declared in
   [slang-diagnostics.lua](../../../../source/slang/slang-diagnostics.lua)
   and processed by
   [slang-diagnostics-helpers.lua](../../../../source/slang/slang-diagnostics-helpers.lua).
   These produce `DiagnosticInfo` entries with a printf-style
   `messageFormat` and integer ids.
2. **Rich diagnostics** declared per-area under
   [diagnostics/](../../../../source/slang/diagnostics) (currently
   `type-errors.lua`). These describe multi-span errors with primary
   and secondary labels, typed parameters, and notes; the FIDDLE
   pipeline generates strongly-typed C++ diagnostic structs.

The header that consumes the generated tables is
[slang-diagnostics.h](../../../../source/slang/slang-diagnostics.h).
Note its comment:

> All diagnostics are now defined in slang-diagnostics.lua and
> generated via slang-rich-diagnostics.h. The old
> slang-diagnostic-defs.h has been removed.

The rich-diagnostics consumer lives in
[slang-rich-diagnostics.h](../../../../source/slang/slang-rich-diagnostics.h)
/
[slang-rich-diagnostics.cpp](../../../../source/slang/slang-rich-diagnostics.cpp).
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
[slang-rich-diagnostics.h](../../../../source/slang/slang-rich-diagnostics.h)
turn the table into a C++ `struct` with one member per `params`
entry. A code path that wants to emit the diagnostic constructs the
struct and calls `sink->diagnose(...)`, which chooses message text
and labels accordingly.

### Anatomy of a legacy diagnostic

Legacy entries in
[slang-diagnostics.lua](../../../../source/slang/slang-diagnostics.lua)
follow the simpler pattern of a kebab-case `name`, an integer `code`,
a short title, and an optional primary `span` plus any additional
spans / notes. The primary span is optional: locationless
diagnostics (e.g. command-line errors such as
`cannot-deduce-source-language`) omit it entirely:

```lua
err(
    "function-redeclaration-with-different-return-type",
    30202,
    "function return type mismatch",
    span { loc = "decl:Decl", message = "function '~decl' declared to return '~newReturnType:Type' was previously declared to return '~prevReturnType:Type'" }
)
```

The `err`, `warning`, `fatal`, `standalone_note`, `span`, and `note`
helpers are defined in
[slang-diagnostics-helpers.lua](../../../../source/slang/slang-diagnostics-helpers.lua);
their signatures (e.g. `err(name, code, message, primary_span, ...)`)
fix the argument order shown above. A `~name:Type` token in a message
is a typed interpolation parameter the call site must supply.

## Source locations and message rendering

When the sink formats a diagnostic, it uses the `SourceManager` to
decode the `SourceLoc` into `file:line:column` and to retrieve the
original source line for caret rendering. Macro-expanded locations
are rendered with their expansion stack so that errors triggered
inside a macro point at both the use site and the macro body.

The sink's writer (`StdWriters::stdError` by default, or any
`ISlangWriter`) receives the formatted text. Tools that consume
diagnostics in machine-readable form can set the
`DiagnosticSink::Flag::MachineReadableDiagnostics` flag declared in
[slang-diagnostic-sink.h](../../../../source/compiler-core/slang-diagnostic-sink.h),
which switches rendering to a tab-separated record of the form
`E<code>\t<severity>\t<filename>\t<beginline>\t<begincol>\t<endline>\t<endcol>\t<message>`
(this is not a JSON schema).

## Error codes and the `name` field

Diagnostic ids live in a single shared integer namespace (`code =
"E30019"` or `30007` in the examples above) that is intended to be
managed centrally, alongside a unique `name`. Ids are normally
unique, but some are intentionally shared by more than one
diagnostic: `getDiagnosticById` in
[slang-diagnostic-sink.h](../../../../source/compiler-core/slang-diagnostic-sink.h)
notes that "it is possible for multiple diagnostics to have the same
id" and returns only the first added, and
[slang-diagnostics-helpers.lua](../../../../source/slang/slang-diagnostics-helpers.lua)
keeps an `intentional_shared_code_list` exempting those ids from the
uniqueness check. Because of this, a tool that needs to target a
precise diagnostic should prefer the `name` or `flag` group over the
integer id. Tools suppress diagnostics by passing the id, name, or
`flag` group through `overrideDiagnostic` / `overrideDiagnostics`
declared in
[slang-diagnostics.h](../../../../source/slang/slang-diagnostics.h).

The user-facing diagnostic style guide is
[../../../diagnostic-guidelines.md](../../../diagnostic-guidelines.md);
this document does not duplicate it. The conventions document covers
how to choose error codes and how to write good messages.

## Internal-compiler errors and assertions

The macros `SLANG_INTERNAL_ERROR`, `SLANG_UNIMPLEMENTED`, and
`SLANG_DIAGNOSE_UNEXPECTED` (defined in
[slang-diagnostics.h](../../../../source/slang/slang-diagnostics.h))
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
[CLAUDE.md](../../../../CLAUDE.md) for the supported values:
`system`, `debugbreak`, `release-assert-only`, or unset). On Windows
the build option `SLANG_IGNORE_ABORT_MSG` further suppresses modal
abort dialogs in unattended runs. These mechanisms are independent of
the diagnostic sink. `SLANG_ASSERT` / `SLANG_RELEASE_ASSERT` (and
`SLANG_ASSERT_FAILURE`, which an assert expands to on failure) route
through `::Slang::handleAssert`, while `SLANG_UNREACHABLE` routes
through `::Slang::handleSignal` with `SignalType::Unreachable`; both
are declared via the `slang-signal.h` include in
[slang-common.h](../../../../source/core/slang-common.h) and bypass
the sink entirely. The sink-based internal-error path is the
`SLANG_INTERNAL_ERROR`, `SLANG_UNIMPLEMENTED`, and
`SLANG_DIAGNOSE_UNEXPECTED` macros above.

## Adding a new diagnostic

1. Choose between rich and legacy. New diagnostics with multi-span
   labels or typed parameters should be **rich** (Lua tables under
   [diagnostics/](../../../../source/slang/diagnostics)). Simple
   diagnostics may continue to use the legacy form in
   [slang-diagnostics.lua](../../../../source/slang/slang-diagnostics.lua).
2. Allocate a unique integer id in the conventional range for the
   subsystem (parser, checker, lowering, IR pass, emit) — the
   conventions are in
   [../../../diagnostic-guidelines.md](../../../diagnostic-guidelines.md).
3. Choose a unique snake_case `name` (rich) or string id (legacy).
4. Write the message text. Use `~param` interpolation in the
   legacy form or `{param}` in the rich form. Fill in `params`,
   `primary_label`, and any secondary labels / notes (rich only).
5. Rebuild — `slang-fiddle` regenerates the consumer headers so the
   diagnostic appears in `Slang::Diagnostics::<Name>`.
6. Call `sink->diagnose(Slang::Diagnostics::<Name>{...})` from the
   site that detects the condition.
7. Add a `DIAGNOSTIC_TEST` regression test under
   [tests/](../../../../tests) (see
   [CLAUDE.md](../../../../CLAUDE.md) for the test directive
   conventions).

## What is not in this document

- The diagnostic style / writing guide. See
  [../../../diagnostic-guidelines.md](../../../diagnostic-guidelines.md).
- The full enumeration of every diagnostic id. The authoritative
  source is
  [slang-diagnostics.lua](../../../../source/slang/slang-diagnostics.lua)
  and the files under
  [diagnostics/](../../../../source/slang/diagnostics). Listing them
  here would replicate the build artefact and drift on every change.
