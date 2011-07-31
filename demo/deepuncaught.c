#include "exclib.h"

int main(void)
{
  TRY {
    TRY {
      TRY {
	THROW(3, NULL);
      } ETRY;
    } ETRY;
  } ETRY;

  return 0;
}
