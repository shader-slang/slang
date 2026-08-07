// Regression test for the CUDA architecture capability atoms added for issue #12426.
//
// This checks two things about the newly-added CUDA architecture atoms
// (compute 7.2, 7.5, 8.6, 8.7, 8.8, 10.0, 10.3, 11.0, 12.0, 12.1), exercised
// here through their public `cuda_sm_x_y` version aliases:
//
//   1. They exist and parse in a `[require(cuda, ...)]` attribute.
//   2. Their implication chain is ordered by (major, minor) compute
//      capability, so a higher architecture implies every lower one. A
//      function requiring a newer architecture than the one made available
//      must therefore fail the restrictive capability check, and requiring an
//      older architecture than the available one must succeed.

// A function requiring compute 12.1 is NOT satisfied when only cuda_sm_9_0 is
// available: 12.1 is newer than 9.0, so the requirement is missing.
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK-FAIL):-stage compute -entry computeMain -target cuda -capability cuda_sm_9_0 -restrictive-capability-check -DTOO_NEW=1

// A function requiring compute 8.6 IS satisfied when cuda_sm_12_0 is available:
// 12.0 implies 8.6 through the linear atom chain, so there is no diagnostic.
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK-OK):-stage compute -entry computeMain -target cuda -capability cuda_sm_12_0 -restrictive-capability-check -DTOO_NEW=0

#if TOO_NEW

[require(cuda, cuda_sm_12_1)]
void archDependent() {}

[CudaKernel]
void computeMain()
/*CHECK-FAIL:
     ^^^^^^^^^^^ entry point uses capabilities not in specified profile
     ^^^^^^^^^^^ Missing capabilities are: 'cuda_sm_12_1'
*/
{
    archDependent();
}

#else

[require(cuda, cuda_sm_8_6)]
void archDependent() {}

[CudaKernel]
void computeMain()
{
    archDependent();
}

#endif
