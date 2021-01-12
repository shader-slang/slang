#pragma pack_matrix(column_major)

#line 21 "tests/bugs/multiple-definitions.slang"
[numthreads(1, 1, 1)]
void main()
{

#line 21
    return;
}

