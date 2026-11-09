// DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK): -target hgbc -entry main

[host]
void main()
{
    let grid = Grid(1u);
}

// CHECK: undefined identifier 'Grid'
