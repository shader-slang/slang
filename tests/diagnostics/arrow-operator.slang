//DIAGNOSTIC_TEST:SIMPLE(diag=diag): -target spirv -emit-spirv-directly

struct Type
{
    int member;
}

struct CB
{
    Type* ptr;
}
ConstantBuffer<CB> cb;

[numthreads(1,1,1)]
void main()
{
    Type val;

    val->member = 2; // Error.
//diag:^^ cannot dereference type
//diag:^^ cannot dereference type 'Type', do you mean to use '.'?

    // OK.
    let a = cb->ptr->member;

}
