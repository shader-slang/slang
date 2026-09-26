# Reproduce shared NVRTC automatic-PCH ownership failures

## Motivation

Slice262 retained a concurrent NVRTC compile error deleting `default_program.pch`. Serial recovery
preserved correctness but did not resolve concurrent reliability. Establish a controlled reproduction
through Slang's actual downstream adapter before changing cache ownership.

## Proposed solution

Run the existing CPU-only invalidation unit in fresh serial, shared concurrent and isolated concurrent
working directories. All24 serial/private controls pass. Shared directories produce8 native reuse
failures and2 crashes among16 processes; every shared round fails at least once. File traces show
independent processes replacing/truncating the same PCH path. This supports an ownership fix, while
the exact262 deletion diagnostic remains unreproduced and open.

## Change summary

[Evidence275](research-evidence.slice-275.json) preserves all40 process obligations,20 round overlap
checks, original incident history, identities and audit references. This report, completed plan and
navigation change only. Raw harnesses/traces/logs remain under `build/nvvm-pch-reproduction275`.
Accepted270 compiler, inputs, layout and full-checkpoint cadence are unchanged.

## Concepts and vocabulary

Automatic PCH caches a precompiled leading-header region. A process owns its NVRTC state but, without
an explicit cache location, independent processes can operate on the same disk filename. A native
`not-created` marker after a proven first creation is the unit's reuse check; the marker alone would
not establish reuse. File-activity overlap is a scheduling check, not a performance measurement.

## Process report

Consider two independent Slang invocations compiling CUDA source beginning with the same prelude
include. `TestToolUtil::_addCUDAPrelude` installs that include; `emitEntryPointsSourceFromIR` produces
an unnamed blob artifact. `ArtifactUtil::findPath` returns an empty name and
`NVRTCDownstreamCompiler::compile` passes it to `nvrtcCreateProgram`, enabling `-pch` for a leading
include on supported versions. Both processes use `default_program.pch` in their shared directory.
Unnamed generated source is valid input. The downstream adapter owns cache policy; changing semantic
IR or inventing source identities would fix the wrong layer.

One qualified pilot and a fixed interleaved40-process inventory used the same absolute native argv,
compute75 target and options; only working-directory sharing differed. Own-child strace records PCH
creation, reads, replacement and cleanup. Every process created a PCH. All8 serial and16 isolated
processes passed; shared processes yielded6 passes,8 assertion failures and2 SIGSEGV exits. The eight
assertions expect the second A compilation to reuse its PCH (unit line191); four creation cycles replace
the controls' three. The crashes have terminal signal evidence, but file-only tracing does not identify
the exact NVRTC API stage. No failures were retried, omitted or reclassified as passes.

All16 concurrent rounds overlap actual PCH file activity: shared0.612–0.967s, isolated0.875–0.931s.
Serial rounds have zero overlap. In shared round1, one PID creates the common file, another unlinks
and recreates it, and both subsequently replace/truncate it before one crashes. Passing pilot cleanup
also has unlink ENOENT results; missing-file syscalls alone are not compiler failure evidence.
Tracing can affect scheduling, so this sample does not estimate an untraced failure rate.

Root separately audited all40 outcomes,320 raw hashes,20 overlap windows and exact native failure
messages. Reviewer delegation was unavailable at the thread limit; this is a local audit, not an
independent-agent review. All37 runtime artifacts,576 inputs,22 pins,100 installed-layout entries and
3 configuration files retain their accepted identities. No owned process remains; no build or source
change occurred. Full270 correctness is inherited, not freshly measured.

Next, establish a private cache namespace with a defensible lifetime and explicit user-option policy.
Preserve named/unnamed source identity and within-owner reuse. The existing compiler wrapper lifetime
can be shorter than the loaded NVRTC library, so removing its directory needs qualification before a
production implementation. Per-compile isolation, global cwd changes, retries and silently disabling
PCH would avoid the ownership contract rather than implement it.
