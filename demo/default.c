#include "exclib.h"
#include <stdio.h>

int main(void)
{
  TRY {
    THROW(2, NULL);
  } DEFAULT {
    EXCLIB_TRACE("Inside of DEFAULT - I catch everything!");
  } ETRY;
  return 0;
}
