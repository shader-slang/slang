//TEST:LANG_SERVER(filecheck=CHECK):

void test()
{
//SIGNATURE:6,25
    let v = float3(1.0, )
}

// CHECK: (selected) float3.init(float x, float
