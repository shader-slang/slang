---
generated: true
model: claude-sonnet-4-6
generated_at: 2026-06-02T10:13:16+00:00
source_commit: 57fb63455095efe1465661001ad147b24b968932
watched_paths_digest: fe6811d43c0f22bebaace550d7e09d466ddbff898615b73ff6d4e14ca4fcf043
source_doc: docs/language-reference/basics-memory-model-address-spaces.md
source_doc_digest: 3851dfb34bba532778d07931eed37a5ffb7720f62c8acf95efe83c116ec49a46
warning: "Auto-generated. May drift from source. Do not edit by hand."
---

# Tests for conformance/basics-memory-model-address-spaces

## Intent

Tests verify address-space claims in the **language reference** at
[`docs/language-reference/basics-memory-model-address-spaces.md`](../../../../language-reference/basics-memory-model-address-spaces.md).
The doc defines eleven address spaces in a single normative table plus three Remarks.
It states that each address space is roughly equivalent to a SPIR-V storage class (Remark 3),
which drives the emission-first coverage strategy: SPIRV, HLSL, GLSL, and CUDA emission
tests pin the storage class / storage qualifier the doc commits to for each kind of
variable or parameter. Runtime value claims for group-shared cross-thread communication
are recorded in `## Untested claims` with reason `gpu-other` because
`GroupMemoryBarrierWithGroupSync` is not available on the local CPU compute target.

## Claims

### Introductory claim

- **C0**: The address space for a variable is determined by its type, modifiers, and attributes.

### Uniform address space (row 1)

- **C1**: `ConstantBuffer<T>` places data in the Uniform address space (instance scope: all threads).
- **C2**: The `uniform` keyword on entry-point parameters also declares variables in the Uniform address space.

### Image address space (row 2)

- **C3**: `Texture2D<T>` (and other texture types such as `Texture1D<T>`) belongs to the Image address space (instance scope: all threads).

### Push constant address space (row 3)

- **C4**: `[vk::push_constant]`-annotated structs belong to the Push constant address space (instance scope: all threads).
- **C5**: `[push_constant]` (non-vk-prefixed form) is the second listed construct for the Push constant address space.

### Storage buffer address space (row 4)

- **C6**: `RWStructuredBuffer<T>` (read-write) belongs to the Storage buffer address space (instance scope: all threads).
- **C7**: `StructuredBuffer<T>` (read-only) also belongs to the Storage buffer address space.

### Group-shared address space (row 5)

- **C8**: `static groupshared` at global scope belongs to the Group-shared address space (instance scope: thread group).
- **C9**: Group-shared variable instances are shared by a thread group (threads in the same group can communicate through groupshared memory after a barrier).

### Function address space (row 6)

- **C10**: Function parameters and non-static local variable declarations belong to the Function address space (instance scope: thread), visible only to the function invocation.

### Thread-local address space (row 7)

- **C11**: `static` (global scope, not `groupshared`) belongs to the Thread-local address space (instance scope: thread; each thread gets a separate instance).

### Input address space (row 8)

- **C12**: Entry-point input parameters (non-uniform) belong to the Input address space (instance scope: thread).

### Output address space (row 9)

- **C13**: Entry-point output parameters and the entry-point return value belong to the Output address space (instance scope: thread).

### Specialization constant address space (row 10)

- **C14**: `[vk::specialization_constant]`-annotated variables belong to the Specialization constant address space (instance scope: all threads).
- **C15**: `[SpecializationConstant]` (non-vk form) is the second listed construct for the Specialization constant address space.

### Host address space (row 11)

- **C16**: The Host address space has no Slang construct; it is used only by the host program and is not directly accessible by a Slang program except when compiled for the C++ target.

### Remarks

- **C17**: Address spaces in Slang are roughly equivalent to SPIR-V storage classes (Remark 3).
- **C18**: A pointer to memory in one address space is generally not interchangeable with a pointer in another address space (Remark 2).
- **C19**: Pointers to group-shared memory are not interchangeable between thread groups (Remark 2 specific case).

## Functional coverage

| Claim                                                                                                                                                                                                                                                      | Intent               | Anchor                                                                                                 | Tests                                                                                                                                                                                                                                                                        |
| ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | -------------------- | ------------------------------------------------------------------------------------------------------ | ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| C0: The address space for a variable is determined by its type, modifiers, and attributes; multiple address spaces can coexist in one shader.                                                                                                              | functional           | [#address-spaces](../../../../language-reference/basics-memory-model-address-spaces.md#address-spaces) | [`address-space-by-type-spirv.slang`](address-space-by-type-spirv.slang)                                                                                                                                                                                                     |
| C1: `ConstantBuffer<T>` belongs to the Uniform address space; SPIRV emits it with `Uniform` storage class.                                                                                                                                                 | functional           | [#address-spaces](../../../../language-reference/basics-memory-model-address-spaces.md#address-spaces) | [`uniform-constant-buffer-spirv.slang`](uniform-constant-buffer-spirv.slang), [`uniform-constant-buffer-hlsl.slang`](uniform-constant-buffer-hlsl.slang)                                                                                                                     |
| C2: The `uniform` keyword on entry-point parameters also belongs to the Uniform address space; SPIRV emits it in a PushConstant block (single-element uniform params collapse to push-constant blocks).                                                    | functional           | [#address-spaces](../../../../language-reference/basics-memory-model-address-spaces.md#address-spaces) | [`uniform-keyword-spirv.slang`](uniform-keyword-spirv.slang)                                                                                                                                                                                                                 |
| C3: `Texture2D<T>` belongs to the Image address space; SPIRV emits it with `UniformConstant` storage class. `Texture1D<T>` (different dimensionality) also lands in `UniformConstant`.                                                                     | functional, boundary | [#address-spaces](../../../../language-reference/basics-memory-model-address-spaces.md#address-spaces) | [`image-texture2d-spirv.slang`](image-texture2d-spirv.slang), [`image-texture1d-spirv.slang`](image-texture1d-spirv.slang)                                                                                                                                                   |
| C4: `[vk::push_constant]`-annotated struct belongs to the Push constant address space; SPIRV emits it with `PushConstant` storage class.                                                                                                                   | functional           | [#address-spaces](../../../../language-reference/basics-memory-model-address-spaces.md#address-spaces) | [`push-constant-spirv.slang`](push-constant-spirv.slang)                                                                                                                                                                                                                     |
| C5: `[push_constant]` (non-vk-prefixed form) also produces `PushConstant` storage class in SPIRV.                                                                                                                                                          | boundary             | [#address-spaces](../../../../language-reference/basics-memory-model-address-spaces.md#address-spaces) | [`push-constant-attr-spirv.slang`](push-constant-attr-spirv.slang)                                                                                                                                                                                                           |
| C6/C7: `RWStructuredBuffer<T>` and `StructuredBuffer<T>` both belong to the Storage buffer address space; SPIRV emits both as `StorageBuffer`. The read-only buffer additionally gets `NonWritable`. HLSL maps them to `u` and `t` registers respectively. | functional           | [#address-spaces](../../../../language-reference/basics-memory-model-address-spaces.md#address-spaces) | [`storage-buffer-rw-spirv.slang`](storage-buffer-rw-spirv.slang), [`storage-buffer-hlsl.slang`](storage-buffer-hlsl.slang)                                                                                                                                                   |
| C8: `static groupshared` at global scope belongs to the Group-shared address space; SPIRV emits it with `Workgroup` storage class, HLSL with `groupshared`, GLSL with `shared`, CUDA with `__shared__`.                                                    | functional           | [#address-spaces](../../../../language-reference/basics-memory-model-address-spaces.md#address-spaces) | [`groupshared-workgroup-spirv.slang`](groupshared-workgroup-spirv.slang), [`groupshared-workgroup-hlsl.slang`](groupshared-workgroup-hlsl.slang), [`groupshared-workgroup-glsl.slang`](groupshared-workgroup-glsl.slang), [`groupshared-cuda.slang`](groupshared-cuda.slang) |
| C10: Non-static local variables belong to the Function address space; SPIRV emits mutable locals that survive SSA folding as `OpVariable` with `Function` storage class.                                                                                   | functional           | [#address-spaces](../../../../language-reference/basics-memory-model-address-spaces.md#address-spaces) | [`function-local-var-spirv.slang`](function-local-var-spirv.slang)                                                                                                                                                                                                           |
| C11: `static` (non-groupshared) at global scope belongs to the Thread-local address space; HLSL emits it as `static` at module scope (per-thread instance).                                                                                                | functional           | [#address-spaces](../../../../language-reference/basics-memory-model-address-spaces.md#address-spaces) | [`thread-local-static-hlsl.slang`](thread-local-static-hlsl.slang)                                                                                                                                                                                                           |
| C12: Entry-point input parameters (non-uniform) belong to the Input address space; SPIRV emits built-in inputs such as `SV_DispatchThreadID` with `Input` storage class.                                                                                   | functional           | [#address-spaces](../../../../language-reference/basics-memory-model-address-spaces.md#address-spaces) | [`input-address-space-spirv.slang`](input-address-space-spirv.slang)                                                                                                                                                                                                         |
| C13: Entry-point output parameters / return value belong to the Output address space; SPIRV emits vertex shader outputs with `Output` storage class.                                                                                                       | functional           | [#address-spaces](../../../../language-reference/basics-memory-model-address-spaces.md#address-spaces) | [`output-address-space-spirv.slang`](output-address-space-spirv.slang)                                                                                                                                                                                                       |
| C14: `[vk::specialization_constant]` belongs to the Specialization constant address space; SPIRV emits it as `OpSpecConstant` with a `SpecId` decoration. C15: `[SpecializationConstant]` (non-vk form) also produces `OpSpecConstant`.                    | functional, boundary | [#address-spaces](../../../../language-reference/basics-memory-model-address-spaces.md#address-spaces) | [`spec-constant-spirv.slang`](spec-constant-spirv.slang), [`spec-constant-attr-spirv.slang`](spec-constant-attr-spirv.slang)                                                                                                                                                 |
| C17: Address spaces are roughly equivalent to SPIR-V storage classes (Remark 3).                                                                                                                                                                           | functional           | [#address-spaces](../../../../language-reference/basics-memory-model-address-spaces.md#address-spaces) | Covered implicitly by every SPIRV storage-class pinning test above.                                                                                                                                                                                                          |

## Untested claims

| Claim                                                                                                                                                                         | Reason        | Anchor                                                                                                 | Why untested                                                                                                                                                                                                                                                                                                                                                                                                                                                 |
| ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | ------------- | ------------------------------------------------------------------------------------------------------ | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ |
| C9: Group-shared variable instances are shared by a thread group; threads in the same group can read values written by other threads after `GroupMemoryBarrierWithGroupSync`. | gpu-other     | [#address-spaces](../../../../language-reference/basics-memory-model-address-spaces.md#address-spaces) | `GroupMemoryBarrierWithGroupSync` is rejected for `-cpu` (E36107: unavailable feature). A `COMPARE_COMPUTE -vk` test was attempted but the local MoltenVK runner produces descriptor buffer allocation failures (VK_NULL_HANDLE descriptor writes), consistent with the pattern seen in the divergence-reconvergence bundle. CI with a discrete GPU validates this behavior.                                                                                 |
| C16: The Host address space has no Slang construct and is accessible only when compiled for the C++ target.                                                                   | out-of-bundle | [#address-spaces](../../../../language-reference/basics-memory-model-address-spaces.md#address-spaces) | The `host-cpp` target behavior for host-side memory access is covered in the types-pointer bundle. No `static groupshared` or other special construct is involved.                                                                                                                                                                                                                                                                                           |
| C18: A pointer to memory in one address space is generally not interchangeable with a pointer to memory in another address space (Remark 2).                                  | gpu-other     | [#address-spaces](../../../../language-reference/basics-memory-model-address-spaces.md#address-spaces) | The doc says "generally not interchangeable" — not "is rejected". No specific diagnostic code is specified; the restriction is a runtime-semantic claim for GPU execution. Attempting to pass a groupshared pointer as a storage-buffer pointer is rejected at the Slang type system level (different pointer types), but the doc does not name this as a "is rejected" claim with a specific error code, so no `DIAGNOSTIC_TEST` can be precisely anchored. |
| C19: Pointers to group-shared memory are not interchangeable between thread groups (Remark 2 specific case).                                                                  | gpu-other     | [#address-spaces](../../../../language-reference/basics-memory-model-address-spaces.md#address-spaces) | Requires two thread groups running simultaneously and sharing a pointer. Only observable on a GPU; no slang-test directive can simulate multi-group pointer aliasing.                                                                                                                                                                                                                                                                                        |

## Doc gaps observed

| Anchor                                                                                                 | Kind            | Gap                                                                                                                                                                                                                                                                                                                                                                                                      | Suggested addition                                                                                                                                                                                     |
| ------------------------------------------------------------------------------------------------------ | --------------- | -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ |
| [#address-spaces](../../../../language-reference/basics-memory-model-address-spaces.md#address-spaces) | missing-surface | The doc lists `uniform` as a Slang construct for the Uniform address space but gives no example of a standalone `uniform`-keyword declaration (only `ConstantBuffer<T>` is commonly used). The compiler maps `uniform` entry-point params to `PushConstant` storage class in SPIRV rather than `Uniform`, which differs from the `ConstantBuffer<T>` mapping. This subtle distinction is not documented. | Add a note clarifying that `uniform` on entry-point parameters may lower to a push-constant block on Vulkan/SPIRV, while `ConstantBuffer<T>` consistently maps to the `Uniform` storage class.         |
| [#address-spaces](../../../../language-reference/basics-memory-model-address-spaces.md#address-spaces) | missing-surface | The doc does not mention what happens to `static` global variables on SPIRV targets. The compiler inlines the per-invocation state into `Function`-scoped SPIRV variables (not `Private` as one might expect from the "roughly equivalent" Remark 3).                                                                                                                                                    | Add a note that thread-local (`static`) globals may lower to SPIRV `Function`-scoped storage per invocation rather than `Private`, because of how Slang specializes entry points.                      |
| [#address-spaces](../../../../language-reference/basics-memory-model-address-spaces.md#address-spaces) | ambiguous-claim | Remark 2 says "generally not interchangeable" but does not specify whether mixing address spaces is a compile-time error, a type-system error, or undefined runtime behavior. The word "generally" introduces ambiguity — does any cross-space pointer conversion ever exist (e.g., via `reinterpret_cast` or `Ptr<T>`)?                                                                                 | Clarify whether cross-address-space pointer conversion is always a type error, always UB, or permissible in specific documented cases. If it is always a type error, name the diagnostic code (E####). |
