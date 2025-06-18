//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK):

interface IBase
{
    int f();
}

interface IDerived : IBase
{
    //CHECK-DAG: ([[# @LINE+1]]): error 30311:
    __init() {}

    property int value{
        get;

        //CHECK-DAG: ([[# @LINE+1]]): error 30311:
        set{ }
    }

    __subscript(int index)->int
    {
        //CHECK-DAG: ([[# @LINE+1]]): error 30311:
        get{}
    }

    //CHECK-DAG: ([[# @LINE+1]]): error 30312:
    override int f() {}
}