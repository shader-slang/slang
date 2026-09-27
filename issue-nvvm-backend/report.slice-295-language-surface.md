# Qualify inheritance and initialization language interactions

## Motivation

The historical corpus inventory selected no inheritance or initializer-list sources directly, and
only two constant-expression sources. That does not prove missing semantics, but it identifies
useful interactions to qualify after the bounded material-cost study. Use four existing tests whose
outputs depend on host constant-buffer values or dispatch IDs, preserving their authored contracts.

## Proposed solution

Run each unchanged source at NVRTC O3 and NVVM O0/O3. Preserve source bodies, shader-object/feature
flags, input declarations and original expected-output sidecars. Independently derive and inspect
all four output words per cell. This is focused qualification outside the main corpus.

## Change summary

[Evidence295](research-evidence.slice-295.json) retains four source contracts and all 12 exact
outcomes. All 48 expected words pass. No compiler, provider, corpus runner, authored test or manifest
changes. The completed plan/report and navigation are retained; raw mirrors, scripts and logs live
under `build/nvvm-language-surface295`. A fresh author and separate reused reviewer audited the work.

## Concepts and vocabulary

A native ordinal identifies an authored harness directive; all four selections use ordinal zero.
The maintained mirror contains one adapted directive per backend/mode. These tests print untyped
output as hexadecimal words, so the sidecar `1111` means decimal 4,369.

## Process report

In `struct-inheritance.slang`, `Derived` inherits field `a` and `tweakBase`. Runtime cbuffer values
set `a=1` and `b=2`. A direct inherited method, value conversion to a `Base` argument, and a derived
method each contribute to the packed result. For dispatch lane `v`, the expression is
`4096 + 273*(v ^ 1) + 2`, giving hexadecimal `1113,1002,1335,1224`.

`derived-struct-init-list.slang` combines inherited defaults with an explicit aggregate initializer.
Starting from `result=1`, default `a=1,b=2` makes the first write `0x112`. The second object uses
`{v,v+1}`, producing `0x11200 + 17*v + 1`: `11201,11212,11223,11234`.

`default-init-16bit-types.slang` zero-initializes int/int16/half/int fields and adds lane values with
weights 1,16,256,4096. The tested half values are exactly representable; the integer result is
`4369*v`. `static-const-in-struct.slang` uses a scoped constant as array length and loop bound via
unqualified, dot and scope access. Its method and global helper each return `17*v`, combined as
`256*(17*v)+17*v`. Both tests expect `0,1111,2222,3333`.

Only the inheritance test reads host-supplied input values. Defaults, fills, calls and arrays may
legally disappear during optimization; these tests do not prove retained runtime instructions or
aggregate memory layout. Lane zero in the latter two tests begins at its expected zero, so its
buffer value alone does not prove that store executed. The other lanes change and all actual native
executions must be confirmed. Preserve the mixed-width test's `-render-feature int16` gate; a skip
is a failed qualification obligation, not a pass.

The maintained directive enumerator selects ordinal zero for each source. Discovery adaptation
preserves the original comparison command, input declarations, shader-object flags and int16 gate;
mirrored bodies and all four copied sidecars match exactly. There are no shader imports/includes.
The frozen runner requires the exact native CUDA identity, return zero, one passed/executed test,
no skips, empty diagnostic/shape and four exact hexadecimal words. All 12 cells pass, with no
failed, ignored, timed-out or unrun obligations and no retry. Independent review checks all raw
outputs and 58 frozen references.

Pre-execution review rejected the unexecuted first runner draft because its custom cleanup could
miss descendants after the leader exited and its CUDA identity check was weak. Version two reuses
the previously qualified owned-process helper, checks exact test identities and excludes inherited
observer/preload settings. Both preparation versions remain archived. Harmless read-only filename
inspection mistakes are retained separately; they did not execute or alter a test. No new compiler
helper, fallback or representation exists to audit.

Before/after identity checks preserve 100 layout entries, 37 runtime artifacts, 11 qualified source
files, two configurations, 576 main inputs and 22 dependency pins. Full 293 / targeted 233 / cadence 0
and 1,703 correct / 37 unresolved / 20 resolved histories remain inherited. These four sources stay
outside the main corpus and discovery capacity. No defect requiring implementation emerged; the
loop continues with an admission-boundary audit for local FP8/BF16 record arrays before choosing a
qualification gate. Existing exclusions remain in force. Slack remains skipped.
