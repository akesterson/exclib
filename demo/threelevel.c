#include "exclib.h"

int main(void)
{
  TRY {
    TRY {
      THROW(3, NULL);
    } CATCH(5) {
      EXCLIB_TRACE("Caught 5");
    } ETRY;
  } CATCH(3) {
    EXCLIB_TRACE("Caught 3");
  } ETRY;

  return 0;
}
