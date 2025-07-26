//TEST:LANG_SERVER(filecheck=CHECK):
void f()
{
    Tuple<int,float> v;
//COMPLETE:6,7
    v.
}

// CHECK: _0
// CHECK: _1
// CHECK: equals