//TEST:SIMPLE(filecheck=CHECK): -target hgbc -entry main -g2 -validate-ir

[noinline]
int marker(int x) { return x + 1; }
[ForceInline]
int inner(int x) { return marker(x); }
[ForceInline]
int outer(int x) { { return inner(x) + inner(x + 1); } }
[host]
void main() { print(outer(1)); }

// CHECK: Section("META")
// CHECK-DAG: [[MAIN:%[A-Za-z0-9_]+]] = OpDebugFunction %strMain
// CHECK-DAG: [[OUTER:%[A-Za-z0-9_]+]] = OpDebugFunction %strOuter
// Function bodies use the function scope directly; only the nested braces add a scope.
// CHECK-DAG: [[OUTER_BLOCK:%[A-Za-z0-9_]+]] = OpDebugLexicalBlock [[OUTER]]
// CHECK-DAG: [[MAIN_SITE:%[A-Za-z0-9_]+]] = OpDebugInlinedAt [[MAIN]] {{.*}} 10 {{[0-9]+}} %id0
// Both sibling inner calls have the same outer chain, not a chain through each other.
// Their caller scope is the nested lexical block, not just the outer function.
// CHECK-DAG: {{%[A-Za-z0-9_]+}} = OpDebugInlinedAt [[OUTER_BLOCK]] {{.*}} 8 {{[0-9]+}} [[MAIN_SITE]]
// CHECK-DAG: {{%[A-Za-z0-9_]+}} = OpDebugInlinedAt [[OUTER_BLOCK]] {{.*}} 8 {{[0-9]+}} [[MAIN_SITE]]
// CHECK: Section("FUNC")
// CHECK: OpAdd
// CHECK: OpDebugScope [[MAIN]]
// CHECK: OpDebugLine {{%[A-Za-z0-9_]+}} 10 10
// CHECK: OpPrint
