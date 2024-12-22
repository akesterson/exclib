#include "exclib.h"

/*
 * What this demo shows:
 * 1- The general usage of this library
 * 2- That a basic usage example works
 */

int main(void)
{
  EXCLIB_TRACE("No stack to print?");

  TRY {
    THROW(3, NULL);
  } CLEANUP {
  } EXCEPT {
  } CATCH(3) {
      EXCLIB_TRACE("Caught 3");
  } FINALLY {
  } ETRY;

  EXCLIB_TRACE("Exiting program");

  return 0;

}
