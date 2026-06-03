# Prompt: cross-cutting/diagnostics.md

See [_common.md](_common.md) for universal rules.

## Target

Produce `docs/generated/design/cross-cutting/diagnostics.md` — a
description of the diagnostic system that all pipeline stages use to
report errors, warnings, and notes.

Audience: a developer adding a new diagnostic, integrating Slang into
a tool that consumes its diagnostics, or modifying error formatting.

## Required structure

1. `# Diagnostics` (title)
2. `## DiagnosticSink` — the central sink interface, declared in
   [slang-diagnostic-sink.h](../../../../source/compiler-core/slang-diagnostic-sink.h).
   Describe how subsystems obtain a sink and how it is threaded
   through compilation.
3. `## Diagnostic definitions` — diagnostics are declared in Lua
   tables under [diagnostics/](../../../../source/slang/diagnostics) and
   in [slang-diagnostics.lua](../../../../source/slang/slang-diagnostics.lua).
   Describe the table schema (id, severity, message text, parameters)
   and how Lua definitions are turned into C++ enumerators / message
   tables (build-time generation; cite
   [slang-diagnostics.h](../../../../source/slang/slang-diagnostics.h) as
   the consumer header).
4. `## Severity levels` — list the levels used (note, warning, error,
   internal-error / ICE, fatal). Verify by reading the watched paths.
5. `## Source locations and message rendering` — how source locations
   from [slang-source-loc.h](../../../../source/compiler-core/slang-source-loc.h)
   become file:line:column strings in formatted output. Note rich
   diagnostics in [slang-rich-diagnostics.h](../../../../source/slang/slang-rich-diagnostics.h).
6. `## Error code namespace` — where diagnostic IDs come from and how
   they are documented (link to
   [../../diagnostic-guidelines.md](../../diagnostic-guidelines.md)
   only if it exists at `source_commit`; otherwise omit).
7. `## Internal-compiler errors` — how `SLANG_ASSERT`,
   `SLANG_ASSERT_FAILURE`, `SLANG_UNREACHABLE` interact with the
   diagnostic sink, and how environment variables (e.g.
   `SLANG_ASSERT`) tune behavior. Cite `source/core/slang-common.h`
   only if it appears in the watched paths; otherwise note it as
   out-of-scope.
8. `## Adding a new diagnostic` — short checklist (Lua entry, message
   text, parameters, severity, calling the sink at the appropriate
   site).

## Quality checklist (in addition to the universal one)

- [ ] Every claim about a sink method, severity, or generation step
      cites a watched file.
- [ ] No invented diagnostic codes.
- [ ] Document length under 24 KB.
