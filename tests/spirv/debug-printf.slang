//TEST:SIMPLE(filecheck=CHECK): -target spirv -stage compute -entry main -emit-spirv-directly
//TEST:SIMPLE(filecheck=CHECK): -target spirv -stage compute -entry main -emit-spirv-via-glsl

void main()
{
    printf("test");
    printf("test1 %d", 5);
    // CHECK: %[[SET:[0-9]+]] = OpExtInstImport "NonSemantic.DebugPrintf"
    // CHECK: {{.*}} = OpExtInst %{{[a-zA-Z0-9_]+}} %[[SET]] 1 %{{[a-zA-Z0-9_]+}}
    // CHECK: {{.*}} = OpExtInst %{{[a-zA-Z0-9_]+}} %[[SET]] 1 %{{[a-zA-Z0-9_]+}} %{{[a-zA-Z0-9_]+}}
}
