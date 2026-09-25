# Establish the CUDA quad helper and reconvergence contract

## Motivation

The last two preflight cells in the selected frozen wave domain are direct O0/O3 for
`tests/hlsl-intrinsic/quad-control/quad-control-comp-functionality.slang`. Both reject
`RequireMaximallyReconverges`. Its 16-thread kernel divides four complete quads among branches:

```slang
uint index = WaveGetLaneIndex();
if (index < 4)
    outputBuffer[index] = uint(QuadAny((index % 4) == 0));
else if (index < 8)
    outputBuffer[index] = uint(QuadAny(false));
else if (index < 12)
    outputBuffer[index] = uint(QuadAll((index % 4) == 0));
else
    outputBuffer[index] = uint(QuadAll(true));
```

The independently expected output is four ones, eight zeros, then four ones. Existing NVRTC passes.
Merely deleting the first rejected instruction would neither implement the quad operation nor
establish its convergence contract. Research 234 identifies the responsible boundary without
changing production, provider, runner, fixtures, corpus registration or expected output.

## Proposed solution

Treat the checked CUDA quad intrinsic as a complete typed helper contract in a later bounded slice.
Its canonical `bool(bool)` body contains two target requirements and terminates in GenericAsm
`_slang_quadAny` or `_slang_quadAll`. CUDA source emission consumes that body as one intrinsic.
A direct recipe can use existing lane-index, integer indexed shuffle, Boolean conversion and
combine operations. A helper-specific admission policy must account for both requirement markers;
standalone requirement instructions must not become globally accepted no-ops.

This is a research handoff, not implemented support. No claim of Vulkan maximal reconvergence or
active-only quad voting is added to CUDA. The CUDA helper's full-mask shuffle participation and
complete source-quad restrictions remain part of the source contract being reproduced.

## Change summary

- This report and the completed slice plan record the canonical trace, external semantics,
  experiments, rejected shortcuts and bounded follow-up.
- `semantic-evidence.slice-234.json` hash-addresses scripts, raw inputs, independent expectations,
  actual outputs, generated CUDA/PTX/cubins, diagnostics, official specifications and identity audits.
- STATUS keeps implementation acceptance at 233, full checkpoint at 229 and cadence at two while
  recording this research handoff. The design document records only durable target distinctions.
- All generated artifacts remain under ignored `build/nvvm-loop/slice-234-quad`. No worker commit.

## Concepts and vocabulary

A _requirement marker_ describes an entry-point requirement consumed by a target; it is not a warp
barrier. A _GenericAsm intrinsic body_ names a target implementation, allowing source emission to
replace the function call rather than execute its ordinary body instructions. A _complete source
quad_ has all four addressed lanes participating in the corresponding shuffle rendezvous. The
_shuffle member mask_ identifies rendezvous participants; it does not define a filtered Boolean
reduction or create values for absent source lanes. LLVM's _convergent_ attribute limits optimizer
changes to control dependence; it does not impose SPIR-V maximal reconvergence on a whole kernel.

## Process report

`hlsl.meta.slang` defines `QuadAny` and `QuadAll` with unconditional calls to
`__requireMaximallyReconverges()` and `__requireQuadDerivatives()` before the target switch.
`core.meta.slang` maps them directly to the two IR instructions. The linked CUDA helper is a single
block with one Boolean parameter, those two markers, and its GenericAsm terminator. The input shape
is canonical, intentional checked semantic data, not malformed frontend syntax or an alternative
value representation. The source comment explicitly discusses SPIR-V/GLSL execution modes.
The public attribute documentation in `core.meta.slang` also scopes those attributes to SPIR-V;
GLSL's actual emitter additionally translates them to corresponding extension layouts.

For SPIR-V, `SPIRVEmitContext::emitLocalInst` handles the markers by requiring execution modes on
referencing entry points. GLSL's `_beforeComputeEmitProcessInstruction` adds entry-point
decorations before source emission. The Khronos extensions impose real reconvergence and
derivative-group requirements there; the quad votes evaluate active invocations. They are not a
portable instruction to synchronize arbitrary CUDA control flow. See the official
[maximal reconvergence](https://github.com/KhronosGroup/SPIRV-Registry/blob/main/extensions/KHR/SPV_KHR_maximal_reconvergence.asciidoc)
and [quad control](https://github.com/KhronosGroup/SPIRV-Registry/blob/main/extensions/KHR/SPV_KHR_quad_control.asciidoc)
specifications. Exact fetched documents and hashes are retained locally.

For CUDA, `findTargetIntrinsicDefinition` in `slang-ir.cpp` recognizes the GenericAsm terminator.
`CLikeSourceEmitter::isTargetIntrinsic`, `emitFunc`/`emitFuncDecl` and `emitCallExpr` suppress the
function definition and emit `_slang_quadAny(expr)`/`_slang_quadAll(expr)` instead. The emitter checks
required CUDA capabilities and preludes, but does not execute the markers in that function body.
Standalone marker probes confirm the distinction: each produces direct E52017 at O0/O3, while
NVRTC/CUDA-source attempts reach E99999 for an unimplemented source opcode. These are existing
compiler limitations of an internal-operation probe, not valid support tests or newly introduced
regressions. Initial target-intrinsic alias probes also produced E45001; those failed constructions
remain intact, separately classified. Correct GenericAsm-body probes succeed on CUDA source/NVRTC.

Direct `_visitNVVMFunction` collects the reachable helper, and `_validateNVVMFunction` scans its
ordinary instructions before reaching the GenericAsm admission switch. It rejects the first marker.
Isolating the second marker proves an independent `RequireQuadDerivatives` rejection. Removing both
markers only in generated research probes reaches the next exact boundary:
`GenericAsm assembly=_slang_quadAny, signature=bool(bool)` and the corresponding `_slang_quadAll`
diagnostic at both direct optimization levels. The responsible layer is thus typed CUDA helper
admission plus its body requirements. No additional compiler feature is needed to describe its
integer shuffle algebra; the controls execute that algebra on the unchanged direct backend.

The CUDA prelude implements each vote with four `__shfl_sync(0xffffffff, expr, base | k)` calls,
where `base = lane & ~3` and `k` ranges from zero through three, followed by OR or AND. It does not
ballot active lanes, supply missing-lane identities, or read a hardware active mask. CUDA's source
shuffle and PTX contracts require matching participants and defined source lanes. On SM70 and later,
matching synchronized shuffles can rendezvous across divergent instruction locations; the older
same-instruction convergence restriction applies to SM6x and below. NVIDIA's own divergent-branch
example corroborates that reading of PTX. See [CUDA 12.9 shuffle rules](https://docs.nvidia.com/cuda/archive/12.9.1/cuda-c-programming-guide/index.html#warp-shuffle-functions),
[PTX 8.8 shfl.sync](https://docs.nvidia.com/cuda/archive/12.9.1/parallel-thread-execution/index.html#data-movement-and-conversion-instructions-shfl-sync)
and [NVIDIA's divergent shuffle example, Listing 4](https://developer.nvidia.com/blog/using-cuda-warp-level-primitives/).

The frozen SM80 PTX retains four full-mask indexed shuffles in each of the four branches, including
constant predicates. Every participating source quad is complete, and each live lane executes the
same four corresponding shuffle modes/masks. Its divergent behavior therefore has a documented
SM80 basis; the CUDA pass need not be explained by undocumented maximal reconvergence. This does
not generalize to a lane skipping the sequence while still needed by its quad, partial-quad exits,
arbitrary mismatched shuffle sequences, or older SM6x hardware. Those cases were not launched and
have no promoted output oracle. Complete-quad exits before the sequence are different: exited lanes
are ignored by the rendezvous and no surviving quad reads them.

The provider maps `SLANG_NVVM_VALUE_OP_WAVE_READ_LANE_AT` to LLVM's synchronized indexed shuffle,
appending clamp 31. `_writeLegacyNVVMAssembly` validates the declaration's exact types and
`convergent`, `inaccessiblememonly`, `nounwind` attributes. NVVM IR documents synchronization and
source-lane restrictions; LLVM 14's convergent attribute preserves control dependence constraints.
Neither is a whole-kernel maximal-reconvergence execution mode. No provider or ABI change is
suggested. See [NVVM 12.9 data movement](https://docs.nvidia.com/cuda/archive/12.9.1/nvvm-ir-spec/index.html#data-movement)
and [LLVM 14 function attributes](https://github.com/llvm/llvm-project/blob/llvmorg-14.0.0/llvm/docs/LangRef.rst).

Fresh smoke passes 4/4 before other GPU work. The untouched frozen source rerun has exactly three
cells: NVRTC correct and the two unchanged direct preflight outcomes. The independently derived
research oracle enumerates all 16 four-lane Boolean truth tables, rotated across quads. A first set
uses 384 unchanged input/expectation pairs: 4/16/32-thread blocks, complete-quad exits including low,
high and alternating groups, ordinary calls, runtime divergent loops before common calls, and a
noinline helper. Loop parity follows directly from odd multipliers and odd increments, without
copying a shuffle algorithm. Public NVRTC and explicit-shuffle controls at NVRTC/direct O0/O3 each
pass every pair. A second set adds 128 pairs for quad-aligned and within-quad divergent Any/All
call sites, both inline and noinline, in 16/32-thread blocks. PTX retains conditional branches and
helper calls. Those four configurations also pass every pair.

In total, 2,048 launches match 131,072 independently expected output words; all 96-word input
regions and inactive/unwritten output sentinels remain intact. There are 512 shared input/expectation
pairs, 2,048 raw output buffers and 3,072 retained binary evidence files. Public and control are
explicitly distinct source families; comparisons across modes within each family use identical
source/input/oracle. Public direct compilations still reject the marker. Fourteen PTX artifacts
assemble, eight of which are the runtime public/control artifacts. The divergent test intentionally
shows CUDA's four-source-lane behavior: on split-by-lane parity, an Any caller can read a true value
from a lane taking the All branch. This must not be silently replaced by active-branch-only voting.

There are no production helper/fallback/special-case changes to inventory. The proposed future
recipe must preserve canonical bool signatures, exact target helper identity, both requirement
markers and existing synchronized shuffle semantics. Reuse the existing intrinsic lookup and typed
operation infrastructure; do not rebuild syntax, synthesize hardware masks, globally ignore
requirements, or treat markers as executable barriers. A negative standalone-marker test should
retain rejection. A marker-only patch fails the marker-free helper controls and cannot be accepted.
The next bounded slice can implement this helper contract with the unchanged frozen test and a
registered dynamic fixture, using the research buffers as exact replay evidence.

All 555 registered input hashes, 30 tested source hashes and 12 artifact hashes preserve accepted 233.
Source revision and commit are both actual `f77142a2e8d54b66482dc85b4e99e2a6e28de777`; compiler
library is `92ae81d069aeda9a6ff2a61edec43f572b2af02bb7ec677fc444490ea9a966f1`, provider remains
`ae0e7859f6062a69f191cace4ae8d96340c074815866808c5b43467f41144372`, ABI 36. Native Ubuntu/L4 SM89,
driver 580.126.09, target SM80, CUDA 12.9.2/NVRTC 12.9.86 and LLVM 14 are unchanged. No rebuild,
production mutation, device loss, system change, commit or push occurred.

Cumulative acceptance remains 233: 1,677 cells, 1,630 correct, 47 unresolved and ten resolved
histories; its 642 fresh and 1,035 inherited full 229 cells are historical acceptance, not a fresh
full research run. Frozen 452/1,356 and discovery 107/321 identities/cells are unchanged. Latest full
checkpoint remains 229 and implementation cadence remains two. Material was reconsidered: all six
compile/assembly passes are inherited, with application binding/texture/LUT/input/oracle still
absent; there is no material runtime/performance claim. Matrix layout, texture dimensions and
other arithmetic gaps remain independent. Research stops at the proven quad helper boundary.

Independent parent acceptance verified 3,246 unique evidence files, all unchanged source/artifact/
registered-input hashes and the three exact frozen outcomes. A separate Boolean oracle recomputed
all 512 expectations directly from raw predicates and trip-count arithmetic, then matched every
complete GPU buffer across 2,048 launches, including input regions and inactive sentinels.
Research 234 is accepted; implementation cadence remains two.
