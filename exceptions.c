#include <stdio.h>
#include <setjmp.h>

struct exc_status {
	struct exc_status *next;
	struct exc_status *prev;
	jmp_buf buf;
	int last_exception;
	int last_caught;
	int tried;
};

struct exc_status __exc_status_head;
struct exc_status *EXC_STATUS_LIST = &__exc_status_head;

int new_exc_frame(struct exc_status *es) 
{
	if ( !es )
		return 1;
	EXC_STATUS_LIST->next = es;
	es->next = NULL;
	es->prev = EXC_STATUS_LIST;
	es->last_exception = 0;
	es->last_caught = 0;
	es->tried = 0;
	EXC_STATUS_LIST = es;
	return 0;
}

int clear_exc_frame()
{
	struct exc_status *es = EXC_STATUS_LIST;
	if ( es->prev )
		es->prev->next = NULL;
	EXC_STATUS_LIST = es->prev;
	es->last_exception = 0;
	es->last_caught = 0;
	es->tried = 0;
}

jmp_buf exc_buf;
int EXC_LAST_EXCEPTION = 0;
int EXC_LAST_CAUGHT = 0;
int EXC_TRIED = 0;

#define TRY struct exc_status es; new_exc_frame(&es); es.last_exception = sigsetjmp(es.buf, 1); switch( es.last_exception ) { case 0:
#define CATCH(x) break; case x: es.last_caught = x;
#define DEFAULT break; default: es.last_caught = 1;
#define ETRY }; clear_exc_frame();
#define FINALLY }; if ( es.last_caught || es.tried ) {
#define THROW(x) EXC_STATUS_LIST->last_exception = x; siglongjmp(EXC_STATUS_LIST->buf, x);

void testfunc(void)
{
	printf("Throwing 34 from testfunc\n");
	THROW(34);
}

int main(void)
{
	TRY {
		printf("Throwing 2\n");
		THROW(2);
	} CATCH(1) {
		printf("Caught 1\n");
	} CATCH(2) {
		printf("Caught 2\n");
		TRY {
			testfunc();
		} CATCH(34) {
			printf("Caught 34!\n");
		} ETRY
	} DEFAULT {
		printf("In default clause\n");
	} FINALLY {
		printf("In finally clause\n");
	} ETRY
	return 0;
}
