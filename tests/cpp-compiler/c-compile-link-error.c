//TEST(smoke):CPP_COMPILER_EXECUTE: 

#include <stdlib.h>
#include <stdio.h>

extern int thing;

int main(int argc, char** argv)
{
    printf("Hello World %d!\n", thing);
	return 0;
}
