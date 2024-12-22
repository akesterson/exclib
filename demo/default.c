#include "exclib.h"
#include <stdio.h>

int main(void)
{
  TRY {
    THROW(2, NULL);
  } CLEANUP {
  } EXCEPT {
  } DEFAULT {
    EXCLIB_TRACE("Inside of DEFAULT - I catch everything!");
    return 0;
  } FINALLY {
  } ETRY;
  return 1;
}
