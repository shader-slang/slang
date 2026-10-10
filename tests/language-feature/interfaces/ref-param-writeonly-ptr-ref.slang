// `Access.WriteOnly` is only valid as the access of `RefParam`.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv

void f(Ptr<int, Access.WriteOnly> p) {}
//CHECK:               ^^^^^^^^^ E30035
//CHECK:               ^^^^^^^^^ 'Ptr' cannot use 'Access.WriteOnly'; it is only valid as the access of 'RefParam'.

void g(Ref<int, Access.WriteOnly> r) {}
//CHECK:               ^^^^^^^^^ E30035
//CHECK:               ^^^^^^^^^ 'Ref' cannot use 'Access.WriteOnly'; it is only valid as the access of 'RefParam'.
