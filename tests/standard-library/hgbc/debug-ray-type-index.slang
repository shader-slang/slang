// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

// TEST:SIMPLE(filecheck=CHECK): -target hgbc -entry main -g2 -validate-ir

// CHECK: Section("FUNC")
// CHECK: OpPrint
// CHECK: OpPrint

import slang.pipeline_rt;

struct Primary : IRayType
{
    float4 value;
}
struct Shadow : IRayType
{
    bool value;
}

[host]
void main()
{
    // Type-pack lookup specializes directly to an index, independently of debug bindings.
    print(rayTypeIndex<Primary, Primary, Shadow>());
    print(rayTypeIndex<Shadow, Primary, Shadow>());
}
