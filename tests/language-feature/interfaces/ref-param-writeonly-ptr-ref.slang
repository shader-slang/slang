// `Access.WriteOnly` is only valid as the access of `RefParam`.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv

void f(Ptr<int, Access.WriteOnly> p) {}
/*CHECK:
       ^^^ E30035
       ^^^ 'Ptr<int, Access.WriteOnly, AddressSpace.Device, DefaultDataLayout>' cannot use 'Access.WriteOnly'; it is only valid as the access of 'RefParam'.
*/

void g(Ref<int, Access.WriteOnly> r) {}
/*CHECK:
       ^^^ E30035
       ^^^ 'Ref<int, Access.WriteOnly, AddressSpace.Device, DefaultDataLayout>' cannot use 'Access.WriteOnly'; it is only valid as the access of 'RefParam'.
*/
