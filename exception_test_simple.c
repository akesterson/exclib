#include "exclib.h"

int main(void)
{
  EXCLIB_TRACE("No stack to print?");

  TRY {
    EXCLIB_TRACE("Inside TRY");
    THROW(3, NULL);
  } CATCH(3) {
    EXCLIB_TRACE("Caught 3");
    THROW(5, NULL);
  } CATCH(5) {
    EXCLIB_TRACE("Caught 5");
    TRY {
      THROW(6, NULL);
    } CATCH (6) {
      EXCLIB_TRACE("Caught 6");
      THROW(7, NULL);
    } ETRY;
  } CATCH(7) {
    EXCLIB_TRACE("Caught 7");
  } FINALLY {
    EXCLIB_TRACE("In finally clause");
  } ETRY;

  EXCLIB_TRACE("Exiting program");

  return 0;

}
