//DIAGNOSTIC_TEST:SIMPLE:

// Here we test that diagnostics appear once per usage of each of the
// deprecated declarations.

[deprecated("please use foo(bool)")]
void foo(int) {}

void foo(bool) {}

[deprecated("non-overloaded bar")]
void bar() {}

const float newPi = 3.1;

[deprecated("more precise approximations are available elsewhere")]
const int pi = 3;

[deprecated("no")]
struct Billiards
{
    [deprecated("puns")]
    bool bs[22];
}

int main()
{
    pi;

    foo(0);
    foo(false); // doesn't warn

    Billiards b;
    b.bs;

    bar();

    return 0;
}
