//TEST:SIMPLE:
//TEST:SIMPLE:

func x<A, B>(f : functype (A) -> B)
{ }

float f(int)
{
    return 0;
}

func y<A, B, C>(g : functype (A, B) -> C)
{ }

void g<A>(A, bool)
{ }

void main()
{
    x(f);
    y(g<int>);
}
