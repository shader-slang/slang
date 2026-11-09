//TEST:SIMPLE(filecheck=ALPHA): -target hgbc -entry main -DTEST_ALPHA
//TEST:SIMPLE(filecheck=BETA): -target hgbc -entry main

#ifdef TEST_ALPHA
import runtime_identity_alpha;
typealias SelectedItem = AlphaItem;
#else
import runtime_identity_beta;
typealias SelectedItem = BetaItem;
#endif

public struct Wrapper<T>
{
    public T value;
}

// The element types have the same name hint and layout. Their module-qualified nominal
// identities must survive inside arrays and multiple levels of generic specialization.
[host]
public float main(
    Wrapper<Wrapper<SelectedItem[2]>> value)
{
    return value.value.value[0].value;
}

// ALPHA-DAG: [[NAME:%[A-Za-z0-9_]+]] : {{.*}} = OpStringLit {{[0-9]+}} "_SR1N{{[0-9]+}}:_ST{{[^"]*}}7Wrapperg1TG1[N{{[0-9]+}}:_SR1{{[^"]*}}runtime_identity_alpha4Item{{[^"]*}}"
// ALPHA-DAG: OpDecorate TypeMangledName [[VALUE:%[A-Za-z0-9_]+]] [[NAME]]
// ALPHA-DAG: OpFuncType {{%[A-Za-z0-9_]+}} [[VALUE]]
// BETA-DAG: [[NAME:%[A-Za-z0-9_]+]] : {{.*}} = OpStringLit {{[0-9]+}} "_SR1N{{[0-9]+}}:_ST{{[^"]*}}7Wrapperg1TG1[N{{[0-9]+}}:_SR1{{[^"]*}}runtime_identity_beta4Item{{[^"]*}}"
// BETA-DAG: OpDecorate TypeMangledName [[VALUE:%[A-Za-z0-9_]+]] [[NAME]]
// BETA-DAG: OpFuncType {{%[A-Za-z0-9_]+}} [[VALUE]]
