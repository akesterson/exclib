#include "exclib.h"

int main(void)
{
    exclib_register_signals();
    TRY {
	exclib_print_exception_stack(__FILE__, (char *)__func__, __LINE__);
	THROW(2, NULL);
    } CATCH(2) {
	exclib_print_exception_stack(__FILE__, (char *)__func__, __LINE__);
	THROW(3, NULL);
    } CATCH(3) {
    } FINALLY {
	exclib_print_exception_stack(__FILE__, (char *)__func__, __LINE__);
	return 0;
    } ETRY;

}
