#include "exclib.h"
#include <stdio.h>

void nullfunc(int *something)
{
    THROW_ZERO(something, EXC_NULLPOINTER, NULL);
    printf("nullfunc got %d\n", *something);
}

void badfunc(void)
{
    // This function represents a function that you think you can trust; but there's a null
    // pointer being passed here that you have no control over. Thankfully 'nullfunc' was
    // written to be safe, so it will illustrate the usefulness of TRY/CATCH/ETRY.
    int *ptr = NULL;
    nullfunc(ptr);
}

void testfunc(void)
{
    THROW(34, "Random test exception");
}

int main(void)
{
    int x = 2;

    printf("Stack frames are %d bytes in size\n\n", sizeof(struct exc_status));

    TRY {
	fprintf(stdout, "Throwing %d\n", x);
	THROW(2, "First exception");
    } CATCH(-1) {
	printf("Caught -1\n");
	THROW(1, "1 Exception");
    } CATCH(1) {
	printf("Caught 1\n");
    } CATCH(2) {
	printf("Caught 2\n");
	TRY {
	    testfunc();
	} CATCH(34) {
	    printf("Caught 34!\n");
	    printf("Throwing 34 Back Out\n");
	    THROW(34, "Second-layer exception 34");
	} ETRY;
    } CATCH(34) {
	printf("Caught 34 in upper level!\n");
	printf("Using THROW_NONZERO\n");
	THROW_NONZERO(strcmp("a", "b"), 0, "String Compare Exception");
    } FINALLY {
	exclib_print_exception_stack("In finally clause", __FILE__, (char *)__func__, __LINE__);
    } ETRY;

    TRY {
	badfunc();
    } ETRY;

    /*printf("\nThrowing an exception with no toplevel TRY block, which should stacktrace and kill us...\n\n");

    THROW(34, "Bad Exception");*/
    return 0;
}
