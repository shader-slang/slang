# Preserve signedness in CUDA firstbithigh

## Motivation

Consider values loaded at runtime:

```slang
uint unsignedValue = inputWords[index]; // 0x80000000u
int signedValue = int(unsignedValue);
uint unsignedIndex = firstbithigh(unsignedValue); // Expected 31.
uint signedIndex = firstbithigh(signedValue);     // Expected 30.
```

CUDA source execution previously returned 30 for both inputs because the unsigned helper applied
signed-negative complement. Research 222 establishes this independently over 96 unique 32-bit and 192
unique 64-bit values: 24 unsigned 32-bit scalar/vector output words are wrong on NVRTC, while direct NVVM and
all signed 32-bit/64-bit and unsigned 64-bit controls pass. The discrepancy predates the preceding FP64 admission.

## Proposed solution

Move the negative-input complement from `U32_firstbithigh` to `I32_firstbithigh`. The unsigned helper
now counts the actual unsigned word; signed-negative inputs complement before delegating. Zero
handling and the 31-minus-CLZ calculation are unchanged. This follows the existing 32-bit CPU and 64-bit CUDA
helper organization and leaves the already-correct direct provider untouched.

## Change summary

- `prelude/slang-cuda-prelude.h` moves the two-line signed normalization to its owning helper and
  explains each helper's rule.
- `nvvm-firstbithigh-signedness.slang` adds runtime-loaded 32-bit/64-bit scalar/vector boundary checks using
  precomputed independent integer expectations, registered as one new discovery identity.
- Full checkpoint results, completed plan/report, design and STATUS retain preservation and provenance.

## Concepts and vocabulary

For unsigned inputs, _highest bit_ means the highest 1 bit. For signed-negative inputs it means the
highest 0 bit, found by complementing within the operand width before the unsigned operation. The
all-ones uint32 result is the sentinel for no matching bit. Vector overloads map components to scalars.

## Process report

The helper/fallback inventory has no new production helper, fallback or special-case shape. The
existing signedness condition moves from a wrongly shared unsigned implementation to its signed
entry point. Standard-module `firstbithigh<T>` already produces the canonical typed intrinsic. CUDA
source emission uses `$P_firstbithigh($0)` to choose the corresponding prelude helper, and vector
mapping preserves each element's signedness. The producer representation is correct; the prelude
consumer owns this intrinsic's source behavior. No syntax reconstruction or downstream NVVM patch
is involved.

Previously U32 cast its input to int32 before testing negativity, so all unsigned inputs with bit 31
set were complemented. I32 depended on that behavior by delegating directly. The fix keeps unsigned
bits unchanged and moves that same test/complement to I32 before delegation. Signed zero/all-ones,
negative boundaries, and every 64-bit helper remain preserved. There is one unsigned CLZ implementation
rather than a duplicate signed algorithm.

The new fixture stores 32 rows of input words and expected unsigned 32-bit/signed 32-bit/unsigned 64-bit/signed 64-bit
indices in a runtime buffer. Expected values were generated with integer bit_length and explicit
width-bounded complement for signed-negative operands only. Each lane checks four scalar results
and eight vector components, including a distinct neighboring input. No expected result comes from
a backend run, no constant input is folded into the tested intrinsic, and no existing oracle changes.

Before the helper change, the final fixture fails NVRTC and passes both direct modes. Its exact
source/input hashes and all before artifacts are retained. Research replay uses the unchanged 222
shader, all 96/192 inputs, and exact expected output arrays. This establishes the responsible source
layer without making agreement with NVRTC the oracle.

Because the prelude is shared, acceptance requires a full checkpoint regardless of the targeted-slice
cadence. Select all 452 frozen identities explicitly from `census.slice-195.tsv`, all 102 discovery
identities, and all six material compile/assembly cells. Compare all 1659 old runtime cells with the
last full 220 plus accepted 221 fixes/addition; report the three new cells separately. Known failures
and their first-known records remain failures unless independently resolved. Material validation
continues to establish compilation/assembly only, not kernel correctness or speed.

Fresh-context delegation remains unavailable at the app's agent-thread limit. Parent local execution
and review follow WORKFLOW's fallback; no independent worker review is implied.

Fresh final gates pass: focused 4/4 plus the existing Windows-only skip, runtime smoke 4/4,
research 18/18, units 478/478 plus the same existing skip, toolkit 18/18 and discovery contracts 6/6.
The exact research 222 replay matches all 6,912 words, fixing its 24 previously different unsigned
32-bit results. Its source, inputs and expected arrays are unchanged.

Full frozen coverage has 1,356 cells: 1,335 correct, five known infrastructure failures and 16 known
preflight stops. Discovery has 306 cells: 276 correct and 30 retained failures. All 1,659 old cells
match their latest accepted classification, return code, complete execution counts, diagnostic and
canonical shape exactly; the three new cells pass. There are no missing, extra, duplicated or
inherited runtime cells. Cumulative full coverage is 1,662 cells, 1,611 correct and 51 known failures.
Every first-known failure record and all six resolved histories remain intact. All six material
compile/assembly cells pass; no material runtime or performance claim is made.

The initial summary assumed five focused passes from the requested selectors. The actual log has
four executed passes and `nvvmSlangIntegerBitHelpersRequestTypedOperations` ignored on Linux by its
existing Windows-only guard. The evidence checker rejected that count; it and the documentation
now record the skip explicitly. The real builder bit-operation test and all three runtime fixture
modes pass. This was a reporting correction with no source or test-result change or rerun.

Local parent acceptance verifies 138 evidence references, 24 tested source hashes, 12 artifacts and
550 runtime input hashes. All 549 old inputs and 101 old discovery manifest rows remain unchanged.
The fixture is byte-identical to its before-change run. Final compiler SHA256 is
`01e06def851b6228dea63d2bbb18cb4c3167ea89542d542623ea79e9d6f3258d`; provider ABI 36 remains
`ae0e7859f6062a69f191cace4ae8d96340c074815866808c5b43467f41144372`.

Accepted locally on 2026-09-25. Slice 223 becomes the latest full checkpoint and resets implementation
cadence to zero. Next isolate the generated KernelContext pointer preflight in the two registered
masked-prefix min/max workloads before considering another implementation slice.
