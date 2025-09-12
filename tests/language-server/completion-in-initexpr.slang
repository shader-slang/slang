//TEST:LANG_SERVER(filecheck=CHECK):
//COMPLETE:17,21
struct MyType
{
    // Regression Condition 1: there must be more than one member in the lookup scope.
    float v;
    int getSum() { return 0; }
}

void m(MyType t)
{
    // Regression condition 2: the completion must be in an init expression.
    // Regression condition 3: none of the candidate members can coerce to the expected type.
    // Regression behavior: no completion candidates are shown, because
    // SemanticsVisitor::resolveOverloadedLookup throws an error when there are 0 applicable candidates
    // after type coercion filtering.
    Texture2D x = t.;
}

// CHECK: getSum
