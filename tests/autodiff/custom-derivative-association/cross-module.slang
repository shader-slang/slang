//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

// Each attribute that gives a function a derivative across modules, and which derivative each way
// of invoking it runs. As in `same-module.slang`, every primal computes `a * x * x`, whose
// derivative is 12 at a = 3 and x = 2, and every user-defined derivative computes 1012 instead.
// `func-extension-cross-module.slang` covers the derivatives that a `__func_extension` declares.
//
// A function and its derivative may be declared together in another module, the function may be
// declared in one module and its derivative in another, or the derivative may be declared in the
// module that uses it. A backward derivative, unlike a forward one, needs the module that declares
// it to import the function with `__exported import`; `non-exported-import.slang` covers the
// error otherwise.

import "cross-module-library";
import "cross-module-primal";
import "cross-module-derivative";
__exported import "cross-module-exported-primal";
import "cross-module-exported-derivative";

// --- Derivatives declared in this module ---

[ForwardDerivativeOf(mainForwardPrimal)]
DifferentialPair<float> mainForwardDerivative(no_diff float a, DifferentialPair<float> x)
{
    return diffPair(a * x.p * x.p, (1000.0 + 2.0 * a * x.p) * x.d);
}

[ForwardDerivativeOf(exportedMainForwardPrimal)]
DifferentialPair<float> exportedMainForwardDerivative(no_diff float a, DifferentialPair<float> x)
{
    return diffPair(a * x.p * x.p, (1000.0 + 2.0 * a * x.p) * x.d);
}

[BackwardDerivativeOf(exportedMainBackwardPrimal)]
void exportedMainBackwardDerivative(no_diff float a, inout DifferentialPair<float> x, float dOut)
{
    x = diffPair(x.p, dOut * (1000.0 + 2.0 * a * x.p));
}

// --- Differentiable callers of functions that have backward derivatives ---

[Differentiable]
float callLibrarySynthesized(float x) { return librarySynthesized(3.0, x); }

[Differentiable]
float callLibraryBackwardNamed(float x) { return libraryBackwardNamed(3.0, x); }

[Differentiable]
float callLibraryBackwardOfPrimal(float x) { return libraryBackwardOfPrimal(3.0, x); }

[Differentiable]
float callExportedSplitBackwardPrimal(float x) { return exportedSplitBackwardPrimal(3.0, x); }

[Differentiable]
float callExportedMainBackwardPrimal(float x) { return exportedMainBackwardPrimal(3.0, x); }

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    // A function declared together with its derivative in another module.
    outputBuffer[0] = fwd_diff(librarySynthesized)(3.0, diffPair(2.0, 1.0)).d;
    var dp0 = diffPair(2.0, 0.0);
    bwd_diff(librarySynthesized)(3.0, dp0, 1.0);
    outputBuffer[1] = dp0.d;
    outputBuffer[2] = fwd_diff(callLibrarySynthesized)(diffPair(2.0, 1.0)).d;
    var dp1 = diffPair(2.0, 0.0);
    bwd_diff(callLibrarySynthesized)(dp1, 1.0);
    outputBuffer[3] = dp1.d;
    // CHECK: type: float
    // CHECK-NEXT: 12.0
    // CHECK-NEXT: 12.0
    // CHECK-NEXT: 12.0
    // CHECK-NEXT: 12.0

    outputBuffer[4] = fwd_diff(libraryForwardNamed)(3.0, diffPair(2.0, 1.0)).d;
    outputBuffer[5] = fwd_diff(libraryForwardOfPrimal)(3.0, diffPair(2.0, 1.0)).d;
    // CHECK-NEXT: 1012.0
    // CHECK-NEXT: 1012.0

    var dp2 = diffPair(2.0, 0.0);
    bwd_diff(libraryBackwardNamed)(3.0, dp2, 1.0);
    outputBuffer[6] = dp2.d;
    outputBuffer[7] = fwd_diff(callLibraryBackwardNamed)(diffPair(2.0, 1.0)).d;
    var dp3 = diffPair(2.0, 0.0);
    bwd_diff(callLibraryBackwardNamed)(dp3, 1.0);
    outputBuffer[8] = dp3.d;
    // CHECK-NEXT: 1012.0
    // CHECK-NEXT: 0.0
    // CHECK-NEXT: 1012.0

    var dp4 = diffPair(2.0, 0.0);
    bwd_diff(libraryBackwardOfPrimal)(3.0, dp4, 1.0);
    outputBuffer[9] = dp4.d;
    outputBuffer[10] = fwd_diff(callLibraryBackwardOfPrimal)(diffPair(2.0, 1.0)).d;
    var dp5 = diffPair(2.0, 0.0);
    bwd_diff(callLibraryBackwardOfPrimal)(dp5, 1.0);
    outputBuffer[11] = dp5.d;
    // CHECK-NEXT: 1012.0
    // CHECK-NEXT: 0.0
    // CHECK-NEXT: 1012.0

    // A function declared in one module and its derivative in another.
    outputBuffer[12] = fwd_diff(splitForwardPrimal)(3.0, diffPair(2.0, 1.0)).d;
    outputBuffer[13] = fwd_diff(exportedSplitForwardPrimal)(3.0, diffPair(2.0, 1.0)).d;
    // CHECK-NEXT: 1012.0
    // CHECK-NEXT: 1012.0

    var dp6 = diffPair(2.0, 0.0);
    bwd_diff(exportedSplitBackwardPrimal)(3.0, dp6, 1.0);
    outputBuffer[14] = dp6.d;
    outputBuffer[15] = fwd_diff(callExportedSplitBackwardPrimal)(diffPair(2.0, 1.0)).d;
    var dp7 = diffPair(2.0, 0.0);
    bwd_diff(callExportedSplitBackwardPrimal)(dp7, 1.0);
    outputBuffer[16] = dp7.d;
    // CHECK-NEXT: 1012.0
    // CHECK-NEXT: 0.0
    // CHECK-NEXT: 1012.0

    // A function declared in another module and its derivative in this one.
    outputBuffer[17] = fwd_diff(mainForwardPrimal)(3.0, diffPair(2.0, 1.0)).d;
    outputBuffer[18] = fwd_diff(exportedMainForwardPrimal)(3.0, diffPair(2.0, 1.0)).d;
    // CHECK-NEXT: 1012.0
    // CHECK-NEXT: 1012.0

    var dp8 = diffPair(2.0, 0.0);
    bwd_diff(exportedMainBackwardPrimal)(3.0, dp8, 1.0);
    outputBuffer[19] = dp8.d;
    outputBuffer[20] = fwd_diff(callExportedMainBackwardPrimal)(diffPair(2.0, 1.0)).d;
    var dp9 = diffPair(2.0, 0.0);
    bwd_diff(callExportedMainBackwardPrimal)(dp9, 1.0);
    outputBuffer[21] = dp9.d;
    // CHECK-NEXT: 1012.0
    // CHECK-NEXT: 0.0
    // CHECK-NEXT: 1012.0
}
