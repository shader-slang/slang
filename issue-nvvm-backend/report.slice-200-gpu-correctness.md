# Slice 200: Require evidence of GPU execution

**Checkpoint status:** Implementation and source/assembly checks are complete. Final GPU replays
are blocked pending a reboot after the device fell off the PCI bus. This report does not claim
completion of the fifth slice or restoration of the historical 423/427 frozen result.

## Motivation

Consider running the existing scalar/control differential fixture on a host without a CUDA driver:

```sh
build/Debug/bin/slang-test -disable-retries \
  slang-unit-test-tool/nvvmSlangCUDAExecutionRuntimeMatchesNVRTC.internal
```

The process legitimately exits zero, but its summary says `0% of tests passed (0/0), 1 tests ignored`.
The original census treated every zero exit as correct. That turns unavailable hardware into false
GPU correctness evidence. Its Windows-only executable/provider paths and default compute_70 target
also prevented a consistent Linux/CUDA 13 validation lane.

## Proposed solution

Use the test reporter's executed/pass/ignored counts as the source of truth. A workload is correct
only if one invocation reports exactly one passed and executed test, no summary exception, and a
zero exit. Preserve general-purpose test skipping behavior; the stricter validation consumer owns
this requirement.

Add a focused physical-device gate around four existing NVVM/NVRTC differential fixtures: scalar
execution, shared memory, integer atomic add, and wave lane reads. Require a usable CUDA driver and
device, record toolkit/device/artifact metadata, and reject every missing prerequisite or ignored
fixture. Keep explicit NVVM O0/O3 and NVRTC O3 validation in the existing frozen/discovery census;
the focused unit fixtures use compiler-default optimization.

## Change summary

- `extras/validate-nvvm-runtime.py` selects native binaries and a toolkit/provider, records compiler,
  provider, and library hashes, queries device zero through libcuda, and invokes four exact internal
  unit fixtures. JSON and logs distinguish blocked, failed, and passed runs.
- `issue-nvvm-backend/run-compute-census.py` shares strict execution-count parsing, native path/target
  arguments, explicit diagnostic-versus-all-correct exit policy, and strict replay inventory checks.
  JSON keeps count objects; TSV explicitly serializes the count cell as JSON.
- `issue-nvvm-backend/run-compute-discovery.py` reuses those helpers and preserves its separate
  manifest and identity namespace. Discovery and log reclassification no longer require binaries.
- The census mirror producer removes every executable directive form recognized by the test parser,
  preserving metadata and ensuring that a selected workload executes exactly once.
- `source/slang/slang-ir-cuda-immutable-load.cpp` preserves ordinary loads from the constant global
  parameter group. Focused scalar/aggregate source checks cover the demonstrated NVRTC failure.
- The requested plan and report record infrastructure, physical GPU, and corpus evidence.
  No frozen/discovery manifests or historical results are changed.

## Concepts and vocabulary

A **frozen identity** is the original source/directive key, independent of the selected architecture.
The frozen census has 452 total identities; its historical NVRTC-healthy subset has 427, of which
423 passed both NVVM optimization levels. Discovery has 82 total identities and 72 historically
healthy cases. These denominators must remain separate. A **diagnostic census** reports explicit
unsupported/preflight/provider stops; its completion does not mean every workload is supported.
`--require-all-correct` is an acceptance gate for a selected supported set. A **strict replay** also
requires exactly the requested identity/mode inventory, so truncated logs cannot become acceptance.

## Process report

`TestReporter::outputSummary` already emits the relevant semantic result: how many tests executed,
how many passed, and whether any were ignored or failed in an exceptional way. The original
`_classify_result` discarded that information and accepted the process exit alone. The new
`execution_counts` preserves the reporter's counts, and `_classify_result` rejects absent, repeated,
zero-execution, ignored, or exceptional summaries. A real passing direct-driver fixture exposed a
second issue: the informational gfx capability banner could say `Check cuda: Not Supported` when
gfx had been built without CUDA. The fixture still initialized the driver and executed successfully.
An exact successful execution summary therefore takes precedence over capability inventory banners. This input shape is intentionally valid in a
general test suite: missing hardware should skip optional tests. The producer does not need a
compiler change; the dedicated correctness gate must require actual execution.

The helper inventory is `execution_counts`, `add_execution_arguments`, `execution_paths`,
`select_architecture`, `inventory_matches`, `result_exit_code`, `load_tool`, `cuda_device_metadata`,
and `require_file`, plus the runtime entry point. Each survives at its owning boundary: reporter
interpretation, shared CLI configuration, native path validation, target selection, workload/mode
identity validation, diagnostic gate policy, reuse of existing scripts, CUDA Driver API metadata,
prerequisite validation, and orchestration. None reconstructs AST/IR values or compensates for a
malformed compiler representation. The identity inventory uses `(workload id, mode)` keys, not
positional matching.

The runtime gate calls libcuda directly and selects device zero, matching the existing fixture
implementation. It compares the full major/minor capability against the requested target. Missing
libcuda, no visible device, or an insufficient device target blocks execution with a nonzero exit.
The selected toolkit must contain libdevice, libNVVM, and NVRTC. Child processes receive consistent
CUDA_PATH, CUDA_HOME, and LIBNVVM_HOME values; both selected library directories precede ambient
search paths. Recorded library paths are required toolkit inputs, not a claim that every platform's
dynamic-loader precedence has been instrumented. Windows-hosted WSL should use Windows Python;
`--host linux` explicitly selects a Linux CUDA runtime when running Python inside WSL.

The four exact `.internal` fixture names each compile using both emission methods and check expected
kernel results. A passing fixture therefore exercises both compiler routes; it does not merely
compare PTX text. The wrapper requires four successful one-test summaries. Its report explicitly
labels compiler-default optimization. The corpus runners own the three explicit optimization modes.
Their minimum target defaults to sm_80, while higher explicit workload requirements remain intact.

The stricter count rule exposed a separate producer error in mirrored test files. For example,
`wave-rotate.slang` contains both `//TEST` and `// TEST`, and `generic-interface-10.slang` contains a
`//DIAGNOSTIC_TEST` beside its CUDA directive. The old mirror filter retained those extra executable
directives, so one selected workload ran two or three tests. `EXECUTION_DIRECTIVE_RE` now follows
`_gatherTestOptions` for optional whitespace, additional comment slashes, diagnostic tests, and
disabled prefixes. It keeps TEST_INPUT, TEST_CATEGORY, and TEST_IGNORE_FILE metadata. This changes
the mirror producer, preserves workload identities, and keeps the one-executed-test invariant.
Parser boundary checks and an actual wave-rotate mirror verified exactly one executable directive.

Another already-unhealthy discovery case, `compute/texture-subscript-multisample.slang`, aborts in
the compiler with E99997 before any kernel launches. Its test wrapper also prints EXPECTED/ACTUAL
result-code text. Classification now recognizes that compiler abort as infrastructure before the
broad output-mismatch marker, so the report does not misrepresent it as incorrect kernel values.
The retained discovery logs were reclassified without changing any identities or healthy results.

The full GPU census exposed a concrete CUDA load-selection bug in `param-block-alignment.slang`:

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

`collectGlobalUniformParameters` places the scalar uniform and the block pointer into a canonical
`IRGlobalParam` of `IRUniformParameterGroupType`. CUDA emission stores that group in `__constant__`
memory. The immutable-load pass previously treated a field of this immutable group as eligible for
read-only **global** memory loading, producing `__ldg(&globalParams_0->frame_0)`. CUDA 13.4 lowered it
to `ld.global.nc.u32 ..., [0]`; independent ptxas rejected that immediate global address. The input
IR is valid and canonical. Its address space, not its spelling, makes that load operation invalid.

The new `canUseReadOnlyGlobalLoad` helper owns this distinction at the IR operation producer. It
reuses `getRootAddr` and the existing immutable-location predicate, then excludes only the canonical
uniform parameter group root. Ordinary field loads remain ordinary loads. A load through a
ConstantBuffer or ParameterBlock pointer already loaded from the group has an `IRLoad` root and
remains eligible for `__ldg`. No AST/IR value is reconstructed and no emitter fallback is introduced.
The helper survives the input-shape audit because the immutable-load pass chooses the memory
operation and already owns its legality. The new scalar and aggregate uniform source fixtures both
fail against the old compiler (0/2); existing constant-buffer coverage checks retained global-load
behavior. The rebuilt focused source suite passes 7/7: the two new tests, two existing constant-buffer
read-only-load checks, and three existing OptiX ordinary-load checks. Those existing SM70 directives
use isolated CUDA 12.9; they are source-emission checks, not the CUDA 13 runtime lane. Under CUDA
13.4, scalar uniforms, aggregate uniforms, param-block alignment, and the constant-buffer case each
pass CUDA emission, NVRTC O3 PTX compilation for SM80, and ptxas assembly (12/12 commands).
Artifact assertions retain `__ldg` and `ld.global.nc` for device buffer data while keeping the
constant-storage uniform load ordinary. The rebuilt parameter-block PTX loads the scalar with
`ld.const.u32 ... [SLANG_globalParams]`, loads the block pointer with `ld.const.u64`, and retains
`ld.global.nc.u32` for the pointed-to device value. Evidence is in
`build/nvvm-slice200-cuda-load-after/results.json`. GPU corpus replay remains pending.

The alternative PTX NUL-termination hypothesis was checked and rejected for this failure. Both
NVRTC and NVVM validate the vendor terminator, remove it from the logical List count, and move the
same allocation into ListBlob. `ListBlob::getObject(SlangTerminatedChars)` explicitly recognizes the
retained zero byte at `buffer[count]`. Fresh getEntryPointCode and RHI paths retain that same blob;
RawBlob copies allocate a terminated buffer. Independent ptxas rejection established invalid PTX
syntax rather than a consumer overread. Generic external cache blobs need their own ownership
contract, but this run supplied no evidence requiring a speculative RHI submodule change.

Local validation has passed:

- Eleven classifier cases, including empty/no-test, ignored-only, extra tests, expected failures,
  dispatch failures, repeated summaries, nonzero exits, and output mismatch.
- A real zero-exit ignored runtime fixture was classified as infrastructure with zero executed and
  one ignored test (`build/nvvm-slice200-real-skip.log`).
- Six diagnostic/strict exit-policy cases; diagnostic classified stops stay distinct from passes.
- Both complete `--classify-only` CLI paths wrote JSON/TSV execution counts consistently and rejected
  the real ignored fixture (`build/nvvm-slice200-reclassify-{frozen,discovery}`).
- Strict replay rejected a truncated 452-workload inventory and accepted explicitly selected
  complete parser fixtures for census substring/regex and discovery substring filters. Full
  manifests retained 452/82 records. These synthetic fixtures are parser checks, not GPU evidence
  (`build/nvvm-slice200-replay-inventory`, `-regex`, and `-discovery-match`). Neither runner has a
  limit flag; all available subset filters already precede strict inventory validation.
- Frozen/discovery `--discover-only` commands preserved the exact 452 and 82 identities from the
  slice-195 snapshots, without requiring installed test binaries.
- Initial runtime preflight exited 2 because libcuda was unavailable. Inspection found an NVIDIA
  PCI device without a bound usable driver, so the initial failure was driver availability, not
  absent physical hardware. The user approved NVIDIA open driver 615.71.09 installation and module
  loading; it became usable without reboot.
- The focused gate then passed **4/4 real differential fixtures, zero ignored**, on NVIDIA RTX A6000
  (SM86, CUDA driver API 13040), compiling for SM80 with CUDA 13.4. Current successful logs and JSON,
  including compiler/provider/library hashes, are under `build/nvvm-slice200-runtime`.
- The GPU-enabled unit run passed **474/474 tests, one ignored**
  (`build/nvvm-slice200-gpu-unit.log`). Fifty-three previously unavailable GPU cases now execute.
  The sole ignored case, `nvvmSlangIntegerBitHelpersRequestTypedOperations`, is explicitly guarded
  by `#if SLANG_WINDOWS_FAMILY`; it is not an unavailable-GPU result.
- The discovery census completed all 246 identity/mode rows. All 72 historically healthy cases
  pass NVRTC O3, NVVM O0, and NVVM O3; no healthy discovery regressions were found
  (`build/nvvm-slice200-discovery-runtime/comparison.slice195.json`). The frozen census and its
  producer-fix replays remain under review.

Final acceptance remains pending. Initial discovery preserved all 72 healthy cases in all three modes; the rebuilt source producer still requires its NVRTC replay. The original CMake cache had
`SLANG_ENABLE_CUDA=FALSE` from configuration before toolkit installation. The existing build was
reconfigured with CUDA enabled and its test/renderer tools rebuilt:

```sh
cmake -S . -B build -DSLANG_ENABLE_CUDA=ON -DCUDAToolkit_ROOT=/usr/local/cuda-13.4
cmake --build build --config Debug --target slang-test render-test --parallel 12
```

The resulting cache has `SLANG_ENABLE_CUDA=TRUE`, `CUDAToolkit_ROOT=/usr/local/cuda-13.4`, and
OptiX remains disabled. The focused gate's four passes do not refresh any corpus totals.

Reproduce the focused gate:

```sh
python3 extras/validate-nvvm-runtime.py --bin-dir build/Debug/bin \
  --provider build/Debug/bin --cuda-path /usr/local/cuda-13.4 --architecture 80 \
  --output build/nvvm-slice200-runtime
```

Then set CUDA_PATH, CUDA_HOME, and LIBNVVM_HOME consistently, prepend the selected toolkit's
`nvvm/lib64` and `lib64` to the Linux library search path, and run:

```sh
python3 issue-nvvm-backend/run-compute-census.py --bin-dir build/Debug/bin \
  --provider build/Debug/bin --architecture 80 --jobs 4 \
  --workload-ids-from issue-nvvm-backend/census.slice-195.tsv \
  --modes nvrtc-o3 nvvm-o0 nvvm-o3 --output build/nvvm-slice200-frozen-runtime
python3 issue-nvvm-backend/run-compute-discovery.py --bin-dir build/Debug/bin \
  --provider build/Debug/bin --architecture 80 --jobs 4 \
  --modes nvrtc-o3 nvvm-o0 nvvm-o3 --output build/nvvm-slice200-discovery-runtime
```

Compare results per identity and mode with the historical snapshots. Keep all four known healthy
frozen gaps and all unhealthy reference cases in their original denominators. A diagnostic exit code
alone cannot replace that comparison. The slice remains incomplete until the required final execution evidence is reviewed.

The initial frozen run completed all **1,356 rows across 452 identities** before the device fault.
Its all-identity raw counts were NVRTC O3: 435 correct, 16 infrastructure, one invalid-PTX failure;
each direct mode: 427 correct, 13 infrastructure, 12 preflight stops. Those all-identity totals are
not the historical healthy427 denominator. Within that fixed historical subset, NVRTC recorded
414 correct, 12 infrastructure, one invalid-PTX failure; each direct mode recorded413 correct,
12 infrastructure, two preflight stops. Ten previously correct direct cases in each mode were
rejected by the newly strict count rule because the mirror producer ran extra diagnostic tests.
These raw results are retained as evidence, not presented as final backend regression totals.
The invalid-PTX reference case and mirror producer have been fixed, but their GPU replays remain
unexecuted. Initial results, hashes, comparison, completion timestamps, and logs are preserved under
`build/nvvm-slice200-frozen-runtime`.

After collection, the NVIDIA driver reported a GPU fallen off the PCI bus. The last collected test
passed at 13:25:40 UTC, and none of the 1,356 test logs contain GPU-fault markers. No GPU clients
remained during recovery. nvidia-smi reset could not find the device; module unload succeeded, but
reload returned `No such device`. A supported function-level PCI reset also failed to restore it.
The next recovery step is a separately approved host reboot; recovery is not yet confirmed. The earlier four-fixture GPU gate,
474/474 unit passes, and healthy discovery results remain recorded; none substitutes for execution
of the rebuilt compiler fixes.

After the device is recovered, rerun the focused gate above. This checkout also has a prepared
`build/nvvm-slice200-frozen-final/resume-validation.sh`, which sets the CUDA 13.4 environment and
executes the following bounded replay:

```sh
python3 issue-nvvm-backend/run-compute-census.py --bin-dir build/Debug/bin \
  --provider build/Debug/bin --architecture 80 --jobs 8 \
  --workload-ids-from issue-nvvm-backend/census.slice-195.tsv \
  --modes nvrtc-o3 --output build/nvvm-slice200-frozen-nvrtc-final
python3 issue-nvvm-backend/run-compute-census.py --bin-dir build/Debug/bin \
  --provider build/Debug/bin --architecture 80 --jobs 4 \
  --workload-ids-from build/nvvm-slice200-frozen-runtime/producer-fix-replay.tsv \
  --modes nvvm-o0 nvvm-o3 --output build/nvvm-slice200-frozen-replay
python3 build/nvvm-slice200-frozen-final/merge-results.py
python3 build/nvvm-slice200-frozen-final/compare-slice195.py
```

The merge validates exactly 452 identities and 1,356 `(id, mode)` rows, replaces all 452 NVRTC rows
plus 24 affected direct rows, and retains 880 initial direct rows. It preserves original hashes and
logs. The plan names the exact 12 replay identities if generated build artifacts need recreation.
Review the final comparison against the fixed historical 427 healthy and 423 direct-correct sets,
including all known gaps. Final acceptance and the slice-complete commit remain pending this work.

The rebuilt producer also needs an 82-row discovery NVRTC replay. Run the prepared, syntax-checked
`python3 build/nvvm-slice200-resume-discovery.py` after device recovery. It retains the initial 164
direct rows and writes an exact 82-identity/246-row final inventory and slice-195 comparison under
`build/nvvm-slice200-discovery-runtime-final`. Require all historical 72 healthy cases to remain
correct in each mode and no formerly-correct regressions. This replay has not been executed.

A final post-recovery probe again exited 2 with `cuInit` status 100; its blocked JSON is
`build/nvvm-slice200-post-recovery-blocked/results.json`. Kernel and recovery logs are preserved
under `build/nvvm-slice200-driver-diagnostics`. No reboot has been attempted.
