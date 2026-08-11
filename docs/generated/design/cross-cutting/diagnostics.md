---
generated: true
model: claude-opus-5
generated_at: 2026-08-03T13:18:32Z
source_commit: 53b76e6d3009b8e6434d41573524c7ce5c499d23
watched_paths_digest: 72308d5b1cf5b2f873570484f93cd6c423c9d145955c7dd61b2a25d788038770
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
Front-end work obtains its sink from `FrontEndCompileRequest` /
`CompileRequestBase`, which stores it; back-end work obtains it from
`CodeGenContext::getSink`, which reads it out of the shared code-gen
state (see
[../architecture/overview.md](../architecture/overview.md)). Either
way, a stage emits diagnostics through the sink pointer it is handed.

The sink owns:

- A `Dictionary<int, Severity>` of per-id severity overrides
  (`m_severityOverrides`) that can upgrade any diagnostic and can
  suppress or downgrade notes and warnings; an override may not lower a
  diagnostic that is already at `Error` or above.
- A bitmask of enabled warning groups (`m_enabledWarningLevels`), which
  gates the opt-in `-Wall` / `-Wextra` / `-Wpedantic` warnings described
  under [Warning groups](#warning-groups) below.
- A `SourceManager` reference (for decoding `SourceLoc` values).
- An `outputBuffer` that accumulates formatted text when no `writer` is
  set, and an `ISlangWriter* writer` for stream output when one is.
- Per-source warning-state tracking (`SourceWarningStateTrackerBase`)
  so that pragmas / per-file overrides can adjust the severity
  enforcement on a token-by-token basis. The user surface is
  `#pragma warning`: `(push)` / `(pop)` bracket a region and
  `(<specifier> : <id-list>)` — `disable` and `suppress` among the
  specifiers — changes the state for the tokens that follow. What the
  directive itself reports is the `pragma-warning-*` block
  (15611-15616) of
  [slang-diagnostics.lua](../../../../source/slang/slang-diagnostics.lua).
  It is parsed in `slang-preprocessor.cpp`, which this page does not
  watch.

A nested sink can inherit settings from an enclosing one: the
`DiagnosticSink(SourceManager*, SourceLocationLexer, DiagnosticSink*
parentSink)` constructor copies the flags, color mode, unicode setting,
enabled warning groups, and severity overrides from `parentSink`. This
is distinct from `setParentSink`, which routes diagnostics upward: a
legacy diagnostic is forwarded as already-formatted text, while a rich
`GenericDiagnostic` is forwarded structurally and re-rendered by the
parent with its own settings.

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
    char const* messageFormat; // legacy $0-style argument format
    WarningLevel level = WarningLevel::Default; // warning group
};
```

The `DiagnosticInfo` instances are generated from the Lua tables
described below. The `level` field has a default so that the older
`DIAGNOSTIC(code, severity, name, messageFormat)` macro catalogs under
[source/compiler-core/](../../../../source/compiler-core) — for example
the one consumed by
[slang-core-diagnostics.h](../../../../source/compiler-core/slang-core-diagnostics.h)
— keep compiling as four-element aggregate initializers.

## Diagnostic definitions

Diagnostics are declared in Lua source files and turned into C++
structs / message tables at build time by `slang-fiddle`. There is a
single catalog, and every entry in it is a rich diagnostic:

1. [slang-diagnostics.lua](../../../../source/slang/slang-diagnostics.lua)
   declares the compiler's diagnostics by calling the `err`, `warning`,
   `standalone_note`, `internal`, and `fatal` helpers.
2. [slang-diagnostics-helpers.lua](../../../../source/slang/slang-diagnostics-helpers.lua)
   collects those calls, validates them, and parses each message into
   typed parameters and spans. The file ends by calling
   `helpers.process_diagnostics` and returning the processed list,
   raising a Lua error if validation failed.
3. [slang-rich-diagnostics.h.lua](../../../../source/slang/slang-rich-diagnostics.h.lua)
   loads that processed list and supplies the mapping helpers used by
   the templates (`toPascalCase`, `getCppType`, `getSeverityEnum`,
   `getWarningLevelEnum`).
4. The FIDDLE templates embedded in
   [slang-rich-diagnostics.h](../../../../source/slang/slang-rich-diagnostics.h)
   and
   [slang-rich-diagnostics.cpp](../../../../source/slang/slang-rich-diagnostics.cpp)
   emit, per entry, a `struct` in `namespace Slang::Diagnostics` with
   one member per parameter and location, a `toGenericDiagnostic()`
   method that interpolates the message and builds the spans, and a
   `DiagnosticInfo` constant carrying the code, severity, name, and
   warning group.

`Diagnostics::getRichDiagnosticsInfo()` /
`getRichDiagnosticsInfoCount()` hand that generated `DiagnosticInfo`
array to `DiagnosticsLookup`, which
[slang-diagnostics.cpp](../../../../source/slang/slang-diagnostics.cpp)
then augments with the non-conflicting entries from
`getCoreDiagnosticsLookup()` and the single alias
`overlappingBindings` → `parameterBindingsOverlap`. An alias is only
observable where a user names a diagnostic — `-warnings-disable`,
`-warnings-as-errors`, `-W<id>`, and `-Wno-<id>` resolve their operand
through `findDiagnosticByName` — so `-warnings-disable
overlappingBindings` keeps working alongside the canonical name.

The header that consumes the generated tables is
[slang-diagnostics.h](../../../../source/slang/slang-diagnostics.h).
Note its comment:

> All diagnostics are now defined in slang-diagnostics.lua and
> generated via slang-rich-diagnostics.h. The old
> slang-diagnostic-defs.h has been removed.

Note that steps 2 and 3 rest on `slang-diagnostics-helpers.lua` and
`slang-rich-diagnostics.h.lua`, neither of which is in this page's
watched paths even though the manifest already watches
`slang-diagnostics.lua` and both `slang-rich-diagnostics` C++ files.
Both should be added to the manifest so changes to the schema or the
generator helpers mark this page stale.

### Anatomy of a diagnostic entry

An entry has a kebab-case `name`, an integer `code`, a short title, and
an optional primary `span` followed by any additional spans, notes, and
a warning-group sentinel:

```lua
err(
    "function-redeclaration-with-different-return-type",
    30202,
    "function return type mismatch",
    span { loc = "decl:Decl", message = "function '~decl' declared to return '~newReturnType:Type' was previously declared to return '~prevReturnType:Type'" }
)
```

The primary span is optional: locationless diagnostics (e.g.
command-line errors such as `cannot-deduce-source-language`) omit it
entirely.

The `err`, `warning`, `internal`, `fatal`, `standalone_note`, `span`,
`note`, `variadic_span`, and `variadic_note` helpers are defined in
[slang-diagnostics-helpers.lua](../../../../source/slang/slang-diagnostics-helpers.lua);
their signatures (e.g. `err(name, code, message, primary_span, ...)`)
fix the argument order shown above. A `~name:Type` token in a message
is a typed interpolation parameter the call site must supply; the
helper's `parse_message` also understands member access such as
`~decl.name`. `validate_diagnostic` rejects a name that is not valid
kebab-case and a `severity` outside `error`, `warning`, `note`,
`internal`, and `fatal`.

The declaring helper fixes the entry's severity, and with it the kind
of condition the entry describes. `err`, `warning`, and
`standalone_note` entries report on the input; `fatal` entries do too,
but on a condition the compiler cannot continue past (e.g.
`cyclic-reference`). `internal` entries report the compiler's own
failure: `internal-compiler-error`, `unimplemented`, and `unexpected`
— all code `99999` — are what the macros under
[Internal-compiler errors](#internal-compiler-errors) raise, so input
that reaches one is a compiler defect, not a supported reproduction.

A `variadic_span` or `variadic_note` becomes a nested struct plus a
`List<>` member on the generated struct, and `toGenericDiagnostic()`
loops over that list adding one `DiagnosticSpan` or `DiagnosticNote`
per element — so a variadic note renders as one note record per
supplied item, not one joined record.
`ambiguous-overload-for-name-with-args` uses one for its candidates:

```lua
variadic_note { cpp_name = "Candidate", message = "candidate: ~candidateSignature", span { loc = "candidate:Decl" } }
```

To put a warning in an opt-in group, pass one of the sentinel values
`helpers.all`, `helpers.extra`, or `helpers.pedantic` as a trailing
positional argument.
[slang-diagnostics.lua](../../../../source/slang/slang-diagnostics.lua)
binds `extra` and `pedantic` as locals near the top of the file; `all`
has no user yet and would need the same one-line binding.
`add_diagnostic` recognises the sentinel by its `is_warning_level`
marker, records it as the entry's `level`, and does not mistake it for a
span:

```lua
warning(
    "vertex-shader-missing-sv-position",
    38052,
    "vertex shader '~entryPoint:Name' has no output with the 'SV_Position' system value semantic",
    span { loc = "location", message = "..." },
    pedantic
)
```

`getWarningLevelEnum` in `slang-rich-diagnostics.h.lua` maps the string
`"default"` / `"all"` / `"extra"` / `"pedantic"` to the matching
`WarningLevel::` enumerator that the generated `DiagnosticInfo` carries.

### The prototype schema under `source/slang/diagnostics/`

[diagnostics/type-errors.lua](../../../../source/slang/diagnostics/type-errors.lua)
uses a different, declarative schema — `diagnostic "name" { code = ...,
severity = ..., flag = ..., params = ..., primary_label = ...,
secondary_labels = ..., notes = ..., helps = ... }` — and describes
itself as a "guinea pig" for prototyping the multi-span diagnostic
system. At `source_commit` no build rule, FIDDLE template, or Lua
`dofile` in the repository loads this file, and no other file in that
directory exists, so nothing in the shipped compiler is generated from
it. Treat `slang-diagnostics.lua` as the live catalog; in particular the
`flag` key shown there has no consumer in the current pipeline.

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

A `static_assert` block in the same header
ensures the enum values match the `SLANG_SEVERITY_*` constants
exposed by [slang.h](../../../../include/slang.h), so callers using the
public API see the same numeric values.

The names rendered to the user (`getSeverityName`) are:
`ignored`, `note`, `warning`, `error`, `fatal error`,
`internal error`. Only five of the six are ever printed: a diagnostic
whose effective severity is `Severity::Disable` returns from
`diagnoseImpl` / `diagnoseRichImpl` before rendering, so `ignored`
names a state of the severity machinery, not a form of output.
`fatal error` and `internal error` are printed and then abort the
compile through `SLANG_ABORT_COMPILATION`.

`DiagnosticSink::getEffectiveMessageSeverity` in
[slang-diagnostic-sink.cpp](../../../../source/compiler-core/slang-diagnostic-sink.cpp)
turns the static `DiagnosticInfo::severity` into the severity actually
used, in this order: the per-source warning-state tracker may adjust a
note/warning first; then a per-id entry in `m_severityOverrides` wins if
one exists — except that it may not lower a severity that has already
reached `Error`, `Fatal`, or `Internal`, where only an override at least
as severe applies — otherwise an un-enabled warning group demotes the warning to
`Severity::Disable`; finally the `TreatWarningsAsErrors` flag promotes
any surviving warning to `Severity::Error`. A per-id override therefore
takes precedence over group gating, which is what lets `-W<id>`
force-enable a single warning from a group that is off. The option
table spells these `-W<id>` / `-Wno-<id>`, but the operand goes
through `overrideDiagnostic`, which accepts an integer id or a
diagnostic name.

### Warning groups

Warnings can be tagged with a group so they are only emitted when the
user opts in. The groups are declared as `WarningLevel` in
[slang-diagnostic-sink.h](../../../../source/compiler-core/slang-diagnostic-sink.h),
modeled on the clang/gcc `-Wall` / `-Wextra` / `-Wpedantic` groups:

```cpp
enum class WarningLevel
{
    Default = 0,
    All = 1,
    Extra = 2,
    Pedantic = 3,
};
```

A `static_assert` block keeps these values in step with the
`SLANG_WARNING_LEVEL_*` constants of the public `SlangWarningLevel` enum
in [slang.h](../../../../include/slang.h), just as the neighbouring
block does for `Severity`.

The groups are **independent, not nested**: a warning is gated on
exactly the one group it carries. `Default` is the implicit group of
every untagged diagnostic and is always emitted. The sink's
`m_enabledWarningLevels` bitmask starts with only the `Extra` bit set,
so `Extra` warnings fire out of the box while `All` and `Pedantic`
warnings stay silent until enabled. `DiagnosticSink::enableWarningLevel`
sets a bit (bounds-checked before shifting, so a bogus integer coming
through the public API cannot cause an out-of-range shift), and
`isWarningLevelEnabled` is the predicate
`getEffectiveMessageSeverity` consults. `getEnabledWarningLevels` /
`setEnabledWarningLevels` expose the raw mask so it can be copied
between sinks, mirroring `getFlags` / `setFlags`.

Because enabling is per-group rather than cumulative,
`overrideDiagnosticSeverity` cannot treat "override equals the nominal
severity" as a no-op for grouped warnings. It only drops such an
override when `info->level == WarningLevel::Default`; for a grouped
warning, overriding it back to `Warning` is the meaningful act of
force-enabling it, so the entry must be kept.

The plumbing that turns a user request into `enableWarningLevel` calls
(the `-Wall` / `-Wextra` / `-Wpedantic` command-line spellings, the
`SlangWarningLevel` enum, and the `CompilerOptionName::WarningLevel`
option that carries the group as `intValue0`) lives in
[include/slang.h](../../../../include/slang.h),
[slang-options.cpp](../../../../source/slang/slang-options.cpp), and
[slang-compiler-options.cpp](../../../../source/slang/slang-compiler-options.cpp),
all three of which this page watches.

## Source locations and message rendering

When the sink formats a diagnostic, it uses the `SourceManager` to
decode the `SourceLoc` into `file:line:column` and to retrieve the
original source line for caret rendering. Two formatting paths differ
here. The legacy path (`diagnoseImpl`, taken only when the
`AlwaysGenerateRichDiagnostics` flag is not set) calls the
`formatDiagnostic` helper in
[slang-diagnostic-sink.cpp](../../../../source/compiler-core/slang-diagnostic-sink.cpp),
which, when the location falls in a synthesized token-paste view
(`PathInfo::Type::TokenPaste`), loops back through
`SourceView::getInitiatingSourceLoc()` emitting a
`MiscDiagnostics::seeTokenPasteLocation` note for each hop. The rich
path (`diagnoseRichImpl`, which is how every catalog entry is
reported) renders through `renderDiagnostic` and has no such loop, so
a rich diagnostic inside a `##` paste points only at the pasted text.

A rendered diagnostic opens with a header of the form
`<severity>[E<5-digit id>]: <message>`. The id is zero-padded and
always carries an `E` prefix whatever the severity, so a warning with
id 41016 prints as `warning[E41016]`, not `W41016`; an entry with a
negative code (`seeTokenPasteLocation` is `-1`) prints no bracket. The
location, source excerpt, and notes follow:

```
warning[E41016]: use of uninitialized variable
 --> uninit.slang:6:13
```

`-diagnostic-color always|never|auto` selects more than colour:
unless a host sets the unicode flag explicitly through
`setEnableUnicode`, the renderer picks its frame glyphs with the same
predicate. With colour off the frame is ASCII (`-->`, `|`, `^`, `-`);
with colour on it is Unicode box drawing (`╭╼`, `│`, `━`, `┬`)
wrapped in ANSI SGR escapes. `auto` asks the writer whether it is a
console, so piped output gets the ASCII form.

Diagnostics raised while parsing the command line are attached to a
synthetic source, not to a file in the translation unit: the options
parser gives its own sink the `CommandLineContext`'s source manager,
whose one `SourceView` carries `PathInfo::Type::CommandLine`. The
renderer special-cases that type and prints the path (`command line`)
with no `line:column`, so a position- or caret-anchored matcher has
nothing to bind to — pin such a diagnostic by its error code.

The formatted text goes to the sink's `writer` if one is set, and
otherwise accumulates in `outputBuffer`, from which
`getBlobIfNeeded` can hand it back as an `ISlangBlob`. Tools that consume
diagnostics in machine-readable form can set the
`DiagnosticSink::Flag::MachineReadableDiagnostics` flag declared in
[slang-diagnostic-sink.h](../../../../source/compiler-core/slang-diagnostic-sink.h)
(`-enable-machine-readable-diagnostics` sets it, plus
`AlwaysGenerateRichDiagnostics`), which switches rendering to a
tab-separated record of the form
`E<code>\t<severity>\t<filename>\t<beginline>\t<begincol>\t<endline>\t<endcol>\t<message>`
(this is not a JSON schema).

## Error code namespace

Diagnostic ids live in a single shared integer namespace (`30202` in the
example above) that is managed centrally, alongside a unique `name`.
`process_diagnostics` in
[slang-diagnostics-helpers.lua](../../../../source/slang/slang-diagnostics-helpers.lua)
enforces three rules over that namespace at generation time:

- Names and codes must both be unique
  (`allow_duplicate_diagnostic_codes` is `false`).
- Diagnostics that do share a code must share a severity
  (`allow_severity_conflicts` is `false`), because
  `-warnings-disable <id>` resolves the id to a single entry and then
  checks its severity.
- A code must not be bound to one name in the Lua catalog and a
  different name in one of the C++ `DIAGNOSTIC(...)` catalogs listed in
  `cpp_diagnostic_defs_files` (the `slang-misc-`, `slang-lexer-`, and
  `slang-json-diagnostic-defs.h` files under
  [source/compiler-core/](../../../../source/compiler-core)).

A short `intentional_shared_code_list` exempts deliberately multi-bound
codes — negative sentinels, the `10000` illegal-character variants, the
`39999` overload/lookup umbrella, the `99999` internal-error catch-all,
and the JSON catalog's `20001`-`20012` range — from the uniqueness and
cross-catalog checks. Because
`getDiagnosticById` in
[slang-diagnostic-sink.h](../../../../source/compiler-core/slang-diagnostic-sink.h)
notes that "it is possible for multiple diagnostics to have the same id"
and returns only the first added, a tool that needs to target a precise
diagnostic should prefer the `name` over the integer id. The same
caveat applies when *reading* output: `39999` is carried by about two
dozen entries, `no-applicable-overload-for-name-with-args` and
`ambiguous-overload-for-name-with-args` among them, and the rendered
header carries the id and the message but never the name, so for a
multi-bound code the message text is the only discriminator.

Tools suppress or promote diagnostics through `overrideDiagnostic` /
`overrideDiagnostics`, declared in
[slang-diagnostics.h](../../../../source/slang/slang-diagnostics.h) and
implemented in
[slang-diagnostics.cpp](../../../../source/slang/slang-diagnostics.cpp).
Each accepts a single identifier (or a comma-separated list) that is
either an integer id or a name. An unrecognised name is reported as
`UnknownDiagnosticName`, whereas an unrecognised numeric id is silently
ignored so that a build script can disable a warning without knowing
which compiler version introduced it. If `originalSeverity` is anything
other than `Severity::Disable`, the looked-up diagnostic's severity must
match it, so `-warnings-disable` cannot be used to silence an error.
The mismatch is rejected here, at the option layer, not by
`getEffectiveMessageSeverity`, and is reported as
`unknown-diagnostic-name` (`31111`) naming the requested identifier —
the same diagnostic an absent name gets. Since that entry is an `err`,
the request fails the compile rather than leaving the diagnostic
un-silenced.
Name lookup is convention-insensitive: `findDiagnosticByName` accepts
the kebab-case spelling from the Lua entry as well as the lower-camel
`DiagnosticInfo::name` the generator produces from it.

The user-facing diagnostic style guide is
[../../../diagnostic-guidelines.md](../../../diagnostic-guidelines.md);
this document does not duplicate it. The conventions document covers
how to choose error codes and how to write good messages.

## Internal-compiler errors

The macros `SLANG_INTERNAL_ERROR`, `SLANG_UNIMPLEMENTED`, and
`SLANG_DIAGNOSE_UNEXPECTED` (defined in
[slang-diagnostics.h](../../../../source/slang/slang-diagnostics.h))
funnel internal-compiler errors through the same sink as ordinary
diagnostics with `Severity::Internal`. In debug builds
`SLANG_INTERNAL_ERROR` and `SLANG_UNIMPLEMENTED` — but not
`SLANG_DIAGNOSE_UNEXPECTED`, which is defined outside the debug
conditional — emit a companion note that records the C++ source
location where the macro fired:

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
the diagnostic sink. `SLANG_RELEASE_ASSERT` always calls
`::Slang::handleAssert` directly; `SLANG_ASSERT` does the same in
debug builds but expands to `SLANG_ASSUME(VALUE)` in release builds,
so it becomes an optimizer hint rather than a check.
`SLANG_ASSERT_FAILURE` is a separate macro that calls
`::Slang::handleAssert` unconditionally, not something an assert
expands to. `SLANG_UNREACHABLE` routes
through `::Slang::handleSignal` with `SignalType::Unreachable`;
`handleAssert` and `handleSignal` are both declared via the
`slang-signal.h` include in
[slang-common.h](../../../../source/core/slang-common.h) and bypass
the sink entirely. The sink-based internal-error path is the
`SLANG_INTERNAL_ERROR`, `SLANG_UNIMPLEMENTED`, and
`SLANG_DIAGNOSE_UNEXPECTED` macros above.

## Adding a new diagnostic

1. Add an `err`, `warning`, `standalone_note`, `internal`, or `fatal`
   call to
   [slang-diagnostics.lua](../../../../source/slang/slang-diagnostics.lua),
   next to the other diagnostics in the same numeric range.
2. Allocate a unique integer id in the conventional range for the
   subsystem (parser, checker, lowering, IR pass, emit) — the
   conventions are in
   [../../../diagnostic-guidelines.md](../../../diagnostic-guidelines.md).
   `process_diagnostics` will fail the build if the id collides with
   another Lua entry or with a C++ `DIAGNOSTIC(...)` catalog.
3. Choose a unique kebab-case `name`; the generator derives the C++
   `PascalCase` struct name and the `lowerCamelCase`
   `DiagnosticInfo::name` from it.
4. Write the message text, using `~param` / `~param:Type` interpolation,
   and add a primary `span` plus any secondary spans, `note`s, or
   variadic spans/notes.
5. For a warning that should not fire by default, append the `all` or
   `pedantic` sentinel as the last positional argument; the warning then
   requires `-Wall` or `-Wpedantic`. Do not use `extra` for this: the
   sink initializes `m_enabledWarningLevels` with the `Extra` bit
   already set, so an `extra` warning fires without any `-W` flag.
6. Rebuild — `slang-fiddle` regenerates the consumer headers so the
   diagnostic appears in `Slang::Diagnostics::<Name>`.
7. Call `sink->diagnose(Slang::Diagnostics::<Name>{...})` from the
   site that detects the condition, including
   `slang-rich-diagnostics.h` in that translation unit.
8. Add a `DIAGNOSTIC_TEST` regression test under
   [tests/](../../../../tests) (see
   [CLAUDE.md](../../../../CLAUDE.md) for the test directive
   conventions).

## What is not in this document

- The diagnostic style / writing guide. See
  [../../../diagnostic-guidelines.md](../../../diagnostic-guidelines.md).
- The full enumeration of every diagnostic id. The authoritative
  source is
  [slang-diagnostics.lua](../../../../source/slang/slang-diagnostics.lua),
  plus the C++ `DIAGNOSTIC(...)` catalogs it cross-checks against under
  [source/compiler-core/](../../../../source/compiler-core). Listing
  them here would replicate the build artefact and drift on every
  change.
- The command-line and API surfaces that drive the sink
  (`-warnings-disable`, `-warnings-as-errors`, `-Wno-<name>`, `-Wall` /
  `-Wextra` / `-Wpedantic`, and the matching `CompilerOptionName`
  entries). These live in
  [slang-options.cpp](../../../../source/slang/slang-options.cpp),
  [slang-compiler-options.cpp](../../../../source/slang/slang-compiler-options.cpp),
  and [include/slang.h](../../../../include/slang.h); the option
  surface itself is documented in the user guide rather than here.
