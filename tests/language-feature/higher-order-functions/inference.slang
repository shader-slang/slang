//TEST:SIMPLE:
//TEST:SIMPLE:

func x<A, B>(f : A -> B)
{ }

float f(int)
{
    return 0;
}

func y<A, B, C>(g : (A, B) -> C)
{ }

void g<A>(A, bool)
{ }

void main()
{
    x(f);
    y(g<int>);
}
