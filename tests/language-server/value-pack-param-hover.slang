//TEST:LANG_SERVER(filecheck=CHECK):

struct Dims<let each D : int>
{
//HOVER:6,40
    int getDimCount() { return countof(D); }
}

// CHECK: (generic value pack parameter) let each D : int
