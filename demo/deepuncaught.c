#include "exclib.h"

int main(void)
{
  TRY {
    TRY {
      TRY {
	THROW(3, NULL);
      } CLEANUP {
      } EXCEPT {
      } FINALLY {
      } ETRY;
    } CLEANUP {
    } EXCEPT {
    } FINALLY {
    } ETRY;
  } CLEANUP {
  } EXCEPT {
  } FINALLY {
  } ETRY;

  return 0;
}
