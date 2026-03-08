//TEST:LANG_SERVER(filecheck=CHECK):

struct Dims<let each D : int>
{
//HOVER:6,35
    int getDim() { return __first(D); }
}

// CHECK: (generic value pack parameter) let each D : int
