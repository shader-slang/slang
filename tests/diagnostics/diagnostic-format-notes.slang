//TEST:SIMPLE(filecheck=CHECK): -target hlsl -diagnostic-format vs

// A structured diagnostic carries a note locating the previous definition.
// CHECK: diagnostic-format-notes.slang(21,6): error E30201: function 'f' already has a body
// CHECK: diagnostic-format-notes.slang(20,6): note: see previous definition of 'f'

#line 20 "diagnostic-format-notes.slang"
void f() {}
void f() {}
