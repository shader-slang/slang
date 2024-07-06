//TEST:SIMPLE(filecheck=METAL): -target metal -stage fragment -entry fragmentMain
//TEST:SIMPLE(filecheck=METALLIB): -target metallib -stage fragment -entry fragmentMain

// METAL: color(7)
// METAL: color(6)
// METAL-DAG: color({{1|2|3|4}})
// METAL-DAG: color({{1|2|3|4}})
// METAL: color(0)
// METAL-DAG: color({{1|2|3|4}})
// METAL-DAG: color({{1|2|3|4}})

// METALLIB: @fragmentMain

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
    float p1 : CoarseVertex7;
    MiddleFragment1 p2 : CoarseVertex6;
    MiddleFragment1 p3 : CoarseVertex0;
};

struct FragmentStageInput
{
	TopFragment	coarseVertex : CoarseVertex;
};

float4 fragmentMain(FragmentStageInput input)
{
    // METAL-DAG: {{.*}}.p1{{.*}}=

    // METAL-DAG: {{.*}}.p2{{.*}}.p1{{.*}}=
    // METAL-DAG: {{.*}}.p2{{.*}}.p2{{.*}}.p1{{.*}}=
    // METAL-DAG: {{.*}}.p2{{.*}}.p3{{.*}}.p1{{.*}}=

    // METAL-DAG: {{.*}}.p3{{.*}}.p1=
    // METAL-DAG: {{.*}}.p3{{.*}}.p2{{.*}}.p1{{.*}}=
    // METAL-DAG: {{.*}}.p3{{.*}}.p3{{.*}}.p1{{.*}}=

    outputBuffer[0] = input.coarseVertex.p1 + input.coarseVertex.p2.p1 + +input.coarseVertex.p3.p1;
    return float4(0, 0, 0, 0);
}
