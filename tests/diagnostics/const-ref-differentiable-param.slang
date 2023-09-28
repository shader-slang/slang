//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK):

[Differentiable]
float f(__constref float3 val)
{
    return val.x;
}

// CHECK: {{.*}}error {{.*}}: cannot use '{{(__)?}}constref' on a differentiable parameter.
