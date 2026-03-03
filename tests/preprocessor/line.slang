//TEST:SIMPLE(filecheck=CHECK):
// #line support
// Foo* is not defined so expect diagnostics for each function

//CHECK: line.slang([[#@LINE+1]]):
FooA a() { return 0; }

//CHECK: b.slang(99):
#line 99 "b.slang"
FooB b() { return 0; }

//CHECK: line.slang([[#@LINE+2]]):
#line default
FooC c() { return 0; }

//CHECK: d.slang(603):
#line 603 "d.slang"
FooD d() { return 0; }

//CHECK: d.slang(40):
#line 40
FooE e() { return 0; }

//CHECK: line.slang([[#@LINE+2]]):
#line
FooF f() { return 0; }
