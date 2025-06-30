//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK):
interface IThing
{
    void thing();
}

void f<T, U>(T t, U u)
    where optional T: IThing
    where optional U: IThing
{
    // Error: cannot upgrade optional to non-optional witness in unchecked context.
    if (U is IThing)
    {
        // U being IThing doesn't justify using T as such!
        t.thing(); // CHECK: error 30403
    }
}
