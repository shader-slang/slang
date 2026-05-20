//TEST:INTERPRET(filecheck=CHECK):
interface ICallable0 { int call(); }
struct Const42 : ICallable0 { int call() { return 42; } }
int invokeZero<F : ICallable0>(F f) { return f.call(); }

interface ICallable8 { int call(int a, int b, int c, int d, int e, int f, int g, int h); }
struct Sum8 : ICallable8 { int call(int a, int b, int c, int d, int e, int f, int g, int h) { return a+b+c+d+e+f+g+h; } }
int invokeEight<F : ICallable8>(F f) { return f.call(1,2,3,4,5,6,7,8); }

void main()
{
    Const42 c;
    Sum8 s;
    // CHECK: zero=42
    printf("zero=%d\n", invokeZero(c));
    // CHECK: eight=36
    printf("eight=%d\n", invokeEight(s));
}
