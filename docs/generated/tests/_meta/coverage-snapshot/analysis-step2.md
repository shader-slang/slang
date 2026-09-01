# Step 2 analysis — variant-matrix doc-driven expansion

**Goal of this step:** create depth by expanding _target-general construct
variants the design docs already enumerate_, under the `_expand.md` contract
(author from the doc, never from coverage lines; coverage only _ranks_ which
bundles are thin).

## What shipped

- New methodology rule committed (`60b4124dc`): `_claims.md §2` "Construct &
  legalization variants (depth)" — doc tables / per-type / per-op enumerations
  become one test per documented branch, branch-driven not Cartesian.
- A doc-driven expansion workflow (`.claude/wf-expand-variants.js`) run over the
  **10 thinnest design bundles** (ranked by `expansion-candidates` from the
  step-1 coverage report — contract-safe, bundle-level only): `pipeline/05-ir-passes`,
  `target-pipelines/{hlsl,cuda,spirv,metal}`, `cross-cutting/targets`,
  `pipeline/04-ast-to-ir`, `name-resolution/visibility`,
  `syntax-reference/keywords-and-builtins`, `pipeline/overview`.
- **+41 tests** across 10 per-bundle commits (`9b1bbc5d9`…`12bfb711e`), all
  `intent=expansion`, CHECK lines pinned to real slangc output. verify FAILED:0,
  lint 0/0 everywhere. Examples: `05-ir-passes` +3 documented pass rows
  (array-return-by-ref, reinterpret, defer); `hlsl` matrix composite-select +
  read-only structured-buffer-of-matrix; `cuda` SV\_\*→threadIdx/blockIdx +
  active-mask synthesis; `metal` +12.

## Coverage result

|                | line cov   | covered lines | vs step-1 floor | vs baseline          |
| -------------- | ---------- | ------------- | --------------- | -------------------- |
| baseline       | 53.10%     | 134,016       | —               | —                    |
| step-1 (floor) | 52.00%     | 131,242       | —               | −2,774               |
| **step-2**     | **52.06%** | **131,410**   | **+168**        | **−2,606 (−1.04pp)** |

Floor re-recorded upward to **131,410**.

## The key finding (this is what to read)

Doc-grounded expansion of 10 bundles produced **41 high-quality tests but only
+168 covered lines**, and the agents — working strictly from the docs — found
only **2 doc gaps** total. That is not under-performance; it is the **doc
ceiling**. The agents correctly expanded everything the docs enumerate and
_stopped_, because the contract forbids inventing claims the docs don't support.

**Conclusion: the remaining ~2,600-line gap is doc-limited, not test-effort
limited.** The deep `slang-emit-spirv` (−610), `slang-ir-glsl-legalize` (−630),
`spirv-legalize` (−142) branches are driven by construct variants (per-type
array strides, the full atomic-op matrix, per-type/-shape emit forms) that the
terse design-doc sections (`spirv.md` Phase D, the GLSL emit path) **do not
enumerate**. You cannot write contract-valid tests for them until the _docs_
describe them.

This is exactly the wall Step 3 (doc-enrichment) exists to address — and it
confirms the earlier diagnosis: the missing methodology piece is not more
test-authoring effort but a rule that ties _doc breadth_ to compiler-path
coverage.

## Step 2 outcome

Recovered +168 lines to 52.06%; floor raised to 131,410. Remaining gap is
doc-limited → Step 3 takes the doc-enrichment route.
