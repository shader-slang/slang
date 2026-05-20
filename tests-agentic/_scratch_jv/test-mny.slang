//TEST:INTERPRET(filecheck=CHECK):
int sum8(int a, int b, int c, int d, int e, int f, int g, int h)
{ return a+b+c+d+e+f+g+h; }
void main()
{
    // CHECK: s=36
    printf("s=%d\n", sum8(1,2,3,4,5,6,7,8));
}
