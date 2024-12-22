#include "exclib.h"

int main(void)
{
  TRY {
    TRY {
      TRY {
	THROW(3, NULL);
      } EXCEPT {
      } FINALLY {
      } ETRY;
    } EXCEPT {
    } FINALLY {
    } ETRY;
  } EXCEPT {
  } FINALLY {
  } ETRY;

  return 0;
}
