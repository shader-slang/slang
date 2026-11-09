//TEST:SIMPLE(filecheck=CHECK): -target hgbc -entry main

[host]
public void main(int x, int y)
{
    print(x < y);
}

// CHECK-DAG: [[INT:%[A-Za-z0-9_]+]] = OpIntType
// CHECK: {{%[A-Za-z0-9_]+}} : {{%[A-Za-z0-9_]+}} = OpLess [[INT]]
