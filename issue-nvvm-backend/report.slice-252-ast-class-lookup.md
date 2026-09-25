# Inline the canonical AST class lookup

## Motivation

Research251 repeatedly sampled the class lookup after accepted246 inlined its subtype predicate.
Both intact tiled-brass entries already compile and assemble in NVRTC O3 and NVVM O0/O3. Consider
this existing material helper:

```slang
SurfaceInteraction make_surface_interaction(float2 uv, float3 wi_ws)
{
    SurfaceInteraction si = {};
    si.uv = uv;
    si.wi_ws = normalize(wi_ws);
    si.normal_ws = float3(0.0, 0.0, 1.0);
    si.front_facing = dot(si.wi_ws, si.normal_ws) >= 0.0;
    si.shading_frame_ws = Frame::identity();
    return si;
}
```

The material contains many generic and overloaded calls; the samples do not attribute all work to
this helper. The observed `inferGenericArguments -> as<CallableDecl>(genericDecl.inner) ->
NodeBase::getClass -> SyntaxClassBase(ASTNodeType)` path motivates one bounded visibility experiment.

## Proposed solution

Prototype the existing constructor inline while retaining the sole generated metadata table.
An incomplete internal `extern` declaration preserves `Slang::kAllSyntaxClasses`, including six
existing NatVis references. The generated definition deduces its own length; a static assertion
independently equates that length to `ASTNodeType::CountOf`. The constructor retains both bounds
and indexes the same table. Predicate and cast implementations remain untouched.

The minimal prototype passes all predeclared performance gates: semantic medians fall 16.86–18.89%,
the sum of the six wall medians falls 7.35527%, and all 12 cell/round wall medians improve. All 264
PTX outputs and 24 independent support assemblies remain exact 250/251. Full correctness acceptance
preserves all1701 runtime cells and passes independent parent acceptance. No alternate
optimization was added.

## Change summary

- Two compiler files implement constructor visibility and internal table linkage/count agreement.
- `check-ast-subtype.py` and its template extend the existing standalone proof for exact typed
  metadata identity, factory/destructor policy and matching Debug invalid-tag diagnostics.
- The plan, compact evidence, this report, design note and STATUS record the final decision and
  distinguish fresh results from inherited accepted250 runtime evidence.

## Concepts and vocabulary

- **Canonical node tag:** ASTBuilder installs the generated `T::kType` before consumers see a node.
- **Class metadata:** a class's existing `kSyntaxClassInfo` holds name, range, visualization kind and
  create/destruct callbacks. One generated table maps tags to these exact metadata objects.
- **Internal linkage versus visibility:** sharing a C++ declaration between compiler translation
  units changes linkage, but the configured hidden symbol visibility keeps the table out of exports.
- **Inclusive phase timer:** named compiler timers can contain other named timers; they are not
  disjoint costs and must not be summed as such.

## Process report

Starting revision is accepted251 `0c460441c0d539ad2f09f3139ce620a8e6a12dd3`, branch nvvm-backend.
Before edits,119 source/generated/test,12 artifact and 563 input hashes match accepted250. Native
Ubuntu24.04, L4SM89 driver580.126.09, targetSM80, CUDA12.9.2/NVRTC12.9.86, isolated LLVM14.0.6,
providerABI41 and RelWithDebInfo remain fixed. The local slang-build skill and 203 environment are
used; Debug and optimized objects are built separately. At most4 CPU workers run, with 2 unit servers
and sequential GPU suites. Performance has no competing owned build/benchmark/GPU workload.

`ASTBuilder::_initAndAdd -> NodeBase::init` already installs the canonical tag. The parser constructs
call expressions through ASTBuilder, and ordinary expression checking reaches overload candidate
selection and `inferGenericArguments`. Its inner-declaration cast obtains class metadata through
`getClass`. That metadata serves subtype checks and other reflection; `ASTBuilder::~ASTBuilder`
also uses `getClass` to dispatch destructors for its registered nontrivial nodes. These are intentional
canonical values. There is no malformed spelling to repair, no source syntax to reconstruct and no
substitution or custom equivalence to introduce. The prototype exposes an existing operation at
its actual producer-to-consumer boundary.

The production helper/fallback/special-case inventory contains only the existing constructor
relocation, one shared internal declaration and one compile-time count assertion. The same generated
initializer remains authoritative. No alternate hierarchy, direct-tag cast, dispatcher, allocator,
semantic cache, solver, fixture or oracle changes. Pointer/reference layout and public headers/API
remain untouched. The constructor, internal declaration and count assertion all survive the completed gates; no fallback or special case is retained.

Before changes, matching Debug and optimized proofs each pass 702 tags,492804 independent C++
inheritance pairs,636 typed factory objects and 66 abstract classes. The 702 metadata records agree
across configurations. Both invalid Debug tags(-1 and 702) produce the actual constructor-bounds
message and SIGABRT. The harness prints Slang InternalError's Message and rethrows it because Slang's
exception does not implement `std::exception::what`; an unrelated abort cannot pass. Optimized objects
never receive invalid tags. Exact linked input hashes and commands are retained.

Fresh baseline units pass 1047 with 13 skips (1060 identities), including the real copied sibling
CapabilityGenerator; all 514 relevant accepted250 unit outcomes match. Semantic coverage passes 1052
with 77 skips (1129 identities) across generics, overloads, operator overloads, diagnostics and
serialization. No compiler change is used to satisfy proof expectations.

The candidate Debug and optimized builds pass the same exhaustive proof and both invalid-tag
checks. All 702 metadata records agree across all four before/candidate/configuration proofs. The
before generated proof hash differs from the candidate only by formatting (include order and comment
wrapping); the exact diff is retained. Do not confuse metadata equality with equal proof-source hashes.
The small runtime gate passes all 4 cases. The table is 5616 bytes (702 pointers), GLOBAL HIDDEN in its
object and LOCAL in the final library, with no export additions/removals. NatVis is unchanged; its
source name/scope is preserved, without claiming a Windows debugger execution on this Linux host.

The initial pre-timing audit stopped on `slang-ast-support-types.h.fiddle`: adding seven header lines
shifts the generated FIDDLE source-line macro names by exactly+7. The corrected audit proves that
specific transformation byte-for-byte; generated metadata and enum files remain unchanged. The
initial script/log remain retained. No production code was changed to satisfy this audit, and no
benchmark had run. A later local loader-oracle audit omitted `_buffer` from an identity and raised
StopIteration; correcting the exact identity confirms both outputs match251. That preparation error
also remains recorded and did not trigger a benchmark retry.

Complete isolated before/candidate bin/lib layouts retain their actual libraries, builtins and
providers. Loader traces prove each layout uses its own copies. The measurement uses2 warmups and 9
samples per cell/build in each of 2 rounds, reversing identity and build order. Piped
Popen.communicate measures through process exit, logs are written afterward, child timeout180 seconds
and suite bound30 minutes. All 216 measured compiles and 48 warmups are retained without retries or
outlier exclusions;24 separate assemblies establish support, not an assembly performance claim.

| Identity            | Semantic before → candidate, ms | Wall before → candidate, ms |
| ------------------- | ------------------------------: | --------------------------: |
| eval / nvrtc / o3   |                 497.25 → 403.30 |           1456.79 → 1338.90 |
| eval / nvvm / o0    |                 498.27 → 413.40 |           1424.27 → 1312.44 |
| eval / nvvm / o3    |                 491.23 → 402.86 |           1521.47 → 1394.26 |
| sample / nvrtc / o3 |                 488.47 → 405.57 |           1460.19 → 1364.21 |
| sample / nvvm / o0  |                 489.07 → 405.32 |           1449.67 → 1340.35 |
| sample / nvvm / o3  |                 487.39 → 405.24 |           1606.66 → 1512.88 |

Before eval/NVRTC O3 has round wall medians 1540.04/1442.08ms, with variation in builtin loading,
semantic checking, IR and output. Its candidate medians are 1338.87/1338.94ms. Candidate sample/NVVM O3
has1543.21/1495.00ms round medians. Causes are not established; all samples remain, all six pooled
wall medians improve5.84–8.36%, and both rounds pass unchanged gates. Inclusive phases are not additive.
The compiler file shrinks 82416 bytes, and its actual ELF `.text` shrinks 67616 bytes; neither observation
is an instruction-count or kernel-speed claim.

After timing, the proof driver's module-doc example is corrected to name the final inline-header
assertion source. Its executable logic is unchanged, and the exact one-string replacement and
before/after hashes are retained. Every production/generated compiler source, measured artifact and
runtime input remains the measured candidate.

The final full checkpoint preserves every five-field outcome for 1701 cells:1662 correct and 39
known unresolved. Frozen 452 identities/1356 cells retain 1347 correct; discovery 115 identities/345
cells retain 315 correct. All 39 unresolved histories and 18 resolved histories survive; no additions,
missing/duplicate cells, lost passes, diagnostic changes or baseline reset. Known texture and
column-major wrong outputs remain unchanged. These fresh registered cells do not relabel historical
independent rawLLVM/BF16/FP8/literal/helper controls linked through accepted250.

All 1060 compiler-unit outcomes match the fresh baseline (1047 pass,13 skipped), including all 514
relevant accepted250 outcomes (513 pass,one skip) and `doubleSourceLiteralsRoundTrip`. All 1129
semantic outcomes match (1052 pass,77 skipped). Toolkit 18 and runner contracts 6 pass. The final 6
material compiles/assemblies exactly match250/251, without runtime dispatch. The 120 final source
snapshots,12 artifact identities and 563 inputs are retained; actual measured compiler library SHA256
is `10ffeb3246d56c9a835b1cd606e35a1a2cd6c8f9fcb3f6bfef36fed26413ea7b`, provider unchanged at
`5fe0b977e22b80acc5ee39147c69510a01c09563354a1a67bd9573d1cda1aeab` on ABI 41. The final helper
inventory records all three production items retained with their responsible-layer proof. No new
correctness blocker was exposed by this slice. Independent parent acceptance is complete: all timing
statistics and artifacts,1701 outcomes, failure histories, unit/semantic identities, four proofs,
120 snapshots and4224 indexed artifacts pass. The four parent audit artifacts are referenced
separately from the closed worker index; all226 compact references verify.

The authoritative accepted full252 checkpoint contains1701 cells,
1662 correct,39 unresolved and 18 resolved histories, with latest targeted233 and cadence 0. No
material runtime or kernel-speed claim is possible without bindings, textures/LUT inputs and an
expected-output contract. The parent accepts the local commit; no worker commit,
push, driver/system change or reboot occurs.
