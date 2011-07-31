#include "exclib.h"
#include <stdio.h>

int main(void)
{
  TRY {
    THROW(2, NULL);
  } FINALLY {
    printf("I am in the finally clause, and I am about to issue an unhandled exception error.\n");
  } ETRY;
  return 0;
}
