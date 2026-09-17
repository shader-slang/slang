//TEST:INTERPRET(filecheck=CHECK):

// Companion to interpreter-runtime-string-diagnostic.slang (issue #13017): a runtime
// `String` is unrepresentable however it is materialized, not only as a ternary result.
// Here it is returned from a function and bound to a named local, which exercises the
// valid-`sourceLoc` reporting branch (the ternary repro exercises the synthesized
// phi-parameter's first-use fallback).

bool cmpF(float a, float b)
{
    return a < b;
}

String choose(bool b)
{
    return b ? "yes" : "no";
}

void main()
{
    String s = choose(cmpF(1.0, 2.0));
    //CHECK: result code = 1
    //CHECK-DAG: error[E52014]
    //CHECK-DAG: 'string' has no HostVM layout
    //CHECK-NOT: error[E99999]
    //CHECK-NOT: error[E99997]
    printf("%s\n", s);
}
