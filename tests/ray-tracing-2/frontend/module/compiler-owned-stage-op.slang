//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

__intrinsic_type(closest_hit_stage_interface) struct FakeStageInterface;
//CHECK:         ^^^^^^^^^^^^^^^^^^^^^^^^^^^ compiler-owned intrinsic operation
//CHECK:         ^^^^^^^^^^^^^^^^^^^^^^^^^^^ intrinsic operation 'closest_hit_stage_interface' is reserved for compiler use

__intrinsic_op(any_hit_stage_interface) void fakeStageOperation();
//CHECK:       ^^^^^^^^^^^^^^^^^^^^^^^ compiler-owned intrinsic operation
//CHECK:       ^^^^^^^^^^^^^^^^^^^^^^^ intrinsic operation 'any_hit_stage_interface' is reserved for compiler use

__intrinsic_op(structuralRayTracingGetRayTCurrent) float fakeStageInputOperation();
//CHECK:       ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ compiler-owned intrinsic operation
//CHECK:       ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ intrinsic operation 'structuralRayTracingGetRayTCurrent' is reserved for compiler use

__intrinsic_op(structuralRayTracingGetPayload) float fakePayloadOperation();
//CHECK:       ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ compiler-owned intrinsic operation
//CHECK:       ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ intrinsic operation 'structuralRayTracingGetPayload' is reserved for compiler use

__intrinsic_op(structuralRayTracingGetCallableData) float fakeCallableDataOperation();
//CHECK:       ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ compiler-owned intrinsic operation
//CHECK:       ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ intrinsic operation 'structuralRayTracingGetCallableData' is reserved for compiler use

__intrinsic_op(structuralRayTracingGetHitAttributes) float fakeHitAttributesOperation();
//CHECK:       ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ compiler-owned intrinsic operation
//CHECK:       ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ intrinsic operation 'structuralRayTracingGetHitAttributes' is reserved for compiler use

__intrinsic_op(structuralRayTracingGetTriangleBarycentricCoord) float fakeTriangleBarycentricOperation();
//CHECK:       ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ compiler-owned intrinsic operation
//CHECK:       ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ intrinsic operation 'structuralRayTracingGetTriangleBarycentricCoord' is reserved for compiler use

__intrinsic_op(structuralRayTracingGetTriangleFrontFacing) bool fakeTriangleFrontFacingOperation();
//CHECK:       ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ compiler-owned intrinsic operation
//CHECK:       ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ intrinsic operation 'structuralRayTracingGetTriangleFrontFacing' is reserved for compiler use

__intrinsic_op(structuralRayTracingTrace) void fakeStructuralTrace();
//CHECK:       ^^^^^^^^^^^^^^^^^^^^^^^^^ compiler-owned intrinsic operation
//CHECK:       ^^^^^^^^^^^^^^^^^^^^^^^^^ intrinsic operation 'structuralRayTracingTrace' is reserved for compiler use

// Numeric spellings resolve directly to the current IR enum and must not bypass the reservation.
__intrinsic_op(302) void fakeStructuralTraceByNumber();
//CHECK:       ^^^ compiler-owned intrinsic operation
//CHECK:       ^^^ intrinsic operation '302' is reserved for compiler use

__intrinsic_op(structuralRayTracingCallShader) void fakeStructuralCallableDispatch();
//CHECK:       ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ compiler-owned intrinsic operation
//CHECK:       ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ intrinsic operation 'structuralRayTracingCallShader' is reserved for compiler use

__intrinsic_op(metalStructuralRayTracingCallShader) void fakeMetalCallableDispatch();
//CHECK:       ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ compiler-owned intrinsic operation
//CHECK:       ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ intrinsic operation 'metalStructuralRayTracingCallShader' is reserved for compiler use

__intrinsic_op(metalStructuralRayTracingTrace) void fakeMetalTrace();
//CHECK:       ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ compiler-owned intrinsic operation
//CHECK:       ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ intrinsic operation 'metalStructuralRayTracingTrace' is reserved for compiler use

__intrinsic_op(metalStructuralRayTracingDispatchRaysIndex) uint3 fakeMetalDispatchIndex();
//CHECK:       ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ compiler-owned intrinsic operation
//CHECK:       ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ intrinsic operation 'metalStructuralRayTracingDispatchRaysIndex' is reserved for compiler use

__intrinsic_op(metalStructuralRayTracingDispatchRaysDimensions) uint3 fakeMetalDispatchDimensions();
//CHECK:       ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ compiler-owned intrinsic operation
//CHECK:       ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ intrinsic operation 'metalStructuralRayTracingDispatchRaysDimensions' is reserved for compiler use

__intrinsic_op(structuralRayTracingEntryPointInfo) void fakeEntryPointInfo();
//CHECK:       ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ compiler-owned intrinsic operation
//CHECK:       ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ intrinsic operation 'structuralRayTracingEntryPointInfo' is reserved for compiler use

__intrinsic_op(structuralRayTracingProgramPayloadLocation) void fakePayloadLocation();
//CHECK:       ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ compiler-owned intrinsic operation
//CHECK:       ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ intrinsic operation 'structuralRayTracingProgramPayloadLocation' is reserved for compiler use

__intrinsic_op(structuralRayTracingSemanticallyEmptyPayload) void fakeEmptyPayloadMetadata();
//CHECK:       ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ compiler-owned intrinsic operation
//CHECK:       ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ intrinsic operation 'structuralRayTracingSemanticallyEmptyPayload' is reserved for compiler use

__intrinsic_op(structuralRayTracingMetalPayloadMetadata) void fakeMetalPayloadMetadata();
//CHECK:       ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ compiler-owned intrinsic operation
//CHECK:       ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ intrinsic operation 'structuralRayTracingMetalPayloadMetadata' is reserved for compiler use

__intrinsic_op(structuralRayTracingHitGroupInfo) void fakeHitGroupInfo();
//CHECK:       ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ compiler-owned intrinsic operation
//CHECK:       ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ intrinsic operation 'structuralRayTracingHitGroupInfo' is reserved for compiler use

__intrinsic_op(structuralRayTracingMissShaderInfo) void fakeMissShaderInfo();
//CHECK:       ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ compiler-owned intrinsic operation
//CHECK:       ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ intrinsic operation 'structuralRayTracingMissShaderInfo' is reserved for compiler use

__intrinsic_op(structuralRayTracingCallableShaderInfo) void fakeCallableShaderInfo();
//CHECK:       ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ compiler-owned intrinsic operation
//CHECK:       ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ intrinsic operation 'structuralRayTracingCallableShaderInfo' is reserved for compiler use

__intrinsic_type(MetalIntersectionFunctionTable) struct FakeMetalIntersectionFunctionTable;
//CHECK:         ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ compiler-owned intrinsic operation
//CHECK:         ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ intrinsic operation 'MetalIntersectionFunctionTable' is reserved for compiler use

__intrinsic_type(MetalVisibleFunctionTable) struct FakeMetalVisibleFunctionTable;
//CHECK:         ^^^^^^^^^^^^^^^^^^^^^^^^^ compiler-owned intrinsic operation
//CHECK:         ^^^^^^^^^^^^^^^^^^^^^^^^^ intrinsic operation 'MetalVisibleFunctionTable' is reserved for compiler use

__intrinsic_op(metalVisibleFunction) void fakeMetalVisibleFunction();
//CHECK:       ^^^^^^^^^^^^^^^^^^^^ compiler-owned intrinsic operation
//CHECK:       ^^^^^^^^^^^^^^^^^^^^ intrinsic operation 'metalVisibleFunction' is reserved for compiler use

__intrinsic_op(metalIntersectionFunction) void fakeMetalIntersectionFunction();
//CHECK:       ^^^^^^^^^^^^^^^^^^^^^^^^^ compiler-owned intrinsic operation
//CHECK:       ^^^^^^^^^^^^^^^^^^^^^^^^^ intrinsic operation 'metalIntersectionFunction' is reserved for compiler use
