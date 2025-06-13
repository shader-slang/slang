//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK):
interface IThing
{
    void thing();
}

void f<T>(T t) where optional T: IThing
{
    // Unchecked optional constraint is an error.
    t.thing(); // CHECK: error 30403
}
