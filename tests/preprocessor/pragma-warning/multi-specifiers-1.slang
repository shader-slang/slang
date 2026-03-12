//TEST:SIMPLE(filecheck=CHECK):
// #ifdef support

int64_t GetValue();

void f()
{
	int i;
#pragma warning (disable : 30081 ; error : 30081)
	// CHECK: error[E30081]
	i = GetValue();
}

// Test that the last specifier is applied