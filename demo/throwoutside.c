#include "exclib.h"

int main(void)
{
  TRY {
    TRY {
      THROW(3, NULL);
    } EXCEPT {
    } CATCH(5) {
      EXCLIB_TRACE("Caught 5");
    } FINALLY {
    } ETRY;
  } EXCEPT {
  } CATCH(3) {
    EXCLIB_TRACE("Caught 3");
  } FINALLY {
  } ETRY;

  return 0;
}
