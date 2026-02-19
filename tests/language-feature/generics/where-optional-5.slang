//DIAGNOSTIC_TEST:SIMPLE(diag=diag):
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
        t.thing();
//diag:   ^^^^^ optional constraint not checked
//diag:   ^^^^^ the constraint providing 'thing' is optional and must be checked with an 'is' statement before usage.
    }
}
