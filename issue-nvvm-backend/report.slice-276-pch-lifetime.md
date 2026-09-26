# Qualify private NVRTC PCH directory lifetimes

## Motivation

Research275 demonstrates cross-process replacement of a common automatic-PCH file. A private cache
directory would isolate those owners, but compiler-object destruction need not unload NVRTC. Check
whether directory retirement while a library reference survives breaks compilation or later cleanup.

## Proposed solution

Model owner/reference lifetimes with the actual installed NVRTC API before changing Slang. All20
fresh processes and100 compilations pass. Creation/reuse/invalidation, retiring an owner's directory,
a surviving owner, recreating the same path and final external-reference release work for both empty
and named source programs. Proceed to a stable per-compiler private directory and actual adapter tests.

## Change summary

[Evidence276](research-evidence.slice-276.json) records all20 lifecycle obligations,100 API outcomes,
source-specific PTX equality, library/source identities, pilot and local audits. The plan, report and
navigation change only. Raw scripts/events/traces/PTX stay under `build/nvvm-pch-lifetime276`.
Accepted270 compiler/layout/input/configuration identities and checkpoint cadence remain unchanged.

## Concepts and vocabulary

An owner is a modeled compiler wrapper with a dlopen reference and a stable absolute `--pch-dir`.
An external reference deliberately keeps NVRTC loaded after that owner releases its handle. Program
handles exist only during individual compilations. Cache retirement removes an exclusively owned
directory after its programs are destroyed; it does not imply the library is unloaded.

## Process report

Consider two compiler wrappers retaining the same NVRTC library. Each compiles the canonical CUDA
prelude followed by an unused macro and an empty kernel. Source A repeats the same macro; source B
changes it. The direct typed API harness preserves the source name and varies only directory and
reference lifetimes. This is valid downstream input; there is no malformed AST/IR to repair.

The pilot proves A/A/B creation/reuse/rebuild with status0/13/0, successful API results and traced
cache creation/read/replacement/final cleanup. Main inventory was frozen before execution: five
scenarios, empty or `named.cu` name, two repetitions. Each fresh serial child has a180s timeout,
owned-child file tracing and flushed API-stage timestamps. No retries, missing or ignored execution.

All20 children exit0, with100 successful compiles. Retained directory gives0/13/0. New or recreated
directory after retiring an owner gives0/13 followed by0/13/0. With two warmed directories, retiring
A leaves B at13/13/0. Retaining a stable directory across owner destruction and an external reference
also preserves13/13/0. Final release succeeds and leaves no PCH files in every scenario. All100 PTX
outputs are8077bytes with identical hashes, including source/name-specific cross-process controls.
The macro change intentionally does not affect the empty kernel's semantics.

Root separately checks180 raw hashes, all20 process outcomes,100 API outcomes, handle/program order,
source-specific output equality and final cleanup. Fresh-agent delegation hit the thread limit;
the existing worker executed and root performed separate local audits, without claiming independent
review. Accepted state remains37 runtime artifacts,576 inputs,22 pins,100 layout entries and3 config
files. No production change, build, GPU execution or new full correctness checkpoint occurred.

This direct harness models ownership, not Slang compiler-wrapper destruction itself. It qualifies
installed NVRTC12.9.86, not an undocumented guarantee across all toolkit versions or arbitrary external
callers. The next implementation must retain actual adapter lifecycle tests, explicit caller-directory
precedence, unchanged program names, same-owner reuse and the fixed275 concurrent comparison. Choose
one stable directory per compiler object; let that owner clean up its files after program destruction.
Do not synthesize source names, change cwd, share by PID alone, disable PCH or add compile retries.
The original262 exact deletion diagnostic remains unclosed until the production reliability gate.
