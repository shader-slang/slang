//TEST:REFLECTION(filecheck=REFLECT):-stage compute -entry computeMain -target spirv -no-codegen


struct X<int v, int k1, int k2>
{
    // If PolynomialIntVal's substitution logic is implemented correctly,
    // reflection should see that the `data` array has 36 elements.
    // Previously, there is a bug in the substitution logic that causes
    // us to mistakenly report elementCount as 6
    // (missing the multiplication of a factor's power).
    //
    // REFLECT: "elementCount": 36
    int data[v*v*k1*k2];
}

uniform X<6,1,1> x;

[numthreads(1,1,1)]
void computeMain()
{}