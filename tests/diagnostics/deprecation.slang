//DIAGNOSTIC_TEST:SIMPLE(diag=diag):

// Here we test that diagnostics appear once per usage of each of the
// deprecated declarations.

[deprecated("please use foo(bool)")]
void foo(int) {}

void foo(bool) {}

[deprecated("non-overloaded bar")]
void bar() {}

static const float newPi = 3.1;

[deprecated("more precise approximations are available elsewhere")]
static const int pi = 3;

[deprecated("no")]
struct Billiards
{
    [deprecated("puns")]
    bool bs[22];
//diag:  ^^ use of deprecated declaration
//diag:  ^^ bs has been deprecated: puns
//diag:  ^^ use of deprecated declaration
//diag:  ^^ bs has been deprecated: puns
}

int main()
{
    pi;
/*diag:
    ^^ use of deprecated declaration
    ^^ pi has been deprecated: more precise approximations are available elsewhere
*/

    foo(0);
/*diag:
    ^^^ use of deprecated declaration
    ^^^ foo has been deprecated: please use foo(bool)
*/

    foo(false); // doesn't warn

    Billiards b;
/*diag:
    ^^^^^^^^^ use of deprecated declaration
    ^^^^^^^^^ Billiards has been deprecated: no
*/

    b.bs;
/*diag:
      ^^ use of deprecated declaration
      ^^ bs has been deprecated: puns
*/

    bar();
/*diag:
    ^^^ use of deprecated declaration
    ^^^ bar has been deprecated: non-overloaded bar
*/

    return 0;
}
