# CUBIN support implementation plan

Add direct CUBIN generation through NVRTC, followed by SASS disassembly through NVIDIA's
`nvdisasm`. Reuse Slang's existing CUDA source generation and artifact pipeline so callers can
request a binary, request assembly, or dump both during compilation.

Implement this in two PRs: binary generation first, then disassembly. Each PR includes its own
tests and documentation. The checklist below tracks implementation work; all items start pending.

## Proposed behavior

Consider this kernel in `kernel.slang`:

```slang
RWStructuredBuffer<uint> outputBuffer;

[shader("compute")]
[numthreads(32, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    outputBuffer[tid.x] = tid.x + 1;
}
```

Compile it for a selected GPU architecture:

```sh
slangc kernel.slang -entry computeMain -target cubin -capability cuda_sm_8_0 -o kernel.cubin
```

Request its machine assembly:

```sh
slangc kernel.slang -entry computeMain -target cubin-asm -capability cuda_sm_8_0 -o kernel.cubin.asm
```

With `-dump-intermediates`, a CUBIN compilation writes the binary and, when the disassembler is
available, its assembly. An explicit assembly request reports a diagnostic if disassembly fails;
optional intermediate disassembly follows the existing best-effort behavior.

The API uses `slang::TargetDesc::format = SLANG_CUDA_OBJECT_CODE` for CUBIN. Add a distinct
assembly target for `cubin-asm`; append public enum values without renumbering existing values.
Keep the existing `cuobj` alias working.

For the initial implementation, require an explicit architecture selection through a CUDA SM
capability or an NVRTC architecture override. Do not select the local GPU implicitly. PTX keeps
its existing defaults. This architecture policy is a proposed default for this plan.

The first release covers ordinary CUDA kernels. Diagnose CUBIN requests for the OptiX
ray-tracing pipeline. Fat binaries, multiple architectures in one output, device linking, LTO,
SASS assembly back into CUBIN, and automatic runtime backend switching are outside this change.

## Existing implementation and invariants

- `include/slang.h` already declares `SLANG_CUDA_OBJECT_CODE`, and
  `source/slang/slang-target.h` exposes `CodeGenTarget::CUDAObjectCode`.
- `source/core/slang-type-text-util.cpp` already accepts `cuobj,cubin` as target names, but its
  CUDA object-code entry currently lists `obj,o` as filename extensions.
- `source/compiler-core/slang-artifact.h` already declares `ArtifactPayload::CuBin`.
- `NVRTCDownstreamCompiler::compile` in `source/compiler-core/slang-nvrtc-compiler.cpp` receives
  a target but always retrieves PTX. Its default architecture uses `compute_XX`.
- `isCUDATarget` in `source/slang/slang-type-layout.cpp` currently omits `CUDAObjectCode`.
- `CodeGenContext::_dumpIntermediateMaybeWithAssembly` already dumps an artifact and requests
  assembly through `ArtifactOutputUtil::maybeDisassemble`.
- `NVRTCDownstreamCompiler::convert` shares the original blob for PTX conversions. Its current
  generic disassembly predicate must be restricted before introducing binary CUDA artifacts.

The intended producer-to-consumer flow is:

```text
Slang checked program
  -> existing CUDA lowering and source emission
  -> NVRTC with a real SM architecture
  -> ObjectCode / CuBin / Kernel artifact
  -> binary output
  -> optional nvdisasm conversion
  -> Assembly / CuBin / Kernel artifact
  -> assembly output
```

The requested target is the source of truth for output format. CUBIN and PTX share CUDA
semantics, parameter layout, and source emission. Disassembly consumes the emitted binary;
it must not invoke a second compilation or substitute PTX text for machine assembly.

## PR one implements binary generation

### Connect target and artifact handling

- [ ] Map `SLANG_CUDA_OBJECT_CODE` to `ObjectCode / CuBin / Kernel` and back in
  `source/compiler-core/slang-artifact-desc-util.cpp`.
- [ ] Add `.cubin` filename inference and artifact naming. Preserve existing target aliases;
  inspect generic object-file inference before changing existing `obj,o` handling.
- [ ] Keep CUBIN classified as binary, with byte lengths preserved through blob retrieval,
  file output, and intermediate dumping.
- [ ] Register CUDA source/header to CUDA object-code transitions through NVRTC in
  `source/slang/slang-global-session.cpp`.
- [ ] Route the target through downstream selection and CUDA source selection in
  `slang-pass-through.cpp` and `slang-code-gen.cpp`.
- [ ] Audit CUDA classification, capabilities, extension tracking, layout, parameter binding,
  unsupported-instruction checks, and emission dispatch. Start with `slang-target.cpp`,
  `slang-type-layout.cpp`, `slang-parameter-binding.cpp`, `slang-ir-check-unsupported-inst.cpp`,
  `slang-emit.cpp`, and `slang-emit-c-like.cpp` under `source/slang/`.
- [ ] Review each PTX-specific switch individually. Retain PTX where it is intentionally the
  default, such as validation of emitted CUDA source and existing runtime backend selection.
- [ ] Update target-support queries and test-runner prerequisite classification so CUBIN
  compilation requires NVRTC, without requiring a GPU or disassembler.

### Retrieve CUBIN from NVRTC

- [ ] Load `nvrtcGetCUBINSize` and `nvrtcGetCUBIN` optionally in the NVRTC wrapper. Missing
  symbols must not prevent loading the compiler for PTX use.
- [ ] Validate the requested output target and report missing CUBIN support through the existing
  artifact diagnostics path.
- [ ] Retrieve PTX or CUBIN according to the target after successful compilation. Keep binary
  bytes exact; do not append, trim, or interpret a string terminator.
- [ ] Report retrieval errors and zero-length CUBIN output with useful diagnostics rather than
  returning a successful empty artifact.
- [ ] Preserve program lifetime management, compiler logs, include handling, floating-point
  options, debug options, and automatic PCH behavior.
- [ ] Restrict the existing NVRTC blob-sharing conversion to PTX artifacts in both directions.

NVIDIA documents that `nvrtcGetCUBINSize` returns zero for a virtual architecture. Treat this
as an output contract to validate at the NVRTC boundary.
See the [NVRTC API documentation](https://docs.nvidia.com/cuda/archive/12.8.0/nvrtc/index.html#nvrtcgetcubinsize).

### Select the architecture

- [ ] Determine how explicit CUDA SM capabilities reach `requiredCapabilityVersions`, including
  whether they can be distinguished from requirements inferred from shader features. Enforce the
  explicit-selection policy at the layer that still has that information.
- [ ] Use `sm_XX` for CUBIN and preserve `compute_XX` for PTX. Share the existing capability
  requirement calculation, but audit its toolkit minimum floors for real SM targets.
- [ ] Reuse or extend `_hasExplicitNVRTCArchitecture` rather than creating a competing option
  scanner. Cover `-arch`, `--gpu-architecture`, and their separate-value and equals forms.
- [ ] Preserve the existing explicit-override precedence and emit no duplicate architecture
  option. Reject virtual `compute_XX` selection for CUBIN with an actionable diagnostic.
- [ ] Preserve real architecture strings, including suffixes and future values. Let NVRTC
  validate supported architectures and conflicting or malformed downstream options.
- [ ] Diagnose the unsupported OptiX ray-tracing pipeline before CUBIN compilation.

### Verify binary generation

- [ ] Add mapping and filename-inference tests that run without CUDA installed.
- [ ] Add API and CLI tests for the example kernel, including inferred `.cubin` output and
  CUDA source pass-through. Check a nonempty ELF artifact and exact byte preservation.
- [ ] Cover capability selection, every supported architecture override spelling, missing
  explicit architecture, and virtual-architecture rejection.
- [ ] Exercise absent CUBIN symbols with a focused library fixture using existing test seams,
  and verify PTX still works. Avoid a production abstraction introduced only for this fixture.
- [ ] Cover compiler errors, retrieval failures, and empty output diagnostics.
- [ ] Load the binary and execute the example on a compatible GPU. Compare all 32 output values
  with `tid.x + 1`; select the test architecture from the device for this runtime test.
- [ ] Run existing PTX architecture-override, downstream-argument, diagnostics, and PCH tests.
- [ ] Document the target, architecture requirement, and NVRTC dependency.

Completion criterion: callers can produce and execute CUBIN through the API and CLI without
installing `nvdisasm`, while PTX behavior remains covered by the regression suite.

## PR two implements disassembly and dumping

### Add the disassembler adapter

- [ ] Add an `nvdisasm` downstream adapter using the existing compiler discovery, command-line,
  process execution, artifact materialization, and temporary-file lifetime utilities.
- [ ] Register its downstream identity and locator. Reuse existing explicit-path configuration
  conventions, followed by CUDA toolkit and `PATH` discovery; document the actual precedence.
- [ ] Support only `ObjectCode / CuBin / Kernel` to `Assembly / CuBin / Kernel` conversion.
- [ ] Write the binary artifact to a managed temporary file, invoke `nvdisasm -ndf`, and capture
  stdout as the assembly blob. Preserve stderr and process failure information as diagnostics.
- [ ] Handle executable paths containing spaces and clean up temporary artifacts on both success
  and failure. Do not construct a shell command with output redirection.
- [ ] Keep adapter availability independent of NVRTC compilation support. Report tool failures,
  including unsupported input architectures, rather than silently switching disassemblers.

`nvdisasm` accepts standalone CUBIN files and emits machine assembly, commonly called SASS.
The `-ndf` option disables dataflow analysis, avoiding its need for complete relocation
information during basic disassembly.
See [NVIDIA's binary utilities documentation](https://docs.nvidia.com/cuda/cuda-binary-utilities/index.html#nvdisasm).

### Connect assembly targets and intermediate output

- [ ] Append the public assembly target and add its internal target, text mappings, help text,
  and `.cubin.asm` naming. Verify compound-extension inference and avoid collisions with `.asm`.
- [ ] Map the assembly target to `Assembly / CuBin / Kernel` and route disassembly to the new
  adapter through `ArtifactOutputUtil::dissassembleWithDownstream`.
- [ ] Make explicit `cubin-asm` requests compile once to the binary target, then convert that
  artifact. Include the assembly target in CUDA semantics and layout where target requests need it.
- [ ] Preserve both artifacts so intermediate dumping can write the actual binary and its
  disassembly without recompilation or one output overwriting the other.
- [ ] Keep optional dumping best-effort; make explicit assembly output propagate missing-tool
  and conversion failures. Classify test prerequisites accordingly.

### Verify disassembly

- [ ] Test conversion dispatch and rejection of unrelated payloads and reverse conversion.
- [ ] Compile the example and check that assembly contains the kernel and a recognizable
  instruction listing. Avoid exact SASS snapshots across toolkit versions and architectures.
- [ ] Exercise executable discovery, paths with spaces, process failure, and temporary-file
  cleanup using focused fixtures where needed.
- [ ] Test explicit assembly failure when the tool is absent, successful binary-only compilation
  in that environment, and successful optional dumping without assembly.
- [ ] With the tool available, verify that `-dump-intermediates` writes distinct binary and
  assembly files, and that the assembly corresponds to the emitted kernel.
- [ ] Update command-line and API documentation with examples and tool dependencies.

Completion criterion: explicit assembly output and intermediate binary-plus-assembly dumping
work through the common artifact pipeline, with clear behavior when the tool is unavailable.

## Build and review requirements

Use the repository's `slang-build` skill for compiler builds, with `docs/building.md` as the
documented fallback when that skill is unavailable. Use Windows-native developer tools on
this checkout. Run focused CUDA tests and relevant unit tests before broadening validation.
Tests requiring NVRTC, `nvdisasm`, or a compatible GPU must identify those prerequisites
separately; mapping tests should run on ordinary CI workers.

Maintain an uncommitted working log during implementation. Record each motivating failure,
its producer-to-consumer trace, the chosen fix, and rejected alternatives. Before finalizing,
inventory new helpers, fallbacks, and special cases and perform the repository's input-shape
audit. In particular, changes to CUDA lowering or layout need a failing test that demonstrates
why the existing representation does not already serve CUBIN.

Write each PR description using Motivation, Proposed solution, Change summary, Concepts and
vocabulary, and Process report. Explain architecture validation and binary artifact handling
at their construction boundaries, and document any departure from this plan with its evidence.
