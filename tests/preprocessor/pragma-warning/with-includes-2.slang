//TEST:SIMPLE(filecheck=CHECK):
// #ifdef support
#include "helper-2.slang"

int64_t GetValue();

void f()
{
	int i;
	// CHECK: warning[E30081]
	i = GetValue();
}

// Test that the #pragma warning (push / pop) englobing helper-2.slang are working