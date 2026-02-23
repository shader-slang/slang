//TEST:SIMPLE(filecheck=CHECK):
// #ifdef support
#pragma warning (error : 30081)
#pragma warning (push)
#pragma warning (disable : 30081)
int64_t GetValue();
#pragma warning (pop)
void f()
{
	int i;
	// CHECK: error[E30081]
	i = GetValue();
}

// Test that #pragma warning (push) and (pop) work as expected