//TEST:LANG_SERVER(filecheck=CHECK):

// Regression test for #12760: a user-defined `[__AttributeUsage]` attribute must appear
// exactly once in attribute-list completion, as a Keyword (14), not also as a Struct (22).

[__AttributeUsage(_AttributeTargets.Function)]
struct ExampleAttribute
{
    int value;
}

//COMPLETE:13,2
[]
void target() {}

// CHECK-NOT: {{^}}Example:
// CHECK: {{^}}Example: 14
// CHECK-NOT: {{^}}Example:
