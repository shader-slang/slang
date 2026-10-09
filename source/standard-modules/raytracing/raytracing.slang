/// Provides the experimental structural pipeline ray-tracing API.
///
/// @par Availability
/// Requires an explicit `import slang.raytracing` and the `-experimental-feature` option.
/// Target adapter synthesis is deferred; API declarations remain available for frontend checking.
[ExperimentalModule]
module raytracing;

__include "compiler-intrinsics";
__include "ray-types";
__include "contexts";
__include "stage-inputs";
__include "stage-contracts";
__include "program-schema";
__include "descriptor";
__include "trace";
