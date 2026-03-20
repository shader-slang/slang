//TEST:LANG_SERVER(filecheck=CHECK):

// Verify hover still reports a useful declaration for a recursive generic
// static const even when constant folding cannot fully evaluate it.

struct Loop<let x : int>
{
    static const int value = Loop<x + 1>::value;
}

//HOVER:13,27
// CHECK: (field) static const int Loop<0>.value
int useValue = Loop<0>::value;
