//TEST:INTERPRET(filecheck=CHECK):

// A user aggregate that contains a `String` has no HostVM layout, so a runtime value
// of it is unrepresentable and must be diagnosed rather than silently allocated a
// 0-byte slot (issue #13017). Selecting one of two `Named` values at runtime produces
// such a value; the diagnostic names the aggregate type.

bool cmpF(float a, float b)
{
    return a < b;
}

struct Named
{
    String s;
}

void main()
{
    Named a = {"x"};
    Named b = {"y"};
    Named r = cmpF(1.0, 2.0) ? a : b;
    //CHECK: result code = 1
    //CHECK-NOT: error[E99999]
    //CHECK-NOT: error[E99997]
    // The aggregate `Named` (it contains a `String`) has no HostVM layout, so any runtime
    // `Named` value is unrepresentable. Per-type deduplication reports the aggregate once,
    // naming `Named` (in practice at the first site that needs its layout, e.g. the `a`
    // initializer), rather than separately at every `Named` slot.
    //CHECK-DAG: error[E52014]
    //CHECK-DAG: 'Named' has no HostVM layout
    //CHECK-NOT: error[E99999]
    //CHECK-NOT: error[E99997]
    printf("%s\n", r.s);
}
