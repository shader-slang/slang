// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//TEST:SIMPLE(filecheck=CODE): -target hgbc -entry main -validate-ir -g1
//TEST:INTERPRET(hgvm,filecheck=CHECK): -api default

// Export keeps the unspecialized body alongside its concrete calls. The template still has
// loop block parameters, so only the concrete functions may undergo HGBC control-flow lowering.
[host]
export uint sum<let Bias : uint>(uint count)
{
    uint result = Bias;
    for (uint i = 0; i < count; ++i)
        result += i;
    return result;
}

[host]
void main()
{
    uint four = hgbc_asm { result : $$uint = OpIntLit 4; };
    uint zero = hgbc_asm { result : $$uint = OpIntLit 0; };
    print(sum<3>(four));
    print(sum<7>(zero));
}

// CODE: OpBranch {{%[A-Za-z0-9_]+}}
// CODE: OpBranchConditional

// CHECK: standard output = {
// CHECK-NEXT: 9
// CHECK-NEXT: 7
