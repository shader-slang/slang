
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -vk -compute -shaderobj
// The test currently does not work in dx12 because of a bug in the fxc compiler
// (The code gets compiled with an array inside of a struct,
// and somehow fxc just "forgets" to emit the array in the assembly)
// To resolve it: wait for a fix in fxc, move to dxc or process the IR to avoid emitting arrays inside of structs
//TEST_DISABLED(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -dx12 -compute -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -cpu -compute -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -cuda -compute -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -wgpu -compute -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -metal -compute -shaderobj

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name output
RWStructuredBuffer<uint> output;

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
// CHECK-NEXT: 0
// CHECK-NEXT: 0
// CHECK-NEXT: 0
// CHECK-NEXT: 0

// CHECK-NEXT: 0

uint decodeNextUnicodePointFromUTF8<ByteReader : IMutatingFunc<uint>>(inout ByteReader utf8Reader)
{
	uint codePoint = 0;
	uint leading = utf8Reader();
	uint mask = 0x80;
	int count = 0;
	while ((leading & mask) != 0 && count < 4)
	{
		count++;
		mask >>= 1;
	}
	codePoint = (leading & (mask - 1));
	for (int i = 1; i <= count - 1; i++)
	{
		uint next = utf8Reader();
		if(next != 0)
		{
			codePoint <<= 6;
			codePoint += (next & 0x3F);
		}
		else
		{
			break;
		}
	}
	return codePoint;
}

__generic<int N>
extension ShortString<N>
{
	struct Reader : IMutatingFunc<uint>
	{
		ShortString<N> str;
		uint it = 0;

		__init(const in ShortString<N> str, uint it = 0)
		{
			this.str = str;
			this.it = it;
		}

		[mutating]
		uint operator()()
		{
			uint res = 0;
			if(it < N)
			{
				res = str[it];
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
		output[offset + i] = decodeNextUnicodePointFromUTF8(it) - ref[i];
	}
	// uint n;
	// uint i = 0;
	// do
	// {
	// 	n = it();
	// 	output[offset + i] = n;
	// 	++i;
	// } while(n != 0);
	return offset + N;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 gid : SV_DispatchThreadID)
{
	// https://en.wikipedia.org/wiki/List_of_Unicode_characters
	uint i = 0;
	i = test<3>("文字列", {'文', '字', '列'}, i);
	i = test<5>("∫ℓ@ñγ", {'∫', 'ℓ', '@', 'ñ', 'γ'}, i);
	i = test<7>("aâäãà🝛🯅", {'a', 'â', 'ä', 'ã', 'à', '🝛', '🯅'}, i);
	i = test<1>("\xFF", {'\xFF'}, i);
}