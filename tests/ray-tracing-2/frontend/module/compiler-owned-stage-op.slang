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
