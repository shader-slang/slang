//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-vk -compute -shaderobj -output-using-type

//TEST_INPUT:ubuffer(data=[0 333 0 0], stride=4):name=sys.entries
//TEST_INPUT:ubuffer(data=[0 0 0 0 0], stride=4):out,name=output

//TEST_INPUT: type_conformance m.ConcreteA:m.IFoo = 0

module main;
import iface;
import type_a;
import type_b;
using m;

ParameterBlock<System> sys;
RWStructuredBuffer<int> output;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    IFoo f = sys.get_foo(0);

    output[0] = f.get_val();
    output[5] = int(sys.entries[0].header);


    output[1] = (f is ConcreteA) ? 1 : 0;

    if (let a = f as ConcreteA)
    {
        output[2] = int(a.payload_a);
    }
    else
    {
        output[2] = -999;
    }

    output[3] = (f is ConcreteB) ? 1 : 0;

    if (let b = f as ConcreteB)
    {
        output[4] = int(b.payload_b);
    }
    else
    {
        output[4] = -999;
    }
}

// BUF: type: int32_t
// BUF-NEXT: 333
// BUF-NEXT: 1
// BUF-NEXT: 333
// BUF-NEXT: 0
// BUF-NEXT: -999
