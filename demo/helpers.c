#include "exclib.h"

int main(void)
{
  TRY {
    THROW_ZERO(NULL, EXC_NULLPOINTER, "I just threw with THROW_ZERO");
  } CATCH ( EXC_NULLPOINTER ) {
    THROW_NONZERO(strcmp("not", "equal"),
		  3,
		  "strcmp was nonzero!");
  } ETRY;

  return 0;
}
