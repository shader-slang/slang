//TEST:LANG_SERVER(filecheck=CHECK):
#language 202c

void test()
{
//SIGNATURE:7,25
    let v = float3(1.0, )
}

// CHECK: (selected) float3.init(float x, float
