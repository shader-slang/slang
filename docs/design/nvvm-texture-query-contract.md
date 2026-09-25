# CUDA texture query boundaries

Slice248 qualifies the existing texture-get-dimensions discovery failure without changing its
oracle or claiming new support. See [report248](../../issue-nvvm-backend/report.slice-248-texture-contract.md)
and [evidence248](../../issue-nvvm-backend/semantic-evidence.slice-248.json).

`TextureTypeInfo::writeGetDimensionFunctions` currently emits base CUDA texture geometry queries
for mip and non-mip GetDimensions alike, ignores the requested mip level, and writes zero for array
and total-level counts. NVVM's exact helper resolver admits selected non-mip forms only. Integer
controls demonstrate these separate limitations; the original uint-to-float packed oracle masks
all seven lost mip counts and exposes only its three array-count errors. Neither matching NVRTC
nor successful PTX assembly establishes the full source API contract.

Render-test allocates full mip chains by default. Its array resource kind depends on
`arrayLength > 1`, so the original declared cube-array resource with length1 binds a nonlayered
cube. The active Slang RHI CUDA texture object uses the resource's array/mipmapped-array plus its
sampler and optional partial resource view. A surface object instead refers to a selected mip's
single CUDA array. Read-only texture queries must not substitute a surface handle.

Independent CUDA12.9/L4/SM80 probes establish a narrower useful contract:

- `txq.level.width` returns the selected mip width, even with sampler max-mip-clamp0. A future
  mip-dimension producer can investigate the existing query instead of assuming it is unavailable.
- 1D-array height yields layer count; 2D-array depth yields layer count; cubemap-array depth yields
  cube count, while host array depth counts faces. Nonlayered cube depth is zero. These observations
  do not resolve the length1 binding distinction or every possible subresource view.
- `txq.array_size` and `txq.num_mipmap_levels` assemble, but their named functions fail CUDA driver
  lookup with error500 on this stack. Identical-entry `txq.level.width` loads and executes. Both
  failed cubins retain an undefined weak `.nv.unified.texrefDescSize`; the internal failure cause
  is not proven. Do not generalize this to all drivers, source APIs or device query families.
- Host resource/array queries expose the allocated levels and geometry. Full four-level and
  partial two-level8x8 chains share base dimensions; inferring total count from width is wrong.
- Surface width queries return elements. Byte-addressed surface accesses are a different contract.

The [HLSL API](https://learn.microsoft.com/en-us/windows/win32/direct3dhlsl/dx-graphics-hlsl-to-getdimensions)
defines requested mip, geometry, array elements and total levels separately. The
[PTX specification](https://docs.nvidia.com/cuda/archive/12.9.0/parallel-thread-execution/index.html#texture-sampler-and-surface-types)
defines opaque handles; it does not license inspecting their private layout. Full API support needs
an explicit allocated/view-level-count contract, producer-owned semantics and corresponding direct
NVVM lowering. Partial geometry support may need no new ABI, but it must preserve unresolved full
API cases and document its boundary. Changes to library/resource/provider/ABI contracts require a
full frozen/discovery checkpoint. No change to those contracts occurs in248.
