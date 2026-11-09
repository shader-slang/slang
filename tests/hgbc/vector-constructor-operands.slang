//TEST:INTERPRET(hgvm):
//TEST:SIMPLE(filecheck=CHECK): -target hgbc -entry main

[noinline]
uint3 appendLane(uint2 prefix, uint last)
{
    return uint3(prefix, last);
}

[noinline]
uint4 concatenate(uint2 left, uint2 right)
{
    return uint4(left, right);
}

void fail(String message)
{
    hgbc_asm { OpFail $message; };
}

[host]
void main()
{
    uint3 three = appendLane(uint2(7u, 11u), 13u);
    if (three.x != 7u || three.y != 11u || three.z != 13u)
        fail("HGVM vector-plus-scalar construction produced the wrong lanes");

    uint4 four = concatenate(uint2(17u, 19u), uint2(23u, 29u));
    if (four.x != 17u || four.y != 19u || four.z != 23u || four.w != 29u)
        fail("HGVM vector-plus-vector construction produced the wrong lanes");
}

// CHECK: Section("TYPE")
// CHECK-DAG: [[UINT:%[A-Za-z0-9_]+]] = OpUIntType
// CHECK-DAG: [[UINT2:%[A-Za-z0-9_]+]] = OpVectorType 2 [[UINT]]
// CHECK-DAG: [[UINT3:%[A-Za-z0-9_]+]] = OpVectorType 3 [[UINT]]
// CHECK-DAG: [[UINT4:%[A-Za-z0-9_]+]] = OpVectorType 4 [[UINT]]
// CHECK: Section("FUNC")
// CHECK-LABEL: %appendLane : {{%[A-Za-z0-9_]+}} = OpFunc
// CHECK: [[PREFIX:%[A-Za-z0-9_]+]] : [[UINT2]] = OpParam
// CHECK: [[LAST:%[A-Za-z0-9_]+]] : [[UINT]] = OpParam
// CHECK: [[X:%[A-Za-z0-9_]+]] : [[UINT]] = OpExtractElement [[PREFIX]]
// CHECK: [[Y:%[A-Za-z0-9_]+]] : [[UINT]] = OpExtractElement [[PREFIX]]
// CHECK: {{%[A-Za-z0-9_]+}} : [[UINT3]] = OpMakeVector [[X]] [[Y]] [[LAST]]
// CHECK-LABEL: %concatenate : {{%[A-Za-z0-9_]+}} = OpFunc
// CHECK: [[LEFT:%[A-Za-z0-9_]+]] : [[UINT2]] = OpParam
// CHECK: [[RIGHT:%[A-Za-z0-9_]+]] : [[UINT2]] = OpParam
// CHECK: [[A:%[A-Za-z0-9_]+]] : [[UINT]] = OpExtractElement [[LEFT]]
// CHECK: [[B:%[A-Za-z0-9_]+]] : [[UINT]] = OpExtractElement [[LEFT]]
// CHECK: [[C:%[A-Za-z0-9_]+]] : [[UINT]] = OpExtractElement [[RIGHT]]
// CHECK: [[D:%[A-Za-z0-9_]+]] : [[UINT]] = OpExtractElement [[RIGHT]]
// CHECK: {{%[A-Za-z0-9_]+}} : [[UINT4]] = OpMakeVector [[A]] [[B]] [[C]] [[D]]
