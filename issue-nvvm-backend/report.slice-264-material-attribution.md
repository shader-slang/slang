# Explain material compile time and retained local storage

## Motivation

Material evaluation is near NVRTC compile-time parity and sampling is slower; direct NVVM uses
67/86 registers and 784-byte stacks versus 48/63 registers and 592/624 bytes. Monday needs an
explanation supported by measurements and a general optimization direction, without promising a win.

## Proposed solution

Instrument existing host profiler boundaries temporarily, qualify unchanged output, and measure
the fixed material protocol. Trace stack fields and retained arithmetic back to canonical LLVM
input. Test a small aggregate probe and a storage-separated control before selecting an optimization.
The research selects **no production change**: serialization is a sub-percent opportunity, while
aggregate optimization needs a stronger differential reproducer and interprocedural work.

## Change summary

[Evidence264](timing-evidence.slice-264.json) records identities, qualification, independent timing
audit and resource/probe observations. [The experiment](experiments/material-attribution/README.md)
retains the exact temporary patch and two reproducible shaders. The design note records durable
findings; STATUS/HISTORY link this research. Raw samples, binaries, logs and audits stay under
`build/nvvm-material-followup`. The next finite slice updates the presentation/reporting package.

## Concepts and vocabulary

Vendor compile means the `nvvmCompileProgram` or `nvrtcCompileProgram` API call, excluding library
loading, NVVM verification and host emission. Shared work here means three disjoint intervals:
builtin-module loading, front-end execution, and Slang IR linking/optimization. Percentages and
residuals are calculated per sample before aggregation. Stack is local storage, not synonymous with
register spills; PTX instruction counts are static and do not measure executed work or GPU speed.

## Process report

The instrumented build is workspace f9532f06e plus the saved six-file profiler patch, provider ABI42,
LLVM14, CUDA12.9.2/NVRTC12.9.86, SM80 on L4. Host scopes leave provider linkage and semantics intact.
Qualification preserves all 42 material/quality PTX and cubin hashes/resources and passes 139/139
relevant downstream/serializer/PCH units. The repeated run completes 132 compiles and 66 assemblies;
independent review rehashes every artifact against263, checks 4,400 raw timer values and 2,420 required
scope counts, and confirms containment. Every output and resource record is unchanged. No sample is
removed or retried. This is instrumentation qualification, not a new full correctness checkpoint.

NVVM O3 shared work occupies 76.4%/72.4% of fresh wall time (evaluation/sampling), while libNVVM
compilation occupies 13.3%/17.7%. Its API medians are 185.44/283.50ms versus NVRTC 200.65/207.07ms.
NVVM host preparation/emission adds 36.62/45.41ms, with vendor verification another 10.43/11.88ms.
Evaluation's vendor advantage is consumed by other backend work; sampling's vendor call itself is
slower in both rounds. New wall medians are 1404.23/1566.61ms versus NVRTC 1399.09/1429.46ms. These
are a separate instrumented session, not a measured regression from263's 5% sampling gap.

`NVVMIRBuilder::serializeModule` queries size and then writes. Each provider invocation calls
`_materializeModule`, repeating verification and LLVM-to-NVVM text conversion. The second call takes
7.96/9.20ms, 0.57%/0.62% of wall time, including necessary copying. A one-shot immutable output API
could remove duplicate work, but requires ABI/lifetime/failure coverage; it is not a promised saving.

Evaluation retains the complete graph created by `make_material_instance`. CUDA's compact float3
layout produces 592 bytes; direct NVVM's ordinary `<3 x float>` allocation/alignment of16 produces
784 bytes. Field offsets in final PTX corroborate the reconstructed layout. Sampling's NVRTC frame
has 32 further bytes not attributed here. All four O3 modules have zero calls and zero spills.
The layout choice explains stack growth, not the exact physical register delta.

NVVM stores zero absorption and false retroreflection, then reloads them after dynamically indexed
array writes. It retains six exponential instructions per material entry; NVRTC retains none.
This establishes extra arithmetic/live values, but does not assign registers to individual paths.
The LLVM input is canonical: typed struct field GEPs already use `inbounds`; array offsets are
conservatively plain. A complete graph crosses a helper return before its retained subset is
extracted. No malformed producer shape or safe one-line metadata fix was found. Slang SSA does not
promote derived field stores, and existing field-key alias analysis should be reused by future work.

The general probe has independent outputs29,29,33,33 and passes all three runtime modes. Both O3
backends retain six exponential instructions in its constant entry, so it is **not** a differential
reproducer. Masking the index at the store changes neither result (compile/assembly-only probe).
Separating local payload from array storage passes three more runtime modes, eliminates the constant
exponentials, and changes NVVM registers21→18 and stack80→32 bytes; its nonzero control still retains
exponential work. This source-level counterfactual is not a compiler optimization or material win.

Helper inventory: temporary profiler scopes/includes only; no new compiler helper, fallback,
alias rule or special case remains. The exact patch was reversed and the full accepted262 bin/lib
layout restored; all 27 recorded identities match. The instrumented layout remains separately under
ignored build. The final checked-in probes also pass6/6 on the restored compiler. Independent closeout review
confirms identities, hashes, units and oracles without findings. Accepted262 and implementation
cadence0 remain unchanged. No full checkpoint was
needed for a discarded measurement-only source change. Research is complete; only the authorized
results refresh follows, then stop and notify the maintainer.
