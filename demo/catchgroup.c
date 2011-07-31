#include "exclib.h"

int main(void)
{
  TRY {
    THROW(2, NULL);
  } CATCH_GROUP(1) {
  } CATCH_GROUP(2) {
  } CATCH_GROUP(3) {
    EXCLIB_TRACE("Inside of CATCH_GROUP");
  } ETRY;
  return 0;
}
