// diagnose-conjunction-equality-witness.slang

// Test that using an AndType (conjunction) as the super-type of
// a generic equality constraint is diagnosed.

//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK):

interface IAlpha
{
    int getAlpha();
}

interface IBeta
{
    int getBeta();
}

// Equality constraint with a conjunction super-type should be an error.
// CHECK: error[E30404]
void f<T>(T val)
    where T == IAlpha & IBeta
{
}
