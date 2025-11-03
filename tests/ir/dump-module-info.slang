//TEST:COMPILE: tests/ir/dump-module-info.slang -o tests/ir/dump-module-info.slang-module 

//TEST:SIMPLE(filecheck=CHECK): -get-module-info tests/ir/dump-module-info.slang-module

module "foo";

// CHECK: Module Name: foo
// CHECK: Module Version: {{.+}}
// Just check that this is in the output with some string
// CHECK: Compiler Version: {{.+}}


