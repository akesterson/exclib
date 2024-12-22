#include "exclib.h"

int main(void)
{
  TRY {
    THROW(3, NULL);
  } EXCEPT {
  } CATCH(3) {
  } FINALLY {
  } ETRY;

  TRY {
    THROW(3, NULL);
  } EXCEPT {
  } CATCH(3) {
      TRY {
	THROW(3, NULL);
      } EXCEPT {
      } FINALLY {
      } ETRY;
  } EXCEPT {
  } FINALLY {
  } ETRY;

  EXCLIB_TRACE("Should never get here");
  return 0;

}
