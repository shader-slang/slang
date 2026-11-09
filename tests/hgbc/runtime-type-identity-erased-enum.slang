//TEST:SIMPLE(filecheck=CHECK): -target hgbc -entry main

public enum First : int { value = 1 }
public enum Second : int { value = 2 }

public struct Token<T>
{
    public T value;
}

// HGBC identity follows lowered types. Both enum arguments become the same IR integer type
// before specialization, so the two parameters deliberately share one runtime Token type.
[host]
public int main(Token<First> first, Token<Second> second)
{
    return int(first.value) + int(second.value);
}

// CHECK-DAG: [[TOKEN_NAME:%[A-Za-z0-9_]+]] : {{.*}} = OpStringLit {{[0-9]+}} "_SR1N{{[0-9]+}}:_ST{{[^"]*}}5Tokeng1TG1[{{[^"]*}}"
// CHECK-DAG: OpDecorate TypeMangledName [[TOKEN:%[A-Za-z0-9_]+]] [[TOKEN_NAME]]
// CHECK-DAG: OpFuncType {{%[A-Za-z0-9_]+}} [[TOKEN]] [[TOKEN]]
