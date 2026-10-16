//TEST:INTERPRET(filecheck=CHECK):

// A function whose result type has no HostVM layout is unrepresentable even when the
// returned value is a string literal: the literal follows the constant path and never
// reaches the value-slot allocator, so the function-result ABI query is the only place
// this is caught. Removing that guard would silently emit a 0-byte result (issue #13017).

String main()
{
    //CHECK: result code = 1
    //CHECK-DAG: error[E52014]
    //CHECK-DAG: 'string' has no HostVM layout
    //CHECK-NOT: error[E99999]
    //CHECK-NOT: error[E99997]
    return "literal";
}
