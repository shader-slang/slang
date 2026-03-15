//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

// Verify frontend generic value evaluation stops with a diagnostic when
// recursive constant folding exceeds the evaluation budget.

struct Loop<let x : int>
{
    static const int value = Loop<x + 1>::value;
//CHECK:             ^^^^^ recursive generic evaluation exceeded maximum depth
//CHECK:             ^^^^^ evaluation of 'value' exceeded the recursive generic evaluation budget (64)
}

int useValue = Loop<0>::value;
