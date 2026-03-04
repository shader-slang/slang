//TEST:SIMPLE(filecheck=CHECK):
// #ifdef support

int64_t GetValue();

void f()
{
	int i;
#pragma warning (once : 30081)
	// CHECK: warning[E30081]
	i = GetValue();
	// CHECK-NOT: warning[E30081]
	i = GetValue();
}

// Simple test of #pragma warning (once)