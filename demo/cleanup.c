#include "exclib.h"

int main(void)
{
  int val = 512;
  int *ptr = &val;

  TRY {
    THROW(3, "Checking to ensure cleanup behavior executes before EXCEPT block");
  } CLEANUP {
    ptr = NULL;
  } EXCEPT {
  } CATCH(3) {
    if ( ptr != NULL ) {
      EXCLIB_TRACE("pointer was not reset to NULL in first CLEANUP block");
      return 1;
    }
  } FINALLY {
  } ETRY;

  ptr = &val;
  TRY {
    /* No exception thrown here, CLEANUP should still execute */
  } CLEANUP {
    ptr = NULL;
  } EXCEPT {
  } FINALLY {
  } ETRY;

  if ( ptr != NULL ) {
    EXCLIB_TRACE("pointer was not reset to NULL in first TRY block");
    return 1;
  }

  return 0;
}
