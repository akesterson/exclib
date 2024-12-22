#include "exclib.h"

int main(void)
{
  TRY {
    THROW(2, NULL);
  } EXCEPT {
  } CATCH_GROUP(1) {
  } CATCH_GROUP(2) {
  } CATCH_GROUP(3) {
    EXCLIB_TRACE("Inside of CATCH_GROUP");
  } FINALLY {
  } ETRY;
  return 0;
}
