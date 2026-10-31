public interface IFoo
{
    associatedtype Assoc;
}

public extern struct A : IFoo;
public extern struct B : IFoo;
uniform A.Assoc data;

[shader("compute")]
[numthreads(1, 1, 1)]
void main() {}
