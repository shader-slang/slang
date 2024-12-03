//TEST:SIMPLE(filecheck=METAL): -target metal -stage fragment -entry fragmentMain
//TEST:SIMPLE(filecheck=METALLIB): -target metallib -stage fragment -entry fragmentMain
//TEST:SIMPLE(filecheck=WGSL): -target wgsl -stage fragment -entry fragmentMain

// METAL: COARSEVERTEX_7
// METAL: COARSEVERTEX_6

// Ensure each attribute which may vary only appears once.
// Ensure 1, 2, 3, 4 all appear

// METAL-DAG: [[ATTR1:COARSEVERTEX_(1|2|3|4)]]

// METAL-NOT: [[ATTR1]]
// METAL-DAG: [[ATTR2:COARSEVERTEX_(1|2|3|4)]]

// METAL: COARSEVERTEX{{(_0|())}}

// METAL-NOT: [[ATTR2]]
// METAL-DAG: [[ATTR3:COARSEVERTEX_(1|2|3|4)]]

// METAL-NOT: [[ATTR3]]
// METAL-DAG: [[ATTR4:COARSEVERTEX_(1|2|3|4)]]

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
    // METAL-DAG: {{.*}}->p1{{.*}}=

    // METAL-DAG: {{.*}}->p2{{.*}}->p1{{.*}}=
    // METAL-DAG: {{.*}}->p2{{.*}}->p2{{.*}}->p1{{.*}}=
    // METAL-DAG: {{.*}}->p2{{.*}}->p3{{.*}}->p1{{.*}}=

    // METAL-DAG: {{.*}}->p3{{.*}}->p1{{.*}}=
    // METAL-DAG: {{.*}}->p3{{.*}}->p2{{.*}}->p1{{.*}}=
    // METAL-DAG: {{.*}}->p3{{.*}}->p3{{.*}}->p1{{.*}}=

    // WGSL-DAG: {{.*}}._S{{.*}}=

    // WGSL-DAG: {{.*}}.p2{{.*}}._S{{.*}}=
    // WGSL-DAG: {{.*}}.p2{{.*}}.p2{{.*}}._S{{.*}}=
    // WGSL-DAG: {{.*}}.p2{{.*}}.p3{{.*}}._S{{.*}}=

    // WGSL-DAG: {{.*}}.p3{{.*}}._S{{.*}}=
    // WGSL-DAG: {{.*}}.p3{{.*}}.p2{{.*}}._S{{.*}}=
    // WGSL-DAG: {{.*}}.p3{{.*}}.p3{{.*}}._S{{.*}}=

    outputBuffer[0] = input.coarseVertex.p1 + input.coarseVertex.p2.p1 + +input.coarseVertex.p3.p1;
    return float4(0, 0, 0, 0);
}
