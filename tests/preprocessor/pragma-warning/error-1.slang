//TEST:SIMPLE(filecheck=CHECK):
// #ifdef support

int64_t GetValue();

void f()
{
	int i;
#pragma warning (error : 30081)
	// CHECK: error[E30081]
	i = GetValue();
}

// Simple test of #pragma warning (error)