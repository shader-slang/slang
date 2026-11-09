//TEST:SIMPLE(filecheck=CHECK): -target hgbc -entry main

[host]
public void main(int left, int right, uint unsignedLeft, uint unsignedRight)
{
    print(left % right);
    print(unsignedLeft % unsignedRight);
    int2 vectorRemainder = int2(left, right) % int2(right, left);
    print(vectorRemainder.x);
}

// CHECK: Section("TYPE")
// CHECK-DAG: [[INT:%[A-Za-z0-9_]+]] = OpIntType
// CHECK-DAG: [[UINT:%[A-Za-z0-9_]+]] = OpUIntType
// CHECK-DAG: [[INT2:%[A-Za-z0-9_]+]] = OpVectorType 2 [[INT]]

// CHECK: Section("FUNC")
// CHECK-DAG: {{%[A-Za-z0-9_]+}} : [[INT]] = OpIRem [[INT]]
// CHECK-DAG: {{%[A-Za-z0-9_]+}} : [[UINT]] = OpIRem [[UINT]]
// CHECK-DAG: {{%[A-Za-z0-9_]+}} : [[INT2]] = OpIRem [[INT2]]
