//TEST:SIMPLE(filecheck=METAL): -target metal -stage fragment -entry fragmentMain
//TEST:SIMPLE(filecheck=METALLIB): -target metallib -stage fragment -entry fragmentMain
//TEST:SIMPLE(filecheck=WGSL): -target wgsl -stage fragment -entry fragmentMain

//METAL-DAG: color(0)
//METAL-DAG: color(1)
//METAL-DAG: color(2)
//METAL-DAG: color(3)
//METAL-DAG: color(4)
//METAL-DAG: color(5)
//METAL-DAG: color(6)
//METAL-NOT: color(7)

//WGSL-DAG: location(0)
//WGSL-DAG: location(1)
//WGSL-DAG: location(2)
//WGSL-DAG: location(3)
//WGSL-DAG: location(4)
//WGSL-DAG: location(5)
//WGSL-DAG: location(6)
//WGSL-NOT: location(7)

//METALLIB: @fragmentMain

RWStructuredBuffer<float> outputBuffer;

struct BottomFragment1
{
    float p1;
};
struct BottomFragment2
{
    float p1;
};

struct MiddleFragment1
{
    float p1;
    BottomFragment1 p2;
    BottomFragment2 p3;
};
struct TopFragment
{
	float p1;
    MiddleFragment1 p2;
    MiddleFragment1 p3;
};

struct FragmentStageInput
{
	float4	coarseVertex	: CoarseVertex;
};

struct FragmentStageOutput
{
	TopFragment fragment	: SV_Target;
};

FragmentStageOutput fragmentMain(FragmentStageInput input)
{
    FragmentStageOutput output;
    output.fragment.p1 = 1;

    output.fragment.p2.p1 = 3;
    output.fragment.p2.p2.p1 = 4;
    output.fragment.p2.p3.p1 = 5;

    output.fragment.p3.p1 = 8;
    output.fragment.p3.p2.p1 = 9;
    output.fragment.p3.p3.p1 = 10;

    // METAL-DAG: ={{.*}}.p1

    // METAL-DAG: ={{.*}}.p2{{.*}}.p1
    // METAL-DAG: ={{.*}}.p2{{.*}}.p2{{.*}}.p1
    // METAL-DAG: ={{.*}}.p2{{.*}}.p3{{.*}}.p1

    // METAL-DAG: ={{.*}}.p3{{.*}}.p1
    // METAL-DAG: ={{.*}}.p3{{.*}}.p2{{.*}}.p1
    // METAL-DAG: ={{.*}}.p3{{.*}}.p3{{.*}}.p1

    // WGSL-DAG: ={{.*}}._S{{.*}}

    // WGSL-DAG: ={{.*}}.p2{{.*}}._S{{.*}}
    // WGSL-DAG: ={{.*}}.p2{{.*}}.p2{{.*}}._S{{.*}}
    // WGSL-DAG: ={{.*}}.p2{{.*}}.p3{{.*}}._S{{.*}}

    // WGSL-DAG: ={{.*}}.p3{{.*}}._S{{.*}}
    // WGSL-DAG: ={{.*}}.p3{{.*}}.p2{{.*}}._S{{.*}}
    // WGSL-DAG: ={{.*}}.p3{{.*}}.p3{{.*}}._S{{.*}}

	outputBuffer[0] = 1;
	return output;
}
