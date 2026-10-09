# Prompt: docs/generated/tests/conformance/basics-memory-model-address-spaces/

See [`_common.md`](_common.md).

## Target

Bundle at `docs/generated/tests/conformance/basics-memory-model-address-spaces/`,
anchored to
[`docs/language-reference/basics-memory-model-address-spaces.md`](../../../../language-reference/basics-memory-model-address-spaces.md).

## Doc summary

The doc (~99 lines) defines eleven address spaces in a single HTML table, then adds
three Remarks. Claims fall into two categories:

1. **Per-address-space declarations** (the table rows, 11 entries):
   Each row names an address space, the Slang construct that declares a variable in it,
   the instance scope (All threads / Thread group / Thread / Host process), and a brief
   description. These are normative; every row is a testable claim about which construct
   produces which address space and what its scope is.

2. **Remarks** (three unnumbered remarks after the table):
   - Remark 1: Graphics-pipeline-stage-specific address spaces are described elsewhere.
     Non-normative for this bundle; out-of-bundle.
   - Remark 2: Pointers to group-shared memory are not interchangeable between thread
     groups. Normative claim about pointer compatibility.
   - Remark 3: Address spaces in Slang are roughly equivalent to SPIR-V storage classes.
     Normative; drives the emission-test strategy.

## Claim extraction strategy

The table has exactly 11 rows. Each row yields one claim: "Slang construct X declares a
variable in address space Y with instance scope Z." The most specific observable surface
for each is the emitted storage class in SPIRV asm (per Remark 3). HLSL and GLSL
targets are secondary observations.

For each address space, the probes below confirmed the SPIRV mapping:

| Address space        | Slang construct                                | Expected SPIRV storage class        |
| -------------------- | ---------------------------------------------- | ----------------------------------- |
| Uniform              | `ConstantBuffer<T>`                            | `Uniform`                           |
| Image                | `Texture2D<T>` (etc.)                          | `UniformConstant`                   |
| Push constant        | `[vk::push_constant]` struct                   | `PushConstant`                      |
| Storage buffer       | `RWStructuredBuffer<T>`, `StructuredBuffer<T>` | `StorageBuffer`                     |
| Group-shared         | `static groupshared` at global scope           | `Workgroup`                         |
| Function             | local `var` in an `inout` function             | `Function`                          |
| Thread-local         | `static` (non-groupshared) at global scope     | `Function` (per-invocation Private) |
| Input                | entry-point input parameter (non-uniform)      | `Input`                             |
| Output               | entry-point return value / output parameter    | `Output`                            |
| Specialization const | `[vk::specialization_constant]`                | `OpSpecConstant` + `SpecId`         |
| Host                 | None                                           | n/a (no Slang construct)            |

Remark 2 (pointer non-interchangeability between address spaces) maps to a `DIAGNOSTIC_TEST`
claim if the compiler rejects mixing — but the doc does not say "is rejected", only "generally
not interchangeable". This is an ambiguous claim; it is recorded as untested.

Remark 3 (roughly equivalent to SPIR-V storage classes) drives the entire emission
test strategy and is implicitly covered by every SPIRV storage-class pinning test.

## What NOT to test here

- Graphics-pipeline-stage-specific address spaces (Remark 1 defers to `shaders-and-kernels.md`).
- Actual runtime values that require a GPU. All tests here are emission-pinning.
- Whether `ConstantBuffer<T>` data is truly constant at dispatch time — a GPU-runtime claim.
- Pointer arithmetic between address spaces — no explicit user syntax in Slang.
