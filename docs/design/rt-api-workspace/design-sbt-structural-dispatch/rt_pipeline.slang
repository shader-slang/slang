/**
Draft pipeline ray tracing API for Slang.

This module sketches a Metal-first pipeline ray tracing model that still maps to
D3D/Vulkan native SBT dispatch. It includes basic ray tracing types, stage
functor contracts, and a structural dispatch table based on native SBT slot
indexing.

@remarks EXPERIMENTAL: This module is design-workspace code and is not a
production API.

@category raytracing
*/
[ExperimentalModule]
module rt_pipeline;

__include "rt_basic_types";
__include "rt_stage_contracts";
__include "rt_structural_dispatch_table";
