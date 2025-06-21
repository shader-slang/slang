//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK):
interface IThing
{
    void thing();
}

void g<T>(T t) where T: IThing
{
}

void f<T>(T t) where optional T: IThing
{
    // Error: cannot upgrade optional to non-optional witness in unchecked context.
    g<T>(t); // CHECK: error 38029
}
