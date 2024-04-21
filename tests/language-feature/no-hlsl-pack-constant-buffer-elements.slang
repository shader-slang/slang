//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -profile cs_6_0 -no-hlsl-pack-constant-buffer-elements

// Test that -no-hlsl-pack-constant-buffer-elements prevents packing of elements in the output HLSL code.

// CHECK-DAG: cbuffer {{.*}} :
cbuffer MyCB
{
    float member0;
    float member1;
}

// CHECK-DAG: return member0{{.*}} + member1{{.*}};
[numthreads(1,1,1)]
float main()
{
    return member0 + member1;
}
