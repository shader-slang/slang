//DIAGNOSTIC_TEST:SIMPLE():

// Use a regular test here rather than a DIAGNOSTIC_TEST because the test
// runner can't yet parse the new diagnostic style
//TEST:SIMPLE(): -enable-experimental-rich-diagnostics

void foo(int x) {}
void foo(float x) {}

void main()
{
    // This should trigger ambiguous overload diagnostic
    // Both overloads are equally applicable due to implicit conversion
    foo(1u);
}
