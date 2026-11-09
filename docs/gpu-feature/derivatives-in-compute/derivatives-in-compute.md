### Derivatives In Compute

An entry point may be decorated with `[DerivativeGroupQuad]` or `[DerivativeGroupLinear]` to specify how to use derivatives in compute shaders.

For SPIR-V and GLSL, when a compute entry point uses derivatives without either attribute, Slang infers linear grouping if the Y extent of `[numthreads]` is a known literal `1`, and quad grouping otherwise. For example, `[numthreads(16, 1, 1)]` with an implicit-LOD `Sample` uses linear grouping. A specialization-constant Y extent uses quad grouping even when its default is `1`.

Explicit attributes override inference. Both explicit and inferred grouping are validated: quad grouping requires even X and Y extents, and linear grouping requires the total thread count to be a multiple of four.

GLSL syntax may also be used, but is not recommended (`derivative_group_quadsNV`/`derivative_group_linearNV`).

Targets:

- **_SPIR-V:_** Enables `DerivativeGroupQuadsKHR` or `DerivativeGroupLinearKHR`.
- **_GLSL:_** Enables `derivative_group_quadsNV` or `derivative_group_linearNV`.
- **_HLSL:_** These attributes have no effect. `sm_6_6` is required to use derivatives in compute shaders; grouping follows the [HLSL thread-group rules](https://microsoft.github.io/DirectX-Specs/d3d/HLSL_SM_6_6_Derivatives.html#thread-groups-and-quads).
