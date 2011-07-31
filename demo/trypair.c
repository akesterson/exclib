#include "exclib.h"

int main(void)
{
  TRY {
    THROW(3, NULL);
  } CATCH(3) {
  } ETRY;

  TRY {
    THROW(3, NULL);
  } CATCH(3) {
    TRY {
      THROW(3, NULL);
    } ETRY;
  } ETRY;

  EXCLIB_TRACE("Should never get here");
  return 0;

}
