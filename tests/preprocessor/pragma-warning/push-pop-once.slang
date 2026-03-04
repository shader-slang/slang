//TEST:SIMPLE(filecheck=CHECK):
// #ifdef support
int64_t GetValue();
void f()
{
	int i;
#pragma warning (once : 30081)
	// CHECK: warning[E30081]
	i = GetValue();
#pragma warning (push)
#pragma warning (once : 30081)
#pragma warning (pop)
	// CHECK-NOT: warning[E30081]
	i = GetValue();
}

// Test that #pragma warning (push) and (pop) with (once) 