//TEST:SIMPLE: -Itests/front-end/subdir

// Confirming import lookup via search paths work

__import import_subdir_a;

float bar(float x) { return foo(x); }