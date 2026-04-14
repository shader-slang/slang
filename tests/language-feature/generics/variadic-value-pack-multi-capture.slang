//TEST:INTERPRET(filecheck=CHECK):

void add(inout int a, int b) { a += b; }

// Expand capturing two packs: expand (each C) * (each D)
int dotProduct<let each C : int, let each D : int>()
{
    int s = 0;
    expand add(s, (each C) * (each D));
    return s;
}

// Expand capturing two packs with addition: expand (each C) + (each D)
int sumPairs<let each C : int, let each D : int>()
{
    int s = 0;
    expand add(s, (each C) + (each D));
    return s;
}

void main()
{
    // C = {1, 2, 3}, D = {4, 5, 6}
    // dot = 1*4 + 2*5 + 3*6 = 4 + 10 + 18 = 32
    printf("%d\n", dotProduct<1, 2, 3, 4, 5, 6>());
    // CHECK: 32

    // C = {10, 20}, D = {3, 4}
    // sumPairs = (10+3) + (20+4) = 13 + 24 = 37
    printf("%d\n", sumPairs<10, 20, 3, 4>());
    // CHECK: 37
}
