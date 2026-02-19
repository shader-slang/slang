//DIAGNOSTIC_TEST:SIMPLE(diag=diag,non-exhaustive): -target spirv

[numthreads(1,1,1)]
void test()
{
    int c = 0;
    let lam = (int a) => {
        c = a + 1;
//diag: ^ attempting to assign to a const variable or immutable member; use '[mutating]' attribute on the containing method to allow modification
//diag:   ^ left of '=' is not an l-value
//diag:   ^ left of '=' is not an l-value.
    };
    let lam1 = (int a) => {
        if (a > 0)
            return 1;
        else
            return 3.0;
//diag: ^^^^^^ lambda return type mismatch
//diag: ^^^^^^ returned values must have the same type among all 'return' statements inside a lambda expression: returned 'float' here, but 'int' previously.
//diag:        ^^^ implicit conversion not recommended
//diag:        ^^^ implicit conversion from 'float' to 'int' is not recommended
    };
    RayQuery q;
    let lam2 = (int a) => {
        q.Abort();
//diag: ^ non-copyable type captured
//diag: ^ cannot capture non-copyable type 'RayQuery<0>' in a lambda expression.
//diag: ^ attempting to assign to a const variable or immutable member; use '[mutating]' attribute on the containing method to allow modification
//diag:        ^ mutating method cannot be called on immutable value
//diag:        ^ mutating method 'Abort' cannot be called on an immutable value
    };
}
