# Locate material semantic-checking call paths

## Motivation

Material attribution272 measures semantic checking at385–397ms per compile. Before changing a
shared compiler path, identify recurring work on the unchanged material and accepted270 compiler.

## Proposed solution

Use owned-child GDB sampling within `FrontEndCompileRequest::checkAllTranslationUnits`, after
qualifying entry/return boundaries, complete stacks and exact PTX. The result identifies two recurring
paths but does not yet justify a compiler optimization or representative source reduction.

## Change summary

[Evidence273](research-evidence.slice-273.json) preserves all16 profile obligations, primary and
sensitivity counts, provenance, failed pilot and limits. This report, completed plan and navigation
are retained. No compiler, corpus, configuration or installed artifact changes survive. Raw controller
versions, transcripts, enriched stacks and source trace remain at `build/nvvm-semantic-profile273`.

## Concepts and vocabulary

A logical leaf is GDB's innermost frame, including optimized inline frames. Inclusive presence counts
one occurrence per function per stack; these overlapping counts cannot be added. Dependency epochs
track extensions contributing to an inheritance result. An incomplete facet is an ancestor omitted
while breaking a semantic cycle; caching that contextual result would lose valid inheritance.

## Process report

Perf is unavailable under current host policy. GDB runs only its own compiler child, with target
function calls, initialization scripts, auto-loading and debuginfod disabled, and ASLR unchanged.
The boundary includes translation-unit and entry-point checking, matching272's logical timer scope.
The initial pilot fails before sampling because GDB cannot resolve a named disassembly expression;
a numeric-address query corrects it. Pilot02 captures40 complete stacks, exits normally and preserves
PTX. Independent review qualifies the method after owned-PID/starttime cleanup hardening.

The declared inventory is two entries x two O3 backends x two intervals(5/10ms) x two fresh repetitions,
with the second repetition in reverse order. All16 runs exit normally and preserve accepted272 PTX;
40–84 stacks per profile yield982 total. Every stack contains the semantic ancestor, unwinds to main,
and has resolved frame/module PCs. Each run has one boundary entry/exit on thread1. Additional NVRTC
threads appear after semantic exit. All owned inferiors are absent at closeout.

Debugger overhead is substantial. Resume-to-stop notifications include scheduling and symbolization,
so neither these intervals nor stack counts measure CPU time or ordinary compile latency. Retain all
982 stacks; a separate sensitivity view excludes70 notifications exceeding twice their requested
interval. Anonymous lambda names are distinguished by source file/line rather than merged blindly.

| Named path                           | All982 stacks | Sensitivity912 stacks |
| ------------------------------------ | ------------- | --------------------- |
| Generic overload candidate inference | 359           | 340                   |
| Inheritance calculation              | 338           | 312                   |

Both paths recur in every profile and across entry/backend/interval/repetition groups. Their ordering
is not stable: repetition1 ties at174 each. Allocation is the leading logical leaf (`_int_malloc`,113
observations), but that does not identify an avoidable allocation or its volume.

A retained non-delayed stack (evaluation/NVRTC/10ms/repetition0/sample14) follows `ResolveInvoke` ->
`addOverloadCandidatesForCallToGeneric` -> `inferGenericArguments` -> structural type unification ->
`_getInheritanceInfo` -> `_calcInheritanceInfo` -> `considerExtension` -> `applyExtensionToType` ->
`getTargetType` -> `Val::substituteImpl` -> the existing `substituteValWithCache`. The generic producer
preserves its outer declaration reference before matching argument and parameter types. Structural
unification consults valid supertypes when declarations differ. Extension application specializes
checked Type/DeclRef values through existing canonical builders; it does not reconstruct syntax.

For example, the existing inheritance code must distinguish an extension for `vector<float,N>` from
a query for `vector<int,2>`. This explains why extension matching is necessary; the sampler did not
capture the material's actual type pair. The inheritance cache already reuses complete entries with
current dependency epochs and handles in-progress cycles. Missing entries, changed extension epochs
and contextual incomplete results can all cause legitimate computation. Substitution already has an
operation-local cache; interface-witness specialization has its own full-input cache.

The input-shape audit therefore finds valid semantic work at its owning layer, without evidence of
repeated identical canonical keys or a malformed producer. No new helper, fallback or equivalence
relation is proposed. Zero reduction variants are attempted: inventing a small generic example would
not prove it represents the material's cost. The next bounded gate should observe existing canonical
key identities, source/candidate context and cache outcomes (cold, valid, computing, stale, incomplete),
then reduce only a demonstrated repeated case. Otherwise preserve the required work.

Independent review recomputes every per-profile and aggregate count, output hash and identity check.
All37 runtime artifacts,576 inputs,22 pins,100 layout entries and3 configuration hashes remain exact.
Accepted270 correctness and failure histories remain inherited; full270/targeted233/cadence0 are
unchanged. This research claims neither a compiler improvement nor material GPU performance. The
loop continues with the narrower observation gate; Slack remains skipped.
