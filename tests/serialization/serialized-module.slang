// serialized-module.slang

// A test to try out the basics of module
// serialization.

//TEST:SERIALIZED_MODULE:


#ifdef PRODUCER

int foo(int x) { return x; }


#endif

#ifdef CONSUMER

import serialized_module;

// TODO: do something with `foo`?

// TODO: need to get the name mangling to line up!
int foo(int x);




#endif