// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK):

int scalarPack() { return __index_of_type_in_pack<int, float>; }
// CHECK: expected a single type followed by a type pack

int packElement<each P>() { return __index_of_type_in_pack<P, P>; }
// CHECK: expected a single type followed by a type pack

int valuePack<let each P : int>() { return __index_of_type_in_pack<int, P>; }
// CHECK: expected a type

int missing() { return __index_of_type_in_pack<bool, expand each Tuple<int, float>>; }
// CHECK: type is absent from the pack in __index_of_type_in_pack

int duplicate() { return __index_of_type_in_pack<float, expand each Tuple<float, float>>; }
// CHECK: type occurs more than once in the pack in __index_of_type_in_pack

int empty() { return __index_of_type_in_pack<int, expand each Tuple<>>; }
// CHECK: type is absent from the pack in __index_of_type_in_pack
