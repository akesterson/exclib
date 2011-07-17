#include "exclib.h"
#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <execinfo.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>

int __exc_curidx = 0;
int __exc_initstrings = 0;
struct exc_status *EXC_STATUS_LIST = &__exc_statuses[0];

char *__exc_names[EXC_MAX_EXCEPTIONS];

void exclib_print_stacktrace(char *msg, char *file, char *func, int line)
{
    size_t bt_size;
    void *bt_frames[EXC_BT_FRAMES];
    char **strings;
    size_t i;

    if (!msg)
	msg = "Unknown error occured";

    fprintf(stderr,
	    "%s:%d:%s: %s\n",
	    file,
	    line,
	    func,
	    msg);

    bt_size = backtrace(bt_frames, EXC_BT_FRAMES);
    strings = backtrace_symbols(bt_frames, bt_size);
    for (i = 0; i < bt_size; i++)
	fprintf(stderr, "#%d: %s\n", i, strings[i]);
    free (strings);
}

void exclib_print_exc_stacktrace (char *msg)
{
    char **strings;
    size_t i;

    if ( !msg )
	msg = "Unhandled Exception";

    fprintf(stderr,
	    "%s:%d:%s: %s '%s' (%d) : %s\n",
	    EXC_STATUS_LIST->file,
	    EXC_STATUS_LIST->line,
	    EXC_STATUS_LIST->function,
	    msg,
	    EXC_STATUS_LIST->name,
	    EXC_STATUS_LIST->value,
	    EXC_STATUS_LIST->description);

    strings = backtrace_symbols (EXC_STATUS_LIST->bt_frames, EXC_STATUS_LIST->bt_size);
    for (i = 0; i < EXC_STATUS_LIST->bt_size; i++)
	fprintf(stderr, "#%d: %s\n", i, strings[i]);
    free (strings);
}

void exclib_name_exception(int value, char *name)
{
    exclib_init_strings();
    __exc_names[value] = name;
}

void exclib_init_strings()
{
    if ( __exc_initstrings = 1)
	return;
    int i = 0;
    // this takes some time but the security is worth it, also the ease of use
    for ( i = 0; i < EXC_MAX_EXCEPTIONS; i++) {
	__exc_names[i] = 0;
    }
    __exc_initstrings = 1;
}

int exclib_new_exc_frame(struct exc_status *es, char *file, char *function, int line)
{
    int i = 0;
    if ( !es )
	return 1;
    if (EXC_STATUS_LIST && EXC_STATUS_LIST != es) {
	EXC_STATUS_LIST->next = es;
	es->prev = EXC_STATUS_LIST;
    } else if (!EXC_STATUS_LIST) {
 	es->prev = NULL;
    }
    es->next = NULL;
    es->file = file;
    es->function = function;
    es->line = line;
    es->name = "Unnamed Exception";
    es->description = "No Description";
    es->value = 0;
    es->caught = 0;
    es->tried = 1;
    es->bt_size = backtrace(es->bt_frames, EXC_BT_FRAMES);
    EXC_STATUS_LIST = es;
    return 0;
}

int exclib_clear_exc_frame()
{
    struct exc_status *es = EXC_STATUS_LIST;
    if ( !es ) {
	exclib_print_stacktrace("exclib_clear_exc_frame was called but there were no exception frames to clear!",
			   __FILE__, (char *)__func__, __LINE__);
	kill(getpid(), SIGKILL);
    }
    if ( es->prev )
	es->prev->next = NULL;
    if ( !es->caught ) {
	// exception was unhandled - do we have anywhere else to go?
        if ( !es->prev ) {
	    // No frame above us to propagate this into, stacktrace and kill ourselves
	    // we do kill here instead of just exit() to try and let any custom signal
	    // handlers clean things up for us
	    exclib_print_exc_stacktrace(NULL);
	    kill(getpid(), SIGKILL);
	} else if ( es->tried ) {
	    // copy this exception up into the upper frame and siglongjmp back to that
	    EXC_STATUS_LIST = es->prev;
	    EXC_STATUS_LIST->caught = 0;
	    memcpy(&EXC_STATUS_LIST->bt_frames, &es->bt_frames, (sizeof(void *)*EXC_BT_FRAMES));
	    EXC_STATUS_LIST->bt_size = es->bt_size;
	    EXC_STATUS_LIST->file = es->file;
	    EXC_STATUS_LIST->function = es->function;
	    EXC_STATUS_LIST->line = es->line;
	    EXC_STATUS_LIST->name = es->name;
	    EXC_STATUS_LIST->description = es->description;
	    THROW(es->value, es->description);
	}
    } else {
	EXC_STATUS_LIST = es->prev;
	es->value = 0;
	es->caught = 0;
	es->tried = 0;
    }
}

