# Qualify six accessor and generic language interactions

## Motivation

The corpus inventory showed sparse direct selection from properties, operators and extensions.
Six existing sources outside the main corpus exercise useful combinations: compound property updates,
properties satisfying interface requirements, multidimensional subscripts, default generic accessors,
variadic value packs and a constrained extension. File counts alone cannot establish their behavior.

## Proposed solution

Run the six original compute contracts at NVRTC O3 and NVVM O0/O3 through the maintained focused
discovery/census helpers. Preserve bodies, inputs, flags and original oracles; independently derive
and check every output word. All18 cells and72 output words pass. No compiler or harness change is needed.

## Change summary

[Evidence278](research-evidence.slice-278.json) records six source hashes/native selections,18 exact
outcomes, original sidecar identities and full output proofs. The completed plan and navigation describe
the result and next action. Raw/generated sources and logs remain under `build/nvvm-accessor-generics278`.
Main manifests, accepted277 compiler/provider bytes and discovery's128-source limit are unchanged.

## Concepts and vocabulary

A native ordinal identifies the selected authored harness directive. A typed output prints decimal
scalars; the untyped buffer writer prints hexadecimal words. A passing inline CHECK can cover fewer
values than the full observed output buffer. The source-derived oracle is independent of both backends.

## Process report

The six selected ordinals are0/0/0/0/1/2. The shared directive enumerator selects them before the
maintained adapter removes original API flags and adds CUDA. It preserves shader-object and output
format flags, the extension's `COMPARE_COMPUTE_EX` command, all input/CHECK declarations and three
original sidecars. Mirrored source bodies and sidecars match exactly. Modes execute sequentially with
one worker, no retries and a30-minute process-group bound.

| Interaction                                                         | Independent result in every mode |
| ------------------------------------------------------------------- | -------------------------------- |
| Property setter, compound XOR and getter                            | Integer257,256,259,258           |
| Generic interface property through two concrete conformers          | Integer33,51,69,87               |
| Two-index subscript setter/getter                                   | Integer3,3,3,3                   |
| Default generic subscript with signed/unsigned indices and mutation | Integer107,10,211,307            |
| Expanded value-pack sum and pack count                              | Integer60,4,0,0                  |
| Constrained extension mutating a variadic generic object            | Float3,3,3,3                     |

For the interface-property example, input `v` initializes `MyCell` to `v+1` and `YourCell` to `v`.
The generic helper increments the selected property and returns it. The concrete field read remains
`v`, so the final expression is `16*(v+2)+(v+1)+v`, or `18*v+33`. The existing sidecar contains
hexadecimal21/33/45/57, matching decimal33/51/69/87. This is concrete generic specialization,
not runtime existential dispatch. The ordinary property sidecar is hexadecimal101/100/103/102.

The default subscript adds offset7 and its mutable bias. Writes through indices5 and6 set the bias
to199 and294, preserving later reads211 and307. The variadic expansion computes60 and count4;
the other two words retain their input zeros. The extension starts at0, adds3, and splats scalar3
into its float4 output. Full-buffer inspection supplements these tests' partial inline CHECKs.
The variadic source already has a synthesized CUDA pass in277's semantic suite; this experiment adds
explicit three-mode/full-buffer evidence. The multidimensional test retains its forced-early-inline
accessor annotations, and the extension's negative SPIRV directives are outside this selection.

All18 unique cells return0, execute/pass1/1, ignore0 and have empty failure diagnostic/shape.
All37 runtime artifact hashes,22 pins and576 main input hashes remain exact. Full277, targeted233,
cadence0 and the37 unresolved/20 resolved histories are inherited, not rerun. No new helper, fallback
or semantic representation exists to audit. Root and the reused worker perform separate read-only
evidence audits; fresh worker/reviewer thread limits prevent an independent-agent review claim.
The executed driver has a weak redundant absolute-key corpus-absence assertion. Pre-execution manifest
checks and final relative-key audits independently confirm all six sources are absent; original driver
bytes remain preserved.

No blocker emerges from these six interactions. Continue with a bounded probe of nested FP8/BF16 local
record composition, a documented boundary of270, before proposing any compiler widening. Material
profiling272 and rejected inheritance-cache lead274 remain closed; this is not a performance study.
