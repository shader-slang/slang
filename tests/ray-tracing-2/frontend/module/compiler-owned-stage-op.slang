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

__intrinsic_op(structuralRayTracingCallShader) void fakeStructuralCallableDispatch();
//CHECK:       ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ compiler-owned intrinsic operation
//CHECK:       ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ intrinsic operation 'structuralRayTracingCallShader' is reserved for compiler use

__intrinsic_op(metalStructuralRayTracingCallShader) void fakeMetalCallableDispatch();
//CHECK:       ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ compiler-owned intrinsic operation
//CHECK:       ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ intrinsic operation 'metalStructuralRayTracingCallShader' is reserved for compiler use

__intrinsic_op(structuralRayTracingHitGroupInfo) void fakeHitGroupInfo();
//CHECK:       ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ compiler-owned intrinsic operation
//CHECK:       ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ intrinsic operation 'structuralRayTracingHitGroupInfo' is reserved for compiler use

__intrinsic_op(structuralRayTracingMissGroupInfo) void fakeMissGroupInfo();
//CHECK:       ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ compiler-owned intrinsic operation
//CHECK:       ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ intrinsic operation 'structuralRayTracingMissGroupInfo' is reserved for compiler use

__intrinsic_op(structuralRayTracingCallableGroupInfo) void fakeCallableGroupInfo();
//CHECK:       ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ compiler-owned intrinsic operation
//CHECK:       ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ intrinsic operation 'structuralRayTracingCallableGroupInfo' is reserved for compiler use
