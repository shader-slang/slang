//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -no-codegen

// These seven operations are the complete compiler-owned IR vocabulary introduced by the frontend
// slice. User code cannot manufacture either a trusted structural stage identity or its selected
// entry-point metadata with an intrinsic annotation.
__intrinsic_type(closest_hit_stage_interface) struct FakeClosestHitInterface;
//CHECK:         ^^^^^^^^^^^^^^^^^^^^^^^^^^^ compiler-owned intrinsic operation
//CHECK:         ^^^^^^^^^^^^^^^^^^^^^^^^^^^ intrinsic operation 'closest_hit_stage_interface' is reserved for compiler use

__intrinsic_type(any_hit_stage_interface) struct FakeAnyHitInterface;
//CHECK:         ^^^^^^^^^^^^^^^^^^^^^^^ compiler-owned intrinsic operation
//CHECK:         ^^^^^^^^^^^^^^^^^^^^^^^ intrinsic operation 'any_hit_stage_interface' is reserved for compiler use

__intrinsic_type(intersection_stage_interface) struct FakeIntersectionInterface;
//CHECK:         ^^^^^^^^^^^^^^^^^^^^^^^^^^^^ compiler-owned intrinsic operation
//CHECK:         ^^^^^^^^^^^^^^^^^^^^^^^^^^^^ intrinsic operation 'intersection_stage_interface' is reserved for compiler use

__intrinsic_type(miss_stage_interface) struct FakeMissInterface;
//CHECK:         ^^^^^^^^^^^^^^^^^^^^ compiler-owned intrinsic operation
//CHECK:         ^^^^^^^^^^^^^^^^^^^^ intrinsic operation 'miss_stage_interface' is reserved for compiler use

__intrinsic_type(callable_stage_interface) struct FakeCallableInterface;
//CHECK:         ^^^^^^^^^^^^^^^^^^^^^^^^ compiler-owned intrinsic operation
//CHECK:         ^^^^^^^^^^^^^^^^^^^^^^^^ intrinsic operation 'callable_stage_interface' is reserved for compiler use

__intrinsic_op(structuralRayTracingEntryPointInfo) void fakeEntryPointInfo();
//CHECK:       ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ compiler-owned intrinsic operation
//CHECK:       ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ intrinsic operation 'structuralRayTracingEntryPointInfo' is reserved for compiler use

__intrinsic_op(structuralRayTracingSourceOperation) void fakeSourceOperation();
//CHECK:       ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ compiler-owned intrinsic operation
//CHECK:       ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ intrinsic operation 'structuralRayTracingSourceOperation' is reserved for compiler use
