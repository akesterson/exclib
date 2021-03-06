#include "exclib.h"

/*
 * What this example shows:
 * 1- That an exception will propagate up out of the current TRY/CATCH context
 * 2- That an uncaught exception will generate an error
 */

int main(void)
{
  TRY {
    THROW(3, NULL);
  } CATCH(3) {
    THROW(3, NULL);
  } ETRY;

  EXCLIB_TRACE("Should never get here");

  return 0;

}
