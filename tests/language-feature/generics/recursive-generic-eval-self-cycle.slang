//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

// Verify an exact same-instantiation recursive reference still diagnoses as
// a direct circular definition instead of degrading into the recursion budget.

struct Loop<let x : int>
{
    static const int value = Loop<x>::value;
//CHECK:             ^^^^^ the initial-value expression for variable 'value' depends on the value of the variable itself
}

int useValue = Loop<0>::value;
