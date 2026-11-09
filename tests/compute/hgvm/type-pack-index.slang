// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//TEST:INTERPRET(hgvm, filecheck=CHECK): -I tests/language-feature/generics
//TEST:DEBUGGER(mode=CLI, prefix=CLI): -I tests/language-feature/generics
//TEST:DEBUGGER(mode=CLI, prefix=CLI, precompile=2): -I tests/language-feature/generics
//TEST:SIMPLE(filecheck=CODE): -entry main -target hgbc -O0 -g2 -validate-ir -I tests/language-feature/generics

import type_pack_index_library;

[host]
void main()
{
    print(typeIndex<int, int, float, bool>()); // CHECK: 0
    print(typeIndex<float, int, float, bool>()); // CHECK: 1
    print(typeIndex<bool, int, float, bool>()); // CHECK: 2
    print(typeIndexAsGenericArgument<float, int, float, bool>()); // CHECK: 1
    TypeIndexedArray<float, int, float, bool> array = { 11, 17 };
    print(array[1]); // CHECK: 17
    print(expandedTypeIndex<bool, int, float, bool>()); // CHECK: 2
    print(valueDependentTypeIndex<3>()); // CHECK: 1
    print(associatedTypeIndex<TypeIndexAssociated, int, float>()); // CHECK: 1
    print(partiallyKnownTypeIndex<int>()); // CHECK: 1
}

// CODE: Section("FUNC")
// CODE: OpPrint
// CLI-SEND: run
// CLI-CHECK: Program exited normally.
// CLI-CHECK: 0
// CLI-CHECK: 1
// CLI-CHECK: 2
// CLI-CHECK: 1
// CLI-CHECK: 17
// CLI-CHECK: 2
// CLI-CHECK: 1
// CLI-CHECK: 1
// CLI-CHECK: 1
// CLI-SEND: quit
