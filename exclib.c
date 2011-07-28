#include "exclib.h"
#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>

int __exc_curidx = 0;
int __exc_initstrings = 0;
struct exc_status *EXC_STATUS_LIST = &__exc_statuses[0];
char *__exc_names[EXC_MAX_EXCEPTIONS];
int __exc_signals[EXC_MAX_EXCEPTIONS];

struct exc_name_data __exclib_exc_names[EXC_PREDEFINED_EXCEPTIONS] = {
    {EXC_NULLPOINTER, "Null Pointer", SIGSEGV},
    {EXC_OUTOFBOUNDS, "Array Index Out of Bounds", SIGTERM}
};

void exclib_print_exception_stack(char *mbuf, char *file, char *func, int line)
{
    char buf[256];
    char flagbuf[256];
    char *excname = NULL;
    struct exc_status *cur = EXC_STATUS_LIST;
    int idx = 0;

    fprintf(stderr,
	    "EXCLIB: %s:%d:%s: %s\n",
	    file,
	    line,
	    func,
	    mbuf);    

    /*fprintf(stderr,
	    "%s:%d:%s: Contents of the Exception Stack at this time:\n",
	    file,
	    line,
	    func);*/

    if ( ! cur )
	fprintf(stderr, "EXCLIB: #0: No exception stack\n");
    while ( cur != NULL ) {
	memset((char *)&buf, 0, 256);
	memset((char *)&flagbuf, 0, 256);
	if ( cur->catching == 1) {
	  if ( strlen((char *)&flagbuf) > 0 )
	    strcat((char *)&flagbuf, ",");
	  strcat((char *)&flagbuf, "catching");
	} 
	if ( cur->caught == 1) {
	  if ( strlen((char *)&flagbuf) > 0 )
	    strcat((char *)&flagbuf, ",");
	  strcat((char *)&flagbuf, "caught");
	} 
	if ( cur->tried == 1) {
	  if ( strlen((char *)&flagbuf) > 0 )
	    strcat((char *)&flagbuf, ",");
	  strcat((char *)&flagbuf, "tried");
	}

	if ( cur->name )
	  excname = cur->name;
	else
	  excname = "NULL";
	sprintf((char *)&buf,
		"EXCLIB: #%d[0x%x] %s:%d:%s:%s:%d:%s: %s\n",
		idx,
		cur,
		cur->file,
		cur->line,
		cur->function,
		excname,
		cur->value,
		(char *)&flagbuf,
		cur->description);
	fprintf(stderr, "%s", (char *)&buf);
	idx += 1;
	cur = cur->prev;
    }
}

void exclib_bulk_name_exceptions(struct exc_name_data *exclib_exc_names, int size)
{
    struct exc_name_data *ptr;
    int i = 0;
    exclib_init_strings();
    //THROW_ZERO(exclib_exc_names, EXC_NULLPOINTER, "Null Pointer");
    ptr = exclib_exc_names;
    for ( i = 0; i < size ; i++) {
	exclib_name_exception(ptr->exc,
			      ptr->name,
			      ptr->signal);
	ptr += sizeof(struct exc_name_data);
    }
}

void exclib_name_exception(int value, char *name, int signal)
{
    exclib_init_strings();
    __exc_names[value] = name;
    __exc_signals[value] = signal;
}

void exclib_init_strings()
{
    // many functions call this one as a prerequisite to make sure the string table
    // is safe, so we just check it here and return if we've already been called
    if ( __exc_initstrings == 1)
	return;
    int i = 0;
    // this takes some time but the security is worth it, also the ease of use
    for ( i = 0; i < EXC_MAX_EXCEPTIONS; i++) {
	__exc_names[i] = 0;
        __exc_signals[i] = SIGTERM;
    }
    // make sure all the exceptions we (the library) export are defined & named
    __exc_initstrings = 1;
    exclib_bulk_name_exceptions(&__exclib_exc_names[0], EXC_PREDEFINED_EXCEPTIONS);
}

void exclib_prep_throw(int value, char *msg, char *file, char *func, int line)
{
    char stbuf[256];
    memset((char *)&stbuf, 0, 256);
    if ( EXC_STATUS_LIST && EXC_STATUS_LIST->tried == 1) {
	sprintf((char *)&stbuf, "Tried to THROW %d but couldn't create new exception frame", value);
	if ( EXC_STATUS_LIST->catching == 1 && EXC_STATUS_LIST->prev ) {
	  if ( exclib_new_exc_frame(EXC_STATUS_LIST, file, func, line) ) {
	    exclib_print_exception_stack((char *)&stbuf, file, func, line);
	    exit(value);
	  }
	  memcpy(EXC_STATUS_LIST->buf, EXC_STATUS_LIST->prev->buf, sizeof(jmp_buf));
        } /*else if ( EXC_STATUS_LIST->catching == 1 && (EXC_STATUS_LIST->prev == NULL) ) {
	  if ( exclib_new_exc_frame(&__exc_statuses[__exc_curidx++], file, func, line) ) {
		exclib_print_exception_stack((char *)&stbuf, file, func, line);
		exit(value);
	  }
	  EXCLIB_TRACE("Condition 2");
	  exclib_clear_exc_frame();
	  __exc_curidx--;
	  } */ else {
	  if ( exclib_new_exc_frame(EXC_STATUS_LIST, file, func, line) )
	    exclib_print_exception_stack((char *)&stbuf, file, func, line);
	}
	EXC_STATUS_LIST->value = value;
	EXC_STATUS_LIST->name = __exc_names[value];
	EXC_STATUS_LIST->description = msg;
    } else {
	sprintf((char *)&stbuf, "Tried to THROW Exception %d but had no exception context. (Called outside of TRY block, or thrown while TRY was setting up?)", value);
	exclib_print_exception_stack((char *)&stbuf, file, func, line);
	exit(__exc_signals[value]);
    }
}


int exclib_new_exc_frame(struct exc_status *es, char *file, char *function, int line)
{
    int i = 0;
    if ( !es )
	return 1;
    exclib_init_strings();
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
    es->tried = 0;
    es->catching = 0;
    EXC_STATUS_LIST = es;
    return 0;
}

int exclib_clear_exc_frame()
{
    struct exc_status *es = EXC_STATUS_LIST;
    if ( !es ) {
	exclib_print_exception_stack("exclib_clear_exc_frame was called but there were no exception frames to clear!",
			   __FILE__, (char *)__func__, __LINE__);
	exit(1);
    }
    if ( es->prev )
	es->prev->next = NULL;
    if ( es->value && !es->caught ) {
	// thrown exception was unhandled - do we have anywhere else to go?
        if ( !es->prev ) {
	  // No frame above us to propagate this into, stacktrace and kill ourselves
	  exclib_print_exception_stack("Uncaught exception", __FILE__, (char*)__func__, __LINE__);
	  exit(es->value);
	  //kill(getpid(), SIGKILL);
	} else if ( es->tried && es->prev && (es->catching == 0)) {
	    // copy this exception up into the upper frame and siglongjmp back to that
	    EXC_STATUS_LIST = es->prev;
	    EXC_STATUS_LIST->caught = 0;
	    memcpy(&EXC_STATUS_LIST->bt_frames, &es->bt_frames, (sizeof(void *)*EXC_BT_FRAMES));
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
	es->catching = 0;
    }
}

