// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//DIAGNOSTIC_TEST:SIMPLE(filecheck=MISSING): -target hgbc -entry missing -O0 -g2 -I tests/language-feature/generics
//DIAGNOSTIC_TEST:SIMPLE(filecheck=DUPLICATE): -target hgbc -entry duplicate -O3 -g2 -I tests/language-feature/generics
//DIAGNOSTIC_TEST:SIMPLE(filecheck=MISSING): -target hgbc -entry empty -O0 -I tests/language-feature/generics
//DIAGNOSTIC_TEST:SIMPLE(filecheck=DUPLICATE): -target hgbc -entry dependentDuplicate -O0 -g2 -I tests/language-feature/generics
//DIAGNOSTIC_TEST:SIMPLE(filecheck=DUPLICATE): -target hgbc -entry duplicate -O3 -g0 -I tests/language-feature/generics
//DIAGNOSTIC_TEST:SIMPLE(filecheck=MISSING): -target hgbc -entry missing -O0 -g0 -I tests/language-feature/generics
//DIAGNOSTIC_TEST:SIMPLE(filecheck=DUPLICATE): -target hgbc -entry partiallyKnownDuplicate -O0 -g2 -I tests/language-feature/generics
//DIAGNOSTIC_TEST:SIMPLE(filecheck=MISSING): -target hgbc -entry unusedGenericArgument -O0 -I tests/language-feature/generics
//DIAGNOSTIC_TEST:SIMPLE(filecheck=MISSING): -target hgbc -entry unusedArray -O0 -I tests/language-feature/generics

import type_pack_index_library;

[host]
void missing() { typeIndex<bool, int, float>(); }
[host]
void duplicate() { typeIndex<float, float, int, float>(); }
[host]
void empty() { typeIndex<int>(); }
[host]
void dependentDuplicate() { valueDependentTypeIndex<4>(); }
[host]
void partiallyKnownDuplicate() { partiallyKnownTypeIndex<float>(); }
[host]
void unusedGenericArgument() { typeIndexAsGenericArgument<bool, int, float>(); }
[host]
void unusedArray() { TypeIndexedArray<bool, int, float> unused; }

// MISSING: type is absent from the pack in __index_of_type_in_pack
// DUPLICATE: type occurs more than once in the pack in __index_of_type_in_pack
