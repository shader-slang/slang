# Slice 200: Require evidence of GPU execution

## Purpose and Observable Result

A focused GPU gate runs existing scalar/control, shared-memory, atomic-add, and wave differential
fixtures and succeeds only when each actually executes and passes. It records the device, driver,
toolkit, target, compiler, and provider. A host without a CUDA device produces a blocked JSON report
and a nonzero exit. The broader census accepts native paths and never counts zero executed tests or
ignored tests as correct.

## Current Status

Implementation and local/source checks are complete. Final GPU acceptance is blocked pending a
reboot after the device fell off the PCI bus. This is a checkpoint, not a completed fifth slice.
Earlier real GPU evidence remains valid and retained; the rebuilt producer fixes still require
physical execution and exact-key corpus comparison.

## Progress

- [x] 2026-09-22: Audited runtime fixtures, census identities, and test-summary production.
- [x] 2026-09-22: Recorded this plan before implementation and agreed the bounded scope with parent.
- [x] 2026-09-22: Implemented shared strict classification and portable frozen/discovery CLI settings.
- [x] 2026-09-22: Implemented the focused device gate, toolkit/driver metadata, and artifact hashes.
- [x] 2026-09-22: Validated 11 classification cases, six census exit-policy cases, real ignored
      fixture rejection, both classify-only JSON/TSV roundtrips, exact 452/82 identities, and blocked
      runtime exit2 when libcuda is unavailable.
- [x] 2026-09-22: With the user-approved NVIDIA open driver 615.71.09 loaded without reboot,
      ran the focused gate on RTX A6000 (SM86, driver API 13040): four executed and passed fixtures,
      zero ignored, targeting SM80 through CUDA 13.4.
- [x] 2026-09-22: GPU-enabled unit run passed 474/474 with one Windows-only ignored fixture;
      53 previously unavailable GPU cases now execute. CUDA-enabled test/renderer tools are rebuilt.
- [x] 2026-09-22: Discovery retained all 72 historically healthy cases in each of NVRTC O3, NVVM O0, and NVVM O3.
- [x] 2026-09-22: Proved both new uniform source tests fail the old compiler (0/2), then pass
      with the fix in the focused 7/7 source suite. Four CUDA 13.4 cases passed CUDA emission, NVRTC
      O3 PTX generation, and ptxas assembly (12/12 commands), retaining device-buffer read-only loads.
- [x] 2026-09-22: Preserved the initial 1,356-row frozen collection, source/binary/result hashes,
      and per-test logs. Collection finished before the later GPU fault.
- [ ] Blocked: complete and review final frozen NVRTC O3/NVVM O0/O3 evidence, including
      producer-fix replays, after recovering the device through a user-approved reboot.

## Surprises and Discoveries

`run-compute-census.py` hardcodes a Windows Release runner/provider path and compute_70 defaults.
Its original classifier marked every zero exit as correct, including ignored-only test runs.
Once a real driver became available, a second classification issue surfaced: the informational
`Check cuda: Not Supported` gfx banner could coexist with a successful direct-driver unit fixture.
The final classifier makes an exact successful execution summary authoritative over such banners. Runtime unit
fixtures compare both compilers against expected kernel results at compiler-default optimization;
they do not expose an O0/O3 override. The census owns explicit NVVM O0/O3 and NVRTC O3 runs.

## Decision Log

- 2026-09-22: Expand the slice only for two failures exposed by real validation: incomplete mirror
  directive removal and the CUDA immutable-load address-space error. The parent approved this
  scope before the compiler owner's implementation; unrelated backend gaps remain out of scope.

- 2026-09-22: Reuse four exact runtime unit fixture names rather than introduce duplicate kernels.
  Label their optimization as compiler-default. Full O0/O3 corpus validation is a separate required
  acceptance step, not inferred from this smoke gate.
- 2026-09-22: Query libcuda directly through ctypes for device metadata and availability. Do not
  infer execution support from toolkit installation or `nvidia-smi` availability.
- 2026-09-22: Keep existing frozen/discovery manifests unchanged. Architecture selection adjusts
  missing/lower baseline targets while preserving each workload's higher explicit requirement.
- 2026-09-22: The user explicitly requested plans committed with each slice. Finished plans follow
  that task exception. This plan must remain visibly incomplete until GPU acceptance exists.

## Context and Current Pipeline

A selected `nvvmSlangSharedMemoryRuntimeMatchesNVRTC` test loads the provider, initializes CUDA,
checks device capability, compiles with both NVVM and NVRTC, and executes a checked expected result.
If CUDA is unavailable, `SLANG_IGNORE_TEST` can still produce a zero process exit. The test reporter
prints executed/pass/ignored counts; that summary is the authoritative source for a gate deciding
whether GPU execution occurred. The original census ignored those counts and therefore accepts an
invalid success shape. Fix that consumer classification rather than changing valid skip semantics
inside general-purpose tests.

## Scope and Non-Goals

The orchestration owner changes `extras/validate-nvvm-runtime.py`,
`issue-nvvm-backend/run-compute-census.py`, `run-compute-discovery.py`, this plan, and its report.
Following the actual GPU census failure, a separate compiler owner may change
`source/slang/slang-ir-cuda-immutable-load.cpp` and focused CUDA regression coverage to correct the demonstrated
immutable-load address-space error. Preserve Slang semantics; do not add fabricated GPU results. Do not modify
frozen/discovery manifests, historical status, CI workflow, or packaging files. Hardware access and
all healthy-corpus results are required before claiming the original GPU validation slice complete.

## Newly Exposed Producer Boundaries

The strict one-test rule exposed a mirror construction error, not a reason to weaken validation.
`tests/hlsl-intrinsic/wave-rotate/wave-rotate.slang` contains both `//TEST` and `// TEST` directives;
`tests/language-feature/dynamic-dispatch/generic-interface-10.slang` also has a diagnostic directive.
The original mirror filter left the spaced/diagnostic directives active, so a selected workload
executed two or three tests. Match the test parser's optional spaces, extra comment slashes,
TEST/DIAGNOSTIC_TEST commands, and disabled prefixes when removing executable directives. Keep
TEST_INPUT, TEST_CATEGORY, and TEST_IGNORE_FILE metadata and the original workload identities.
Replay affected identities after collection, and merge evidence by `(id, mode)`. Because the
immutable-load pass changes CUDA source generation broadly, replay all 452 frozen NVRTC rows;
replay the 11 mirror-affected identities plus param-block alignment in both direct NVVM modes.
Retain the original 1,356-row collection and hashes beside the merged evidence.

The GPU census also exposed invalid NVRTC PTX from `tests/cuda/param-block-alignment.slang`:

```slang
uniform uint frame;
struct Block { uint dummy; };
ParameterBlock<Block> block;
RWStructuredBuffer<uint> outputBuffer;
[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    outputBuffer[0] = frame;
    outputBuffer[1] = block.dummy;
}
```

The checked uniform load is valid Slang input. CUDA source emission produces
`__ldg(&globalParams_0->frame_0)` for a field backed by constant storage; CUDA 13.4 then emits
`ld.global.nc.u32 ..., [0]`, which ptxas rejects because an immediate global-memory address is
invalid. The producer trace places the correction in `ImmutableBufferLoadLoweringContext`, before
CUDA emission: immutability is not proof of global memory. `collectGlobalUniformParameters` creates
a canonical `IRGlobalParam` with `IRUniformParameterGroupType`, and
`CUDASourceEmitter::emitParameterGroupImpl` emits that group in `__constant__` storage.
`canUseReadOnlyGlobalLoad` retains the existing immutability predicate but rejects that constant
storage root when selecting the IR read-only global-load operation. Loads through a subsequently
loaded ConstantBuffer/ParameterBlock pointer retain an `IRLoad` root and remain eligible. Add scalar
and aggregate uniform coverage, retain the existing constant-buffer `__ldg` coverage, and validate
source, PTX assembly, GPU values, and the previously healthy corpus identity. This fixes the memory
operation producer while preserving the valid canonical IR representation.

The alternative NUL-termination hypothesis was audited and rejected for this failure. Both NVRTC
and NVVM validate the vendor terminator, decrement List count, and move its unchanged allocation
into ListBlob. `ListBlob::getObject(SlangTerminatedChars)` explicitly recognizes the retained byte
at buffer[count]. Fresh getEntryPointCode/RHI paths retain the same blob; RawBlob copies allocate
an additional terminated byte. Independent ptxas rejection proves the actual syntax problem.
Do not change provider blob representation or an external RHI submodule speculatively.

## Architecture and Invariants

The runtime wrapper uses explicit compiler/bin, provider, toolkit, architecture, and output paths;
loads CUDA to obtain visible device zero (matching the fixtures); validates its target capability;
and invokes each exact fixture independently with retries disabled. It uses the census classifier
so one summary rule owns the success decision. Every invocation must report exactly one passed and
one executed test with no ignored/expected-failure/dispatch-failure suffix. Missing prerequisites
produce structured blocked evidence and nonzero status. Census discovery/reclassification are
read-only analysis workflows and do not require installed compiler/provider binaries.

## Interfaces and Dependencies

Python 3.10+, native Slang test binaries, the compiler-matched provider, CUDA Toolkit, and a physical
CUDA device are required for runtime validation. Native Windows and Linux paths are supported;
Windows-hosted WSL should use Windows Python so ctypes and test processes see the same driver.
CLI parameters select bin/config, provider, CUDA root, architecture (70/80/90), and output directory.
The runtime report records a schema, status, preflight metadata, fixture results, and blocked reason.

## Milestones

1. Add single-summary parsing and strict census classification, parameterize hardcoded paths/target.
2. Add the small device preflight and exact-fixture runner.
3. Verify false-green rejection and discovery identity preservation without a GPU.
4. Run physical hardware acceptance when hardware becomes available; retain original denominators.
5. Fix the demonstrated mirror producer and replay affected identities under the unchanged strict
   count rule. Reclassify compiler abort diagnostics before output-comparison wrappers.
6. Audit and fix the demonstrated CUDA immutable-load producer, with focused regression coverage
   and replay of the previously healthy param-block-alignment reference case.

## Validation and Acceptance

Local checks cover no tests, zero executed, ignored, expected failure, multiple summaries, nonzero
exit, legitimate one-pass, and runtime mismatch output. Discover the 452 frozen IDs from `census.slice-195.tsv` and the 82 discovery IDs using
`run-compute-discovery.py`, then compare them with `discovery-census.slice-195.tsv` exactly without
changing manifests. The historical healthy subsets are 427 frozen and 72 discovery cases. Run the runtime gate locally: no CUDA driver/device must return
blocked/nonzero, never pass. Required hardware acceptance is four real differential fixture passes,
followed by NVVM O0/O3 and NVRTC O3 runs of the frozen/discovery sets. Preserve historical
423/427 healthy frozen and 72/72 healthy discovery evidence as historical until refreshed.

## Failure and Recovery

JSON and per-fixture logs remain under the chosen build output directory. Install missing runtime
prerequisites or move to a GPU host and rerun the same command. Skip/zero-count runs cannot satisfy
acceptance. Run census `--discover-only` without GPU access to inspect scope; use manifest selection
to retain the original denominator. No GPU access means this slice remains incomplete. After the initial successful collection, the
NVIDIA driver reported that the GPU had fallen off the bus. No GPU clients remained; nvidia-smi
reset could not find the device. Module unload succeeded, but reload returned `No such device`.
An available function-level PCI reset also failed to restore the device. Do not repeat GPU runs
or report final acceptance until the device is recovered. Reboot approval is a separate system
action owned by the parent.

## Artifacts and Hand-Off

Keep runtime logs/metadata and census discovery output under `build/nvvm-slice200-*`. Commit only
scripts and requested plan/report after parent review. The parent owns durable architecture updates
and reports final hardware and corpus evidence to the user.

Additional replay validation checked census substring/regex and discovery substring selection.
Those filters precede strict inventory validation; exact selected parser fixtures pass while the
written manifests retain their complete 452/82 denominators. Neither runner defines a limit flag.

## Outcomes and Retrospective

Infrastructure and the focused real GPU gate are validated. Initial libcuda unavailability was a
driver/access issue: parent inspection found an NVIDIA PCI device at 0000:65:00.0. The user approved
installing NVIDIA open driver 615.71.09, which loaded without reboot. The gate then passed all four
fixtures on RTX A6000 (SM86, CUDA driver API 13040), targeting SM80 with CUDA 13.4. Its current JSON
is `build/nvvm-slice200-runtime/results.json`. Root CMake had cached SLANG_ENABLE_CUDA=FALSE before
the toolkit was installed; enabling it and rebuilding the test/renderer tools prepares the full
corpus lane. The GPU-enabled unit run passed 474/474 with one Windows-only ignored case; it is not
an unavailable-device skip. Discovery retained its 72 healthy cases in all three modes. Do not mark
the slice complete or refresh historical corpus correctness totals until
the frozen/discovery results are reviewed.
The exact corpus identities remain 452 total frozen and 82 total discovery; historical healthy
subsets remain 427 and 72, with the four known frozen gaps retained. Generated evidence lives
under `build/nvvm-slice200-*`.

## Post-Reboot Acceptance Hand-Off

Run the focused gate again, then `bash build/nvvm-slice200-frozen-final/resume-validation.sh`.
The prepared script reruns all 452 NVRTC O3 identities and the 12 affected identities in both direct
modes, then merges by `(id, mode)` and compares with slice 195. The final report must contain exactly
452 distinct identities and 1,356 rows, replacing 452 + 24 rows and retaining the other 880 rows.
Do not infer acceptance from diagnostic census exit codes. Confirm the historical healthy427 and
correct423 sets per identity and optimization mode, and account separately for known gaps.

The original collection is under `build/nvvm-slice200-frozen-runtime`, including results.json,
results.tsv, summary.json, comparison-slice195.json, initial-result-hashes.json,
initial-binary-hashes.json, completion-timing.json, and all logs. These generated artifacts are
intentionally outside version control. The report contains explicit runner commands if the resume
script is unavailable. If the 12-row replay manifest must be regenerated, select these frozen
identities from `census.slice-195.tsv`: `cuda/param-block-alignment.slang#cuda-1`,
`hlsl-intrinsic/wave-rotate/wave-rotate.slang#cuda-1`, and the following files under
`language-feature/dynamic-dispatch/`, each with `#cuda-1`: `generic-interface-10.slang`,
`generic-interface-4.slang`, `generic-interface-5.slang`, `generic-interface-6.slang`,
`generic-interface-7.slang`, `generic-interface-9.slang`,
`groupshared-struct-with-interface.slang`, `mutating-dispatch.slang`,
`return-interface-from-dispatch.slang`, and `this-return-chained.slang`.

After the compiler source fix, also rerun all 82 discovery NVRTC rows with
`python3 build/nvvm-slice200-resume-discovery.py`. This prepared script has been syntax-checked but
not executed. It writes `build/nvvm-slice200-discovery-nvrtc-after`, retains the initial 164 direct
rows, and writes a final comparison under `build/nvvm-slice200-discovery-runtime-final`. Require
exactly 82 identities and 246 rows, all historical 72 healthy cases correct in every mode, and no
formerly-correct discovery regressions. It refuses to overwrite existing final artifacts.
