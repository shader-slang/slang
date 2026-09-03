# Slice 189: Carry zero-index bounds policy into direct NVVM IR

## Motivation

Consider this existing compute kernel:

```slang
total += byteAddressBuffer.Load<int>(-tid * 4);
total += structuredBuffer[-tid];
total += rwStructuredBuffer[-tid];
total += fixedArray[-tid];
outputBuffer2[tid + 1] = total;
```

Its test defines `SLANG_ENABLE_BOUND_ZERO_INDEX`. Generated CUDA expands that option through
`slang-cuda-prelude.h`, replacing every out-of-range index with zero. Direct NVVM consumes linked
Slang IR rather than generated CUDA, so before this slice the option had no effect and this was the
only healthy frozen-v1 workload that compiled and ran but disagreed with its NVRTC reference in
both optimization modes.

## Proposed solution

Materialize the selected policy in the existing direct-NVVM legalization boundary, before
capability preflight. Match only the four canonical access instructions produced by the kernel and
replace each index with ordinary typed comparison/select IR using an extent obtained from the
canonical resource or fixed-array type. This keeps the provider generic, leaves inactive compiles
unchanged, and avoids reconstructing source syntax or recognizing the fixture.

## Change summary

- Direct NVVM recognizes the exact `SLANG_ENABLE_BOUND_ZERO_INDEX` target option.
- Byte-address loads use their CUDA layout size and runtime byte extent; structured accesses use
  their runtime element count; fixed arrays use their literal element count.
- The existing compute test now has permanent direct O0 and O3 differential-runtime lanes.
- Frozen and discovery corpus snapshots, the capability ledger, and the backend design record the
  resulting coverage and ownership contract.
- Provider ABI revision 34 is unchanged.

## Concepts and vocabulary

**Zero-index bounds policy** means changing an out-of-range index to zero before performing the
access. **Equivalent structured buffer** is the canonical typed IR view used to query the number of
32-bit words backing a byte-address buffer. **NVVM legalization** is the compiler boundary between
common linked IR and exact direct-backend preflight.

## Process report

`CodeGenContext::emitNVVMForEntryPoints` first calls `linkAndOptimizeIR`, then late bit-cast
normalization, `legalizeIRForNVVM`, and finally `validateNVVMSupportedIR`. The motivating kernel's
final linked representation contains `ByteAddressBufferLoad`, `StructuredBufferLoad`,
`RWStructuredBufferGetElementPtr`, and `GetElement` over a fixed `ArrayType`. These are canonical,
intentional operations produced from the resource methods and subscripts in the source; the bad
shape was not an alternate spelling that an upstream producer should eliminate. The missing fact
was the target option, which remains available as the existing `MacroDefine` compiler option even
though direct NVVM never preprocesses the CUDA prelude.

`_legalizeNVVMZeroIndexBounds` joins those two existing sources of truth. For structured resources,
it emits `StructuredBufferGetDimensions` and extracts the element count. For byte-address buffers,
`_emitNVVMByteAddressBufferSize` follows the same representation as
`ByteAddressBuffer.GetDimensions`: `GetEquivalentStructuredBuffer` creates a `uint` word view,
`StructuredBufferGetDimensions` obtains its count, and multiplication by four gives the byte
extent. `getSizeAndAlignment` supplies the accessed value size under CUDA layout. Fixed arrays
already carry their literal count. The pass then changes only operand one of the canonical access,
using `index < count ? index : 0`, or the CUDA byte-address contract
`index <= sizeInBytes - elementSize ? index : 0`. Signed 32-bit element indices are compared through
an unsigned view because the CUDA wrapper converts them to `size_t` before checking them.

The transformation belongs in target legalization rather than the emitter. It is selected by
target semantics, has both the canonical access and its extent at this boundary, and is fully
expressible with existing generic IR/provider operations. Preflight and emission therefore remain
one consumer of ordinary canonical operations; no new provider callback, CUDA-text matcher,
fallback, or physical-resource-layout dependency was added.

The self-review inventory contains the option predicate, the element-index helper, two typed extent
helpers, and the finite legalization walk. Each survives because the focused runtime test fails
without the transformation. The walk intentionally excludes byte-address stores and explicit RW
structured-buffer loads: they are adjacent valid IR shapes, but this slice has no test proving
their exact contract. Repository search confirms that the bounds option occurs only in this test,
whose final IR exercises every retained access shape. The remaining corpus is strong inactive-path
coverage because `_isNVVMZeroIndexBoundsEnabled` returns before inspecting or mutating its IR.

Before the fix, direct PTX with the option had SHA-256
`603FB946B63FA99923F1B383B45DAD11F3E5528828C7473F3E06F39841B20526`. After legalization it has
SHA-256 `27D5A28D42E468E20AE4809AD02558B7704D56CD50F6C7E26E63CF502EC32B0C` and contains the expected
additional compare/select instructions. More importantly, all four native, CPU, direct O0, and
direct O3 runtime lanes now agree.

Frozen corpus v1 remains 452 workloads and 427 healthy MVP references. Correctness advances from
418 to 419 in O0, O3, and both modes, with this one gain and zero old-correct regressions. Across
all rows, each direct mode now reports 433 correct, 18 preflight failures, one infrastructure
failure, and zero runtime mismatches. Discovery remains separate at 82 workloads and 72 healthy
references; it stays 72/72/72. Each direct mode has 72 correct, seven infrastructure failures,
one runtime mismatch, and two preflight failures. The selected prefix passes 437/437, and the
permanent NVVM category passes 94/94.

The exploratory measurement gate compiled and assembled native NVRTC, direct O0 SM70, and direct
O3 SM70/SM80/SM90 PTX. Native compilation measured a 367.8 ms median and 12,717 bytes of PTX;
direct SM70 measured 240.3 ms and 4,069 bytes at O0, then 247.6 ms and 3,476 bytes at O3. Direct O3
PTX remained 3,476 bytes at SM80 and SM90, and all five configurations assembled successfully.
