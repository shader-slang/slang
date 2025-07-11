
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -vk -compute -shaderobj
//TEST_DISABLED(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -dx12 -compute -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -cpu -compute -shaderobj

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name output
RWStructuredBuffer<uint> output;

// Check that ShortString::getArray() is correctly resolved
// even when not inlined in the peephole pass

// CHECK: 0
// CHECK-NEXT: 0
// CHECK-NEXT: 0
// CHECK-NEXT: 0
// CHECK-NEXT: 0
// CHECK-NEXT: 0
// CHECK-NEXT: 0
// CHECK-NEXT: 0
// CHECK-NEXT: 0
// CHECK-NEXT: 0
// CHECK-NEXT: 0

__generic<int N>
extension ShortString<N>
{
	struct Reader : IMutatingFunc<uint8_t>
	{
		ShortString<N> str;
		uint it = 0;

		__init(const in ShortString<N> str, uint it = 0)
		{
			this.str = str;
			this.it = it;
		}

		[mutating]
		uint8_t operator()()
		{
			uint8_t res = 0;
			if(it < N)
			{
				res = str.getArray()[it];
				++it;
			}
			return res;
		}
	}
	
	Reader begin()
	{
		return Reader(this);
	}
}

uint test<int N, int M>(const in ShortString<M> str, Array<uint, N> ref, uint offset)
{
	var it = str.begin();
	for(uint i = 0; i < N; ++i)
	{
		output[offset + i] = it() - ref[i];
	}
	return offset + N;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 gid : SV_DispatchThreadID)
{
	uint i = 0;
	i = test<11>("Lorem Ipsum", {'L', 'o', 'r', 'e', 'm', ' ', 'I', 'p', 's', 'u', 'm'}, i);
}