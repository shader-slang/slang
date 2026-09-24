# Slice 204: Integer and vector texture dimension queries

Status: accepted with a full checkpoint on 2026-09-24.

## Motivation

Both tiled-brass entries stopped in `render.TextureHandle.resolve_udim` when querying the geometry
of their integer indirection texture. Consider this complete reduced user-code path:

```slang
DescriptorHandle<Texture2D<uint>> inputTexture;
RWStructuredBuffer<int2> outputBuffer;

[noinline]
int2 dimensions(uint64_t handle)
{
    Texture2D<uint> texture = DescriptorHandle<Texture2D<uint>>(handle);
    int2 size;
    texture.GetDimensions(size.x, size.y);
    return size;
}

[numthreads(1, 1, 1)]
void computeMain()
{
    outputBuffer[0] = dimensions(uint64_t(inputTexture));
}
```

The material's shader is unchanged. The focused executable fixture uses its descriptor reconstruction
and signed output boundary, binds actual textures, and independently expects their configured sizes.
It also covers Float32/Int32/UInt32 scalar, two-lane and four-lane texels with both signed and unsigned
dimensions. One-texel textures cover the smallest geometry, and 4/8-sized textures distinguish
bindings. Additional 1D, 3D, cube and array queries preserve shape semantics and explicitly verify
CUDA's zero result for array counts. This proves the query feature, not full material execution.

## Proposed solution

Let the existing read-only texture classifier define eligible query resources. Remove the resolver's
additional scalar-Float32 filter and admit signed as well as unsigned local i32 output parameters.
The provider validates queries against its existing numeric texel domain, shared with fetches.
The query intrinsic already consumes only the texture handle and returns i32, so neither sampling,
numeric conversion, provider interface layout nor resource representation needs changing.

## Change summary

- `source/slang/slang-emit-nvvm.cpp`: broaden the existing exact dimension-helper resolver using
  existing canonical texture and signed/unsigned i32 predicates; document a concrete example.
- `source/slang-llvm-nvvm/slang-llvm-nvvm.cpp`: use the existing numeric texel classification for
  width/height/depth queries. Rename the predicate from fetch-only wording and remove the obsolete
  scalar-Float32 predicate. Fetch, sample and gather behavior is unchanged; provider ABI35 stays.
- Unit builder and fake-provider support: mirror the contract and extend the existing query shape
  matrix to all nine eligible texel families without adding a second production classification.
- `tests/cuda/nvvm-texture-dimensions.slang`: real-output oracle in all three modes; selected once
  as a disjoint discovery addition. Existing CUDA dimension source remains reused unchanged.
- Query negatives: replace the former vector-texel rejection with float-output, mip and multisample
  boundary checks at O0/O3. The former float2 case is explicitly covered by the positive runtime
  fixture. The neighboring fetch test only corrects its stale diagnostic prefix.
- Full outcome manifests, comparison/provenance, completed plan, design note and STATUS preserve
  the checkpoint and the next independent blocker.

## Concepts and vocabulary

A **query operation** asks for texture-object geometry; its result does not depend on the numeric
format returned by a texel fetch. **GenericAsm** is the finalized standard-library CUDA helper's
assembly template and signature, which jointly identify the operation accepted by this resolver.
A **dimension output** is an `OutParam<int>` or `OutParam<uint>` local pointer; both lower to i32
storage. **Array-count zero** is the explicit CUDA prelude behavior for unavailable array-size
queries, distinct from width and height queries on the array texture itself.

## Process report

`TextureTypeInfo::writeGetDimensionFunctions` generates overloads for float, int and uint dimension
outputs. For the example above it emits the canonical CUDA helper containing `txq.width.b32` and
`txq.height.b32`, with two `OutParam<int>` arguments. This is an intentional, valid producer shape.
The source's descriptor handle conversion already belongs to slice202's accepted representation.
The existing `getNVVMSupportedReadOnlyTextureType` validates read-only, non-MS/non-shadow resources,
selected shapes and 32-bit numeric texel families; it is the semantic source of truth.

`_resolveNVVMTextureDimensionsGenericAsm` recognized the exact assembly but unnecessarily required
scalar Float32 texels and unsigned dimensions. Removing that redundant element restriction and
using `isNVVMSignedI32Type` alongside the existing unsigned predicate admits the canonical overload.
The exact assembly, shape, arity, local numeric pointer and OutParam checks remain. Floating outputs
still need numeric conversion and are rejected; mip helpers carry a different signature/template,
and MS resources fail existing resource eligibility. Those are deliberate independent boundaries.

The query requirement passes the original texture element descriptor to the provider unchanged.
`_isTextureOperationSupported` previously repeated the scalar-Float32 restriction; it now reuses
the existing numeric domain used by fetches. `_getTextureIntrinsicID` maps the query operation to
`nvvm_txq_width`, `nvvm_txq_height` or `nvvm_txq_depth`. `_emitTextureOperation`'s `!hasCoordinate`
branch calls that intrinsic with only the i64 handle. No texel element value reaches the call and
no typed sample instruction is selected. The emitter's existing dimension output store writes the
i32 result with alignment four through either integer pointer. Its trailing-zero path uses the
actual output value type, so signed array-count outputs require no additional special case.

Helper/fallback inventory: the existing query resolver loses one redundant classification and gains
an existing signed-i32 predicate; the provider and fake provider reuse one numeric classification;
the existing unit matrix expands its input domain. All survive the input-shape audit. There is no
new helper, custom equivalence, arbitrary operand walk, syntax reconstruction, fallback, shader
rewrite, provider ABI extension or shared lowering change. Fixing the producer would be wrong here:
it already intentionally represents the source overload and its valid pointer roles.

The final positive fixture was written and run before compiler/provider edits. NVRTC O3 passed all
independently expected outputs; NVVM O0/O3 rejected the same dimension GenericAsm shape as the
material. Its SHA256 remained byte-identical through final validation. That direct before/after
experiment supplies the removal proof without another full source revert/rebuild. Both resolver
and provider restrictions matter: removing only one leaves the other rejection boundary intact.

The first negative-harness run requested no PTX output and consequently did not trigger code
generation. Adding explicit output paths fixes that test setup. The pre-existing fetch negative
expected a quote immediately after `GenericAsm`, but `_diagnoseUnsupportedGenericAsm` already
appends `assembly=...` and `signature=...` at the base revision. Its entire implementation is
byte-identical before/after (retained audit and hash); the corrected expected `tex1Dfetch_int` prefix
is distinct from new query support. Neither fetch admission nor its code generation changes.

The final 14 focused checks, four runtime fixtures, 473 units with one platform skip, and 18 toolkit
cells pass. The provider
admission change triggers a full checkpoint under WORKFLOW, even though the code change is bounded.
Full runtime inventory and exact preservation results are recorded in `runtime-validation.slice-204.json`.

| Corpus                                       | Identities | NVRTC O3 correct | NVVM O0 correct | NVVM O3 correct |
| -------------------------------------------- | ---------: | ---------------: | --------------: | --------------: |
| Frozen full selection                        |        452 |              449 |             438 |             438 |
| Discovery full selection, including addition |         85 |               75 |              75 |              75 |

All 1,611 runtime cells are fresh: 1,608 previous cells and three additions. The comparison uses
both the optimized full checkpoint and slice203's accepted overlay/addition. All 1,547 previous
correct results are preserved; the three new cells bring the total to 1,550. All 61 known failures
retain their exact classification, return code, execution count and diagnostic. There are zero
outcome/diagnostic deltas, missing cells, duplicate cells or inherited runtime outcomes. Accepting
this full checkpoint resets the implementation-slice cadence to zero.

The tested base is `ff3c679909428fd963787de74abc5cfb72faa6f3` plus exact source hashes in the manifest.
Final compiler SHA256 is `cb187104da2ae3699c93d7189cd893a142b4bdc2bea52fcc9a1eb08000dedb09`;
provider SHA256 is `1f3ef9bd03de64838dc039a97ec30f4fe00cd33682d0d95b06abac895446124f`.
Final source and artifact hashes were rechecked after all gates. The L4 remained healthy, with
4 MiB allocated after validation. Raw commands, logs and audits stay in ignored
`build/nvvm-loop/slice-204-before` and `build/nvvm-loop/slice-204-after`.

All six complex support cells were reassessed. NVRTC still compiles/assembles both entries. All four
direct cells now reject `call argument type: OutParam<mtlx.BSDF> -> BorrowInOutParam<mtlx.BSDF>`.
A minimal matching source boundary is `mx_layer_bsdf(..., out BSDF result)` invoking the mutating
`result.set_layer(...)`; the callee's `this` parameter is BorrowInOutParam. The retained slice203
IR corroborates that exact type relation, while the new six-cell logs are fresh diagnostic evidence.
This is a separate helper-argument feature and is handed off without implementation. Missing
material bindings, texture/LUT fixtures and output oracles still preclude material runtime or
kernel-performance claims.
