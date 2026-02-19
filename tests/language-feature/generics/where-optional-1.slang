//DIAGNOSTIC_TEST:SIMPLE(diag=diag):
interface IThing
{
    void thing();
}

void f<T>(T t) where optional T: IThing
{
    // Unchecked optional constraint is an error.
    t.thing();
/*diag:
      ^^^^^ optional constraint not checked
      ^^^^^ the constraint providing 'thing' is optional and must be checked with an 'is' statement before usage.
*/
}
