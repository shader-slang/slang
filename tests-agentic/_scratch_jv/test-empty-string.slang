//TEST:SIMPLE(filecheck=CHECK):-target cpp -entry main -stage compute
[numthreads(1, 1, 1)]
void main()
{
    String s = "";
    printf(s);
    //CHECK: ""
}
