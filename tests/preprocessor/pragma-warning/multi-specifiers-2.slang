//TEST:SIMPLE(filecheck=CHECK):
// #ifdef support

int64_t GetValue();

void f()
{
	int i;
#pragma warning (disable : 30081 15205)
	// CHECK-NOT: warning[E30081]
	i = GetValue();
	// CHECK-NOT: warning[E15205]
#if MY_MACRO

#endif
}

// Test a warning specifier on two ids