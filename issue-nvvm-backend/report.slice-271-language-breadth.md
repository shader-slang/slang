# Qualify four existing language interactions

## Motivation

Corpus counts do not tell us whether useful language interactions run on NVVM. After fixing FP8/BF16
records, probe four existing tests outside the main580-case selection: switch fallthrough across a
loop, a captured lambda passed into a mutating array map, tuple mutation/swizzling, and nested defer
execution around a typed exception.

## Proposed solution

Run each unchanged source at NVRTC O3 and NVVM O0/O3 using maintained discovery directive adaptation,
source mirroring and census execution. Preserve the existing source oracles, and inspect actual GPU
output independently. All twelve cells pass; no compiler or harness implementation change is needed.

## Change summary

[Evidence271](research-evidence.slice-271.json) retains the four source hashes/native ordinals, twelve
exact outcomes, independently checked output buffers and accepted270 provenance. The completed plan,
STATUS/HANDOFF and HISTORY describe the research result and next action. Raw scripts, mirrored sources
and execution logs remain under `build/nvvm-breadth271`. Main manifests and discovery capacity remain
unchanged; these four sources are focused evidence outside the main corpus.

## Concepts and vocabulary

A native test ordinal counts every harness directive, including directives not selected here. Only
the selected CPU comparison directive is adapted to CUDA; shader bodies, input data, output format,
shader-object flags and inline CHECK prefixes remain unchanged. A passing unanchored CHECK can be
weaker evidence than inspecting the actual output at the intended index.

## Process report

The selected CPU ordinals are3/1/2/2 for switch/lambda/tuple/defer. Maintained helpers select those
exact directives and strip other test directives while preserving source bodies and all inline
oracles. None has an expected-output sidecar. The ignored focused driver constructs four workloads
without replacing manifest entries, padding the inventory or changing discovery's128-source limit.
Modes run sequentially with one worker and retries disabled.

| Interaction                                                  | Independent expected result            | Observed in all three modes |
| ------------------------------------------------------------ | -------------------------------------- | --------------------------- |
| Switch loop state falls through into case1                   | 15+100, direct100, case200, default-1  | `[115,100,200,-1]`          |
| Captured scalar2 multiplies local element4                   | Selected result8                       | `[8,0,0,0]`                 |
| Inout tuple swizzle writes uint4, then reordered extraction  | First result4                          | `[4,2,3,4]`                 |
| Nested defers run in reverse order across normal/throw paths | Normal2/4; catch255, nested128, outer3 | `[2,4,255,128,3]`           |

Independent review found the tuple's original `CHECK: 4` can match the unchanged fourth input element.
The original oracle is preserved, but all three actual buffers independently confirm the first element
changed to4. Lambda observes one mapped element, not every local array element. These are specific
interaction proofs rather than complete feature coverage.

Each of twelve unique cells has return0, passed/executed1/1, ignored0 and empty diagnostic/shape.
Lead and independent review verify actual outputs, generated body hashes and mode inventory. All37
runtime artifacts,22 dependency pins and576 main source hashes remain unchanged. No new compiler
helper, fallback or input representation exists to audit. The only execution incident was sandbox
initialization before read-only inspection; approved local execution then completed without test
failures. The21.65-second experiment duration is operational metadata, not a performance benchmark.

Accepted270 remains the full correctness baseline:1,703correct/37unresolved,20resolved histories;
last full270 and implementation cadence zero. Those outcomes are inherited, not rerun. This probe
finds no blocker requiring implementation. Next, refresh material profiling and code-quality evidence
before choosing another material-driven optimization. The development loop continues; Slack stays
skipped at the user's request.
