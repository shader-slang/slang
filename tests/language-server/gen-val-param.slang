//TEST:LANG_SERVER(filecheck=CHECK):
struct LongTypeNameLongTypeName
{
    int x;
    int y;
}

[numthreads(1,1,1)]
void computeMain<let geparam : int>()
{
    int abc = 2;
    //HOVER:13,47
    LongTypeNameLongTypeName var = {abc, geparam};
    // Check that language server can report correct line number for geparam.
    // CHECK: (9)
}