#include "exclib.h"
#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>

int __exclib_curidx = 0;
int __exclib_inited = 0;
struct exclib_status __exclib_statuses[EXC_MAX_FRAMES];
struct exclib_status *EXCLIB_EXCEPTION = &__exclib_statuses[0];
char *__exclib_names[EXC_MAX_EXCEPTIONS];
int __exclib_rc;
char __exclib_strbuf[EXC_STRBUF_SIZE];

struct exclib_name_data __exclib_exc_names[EXC_PREDEFINED_EXCEPTIONS] = {
    {EXC_NULLPOINTER, "Null Pointer", SIGSEGV},
    {EXC_OUTOFBOUNDS, "Array Index Out of Bounds", SIGTERM}
};

void exclib_print_exception_stack(char *mbuf, char *file, char *func, int line)
{
    char buf[512];
    char flagbuf[256];
    char *excname = NULL;
    struct exclib_status *cur = EXCLIB_EXCEPTION;
    int idx = 0;

    fprintf(stderr,
	    "EXCLIB: %s:%d:%s: %s\n",
	    file,
	    line,
	    func,
	    mbuf);    

    for ( idx = 0; idx < EXC_MAX_FRAMES; idx++ ) {
      if ( __exclib_statuses[idx].file != NULL )
	cur = &__exclib_statuses[idx];
    }
    idx = 0;

    if ( ! cur )
	fprintf(stderr, "EXCLIB: #0: No exception stack\n");
    else {
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
	if ( cur->thrown == 1) {
	  if ( strlen((char *)&flagbuf) > 0 )
	    strcat((char *)&flagbuf, ",");
	  strcat((char *)&flagbuf, "thrown");
	}

	if ( cur->name )
	  excname = cur->name;
	else
	  excname = "NULL";
	sprintf((char *)&buf,
		"EXCLIB: #%d[0x%lx] %s:%d:%s:%s:%d:%s: %s\n",
		idx,
		(unsigned long int)cur,
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
}

void exclib_bulk_name_exceptions(struct exclib_name_data *exclib_exc_names, int size)
{
    struct exclib_name_data *ptr;
    int i = 0;
    /*THROW_ZERO(exclib_exc_names, EXC_NULLPOINTER, "Null Pointer");*/
    ptr = exclib_exc_names;
    for ( i = 0; i < size ; i++) {
	exclib_name_exception(ptr->exc, ptr->name);
	ptr += sizeof(struct exclib_name_data);
    }
}

void exclib_name_exception(int value, char *name)
{
    __exclib_names[value] = name;
}

void exclib_init()
{
  int i = 0;
  if ( __exclib_inited == 1 )
    return;
  __exclib_inited = 1;
  memset(&__exclib_statuses, 0x00, sizeof(struct exclib_status) * EXC_MAX_FRAMES);
  /* for whatever reason memset isn't safe here, so ... */
  for ( i = 0 ; i < EXC_MAX_EXCEPTIONS; i++ ) {
    __exclib_names[i] = NULL;
  }
  exclib_bulk_name_exceptions(&__exclib_exc_names[0], EXC_PREDEFINED_EXCEPTIONS);
}

void exclib_prep_throw(int value, char *msg, char *file, char *func, int line, int setflag)
{
    if ( EXCLIB_EXCEPTION && EXCLIB_EXCEPTION->tried == 1) {
		sprintf((char *)&__exclib_strbuf, "Tried to THROW %d but couldn't create new exception frame", value);
		if ( EXCLIB_EXCEPTION->catching == 1 && EXCLIB_EXCEPTION->prev ) {
			if ( exclib_new_exc_frame(EXCLIB_EXCEPTION, file, func, line) ) {
				exclib_print_exception_stack((char *)&__exclib_strbuf, file, func, line);
				exit(value);
			}
			memcpy(EXCLIB_EXCEPTION->buf, EXCLIB_EXCEPTION->prev->buf, sizeof(jmp_buf));
			if ( setflag )
				EXCLIB_EXCEPTION->thrown = 1;
			} else if ( EXCLIB_EXCEPTION->catching == 1 && (EXCLIB_EXCEPTION->prev == NULL) ) {
				exclib_clear_exc_frame();
			} else {
			if ( exclib_new_exc_frame(EXCLIB_EXCEPTION, file, func, line) )
				exclib_print_exception_stack((char *)&__exclib_strbuf, file, func, line);
			if ( setflag )
				EXCLIB_EXCEPTION->thrown = 1;
		}
		if ( setflag ) {
			EXCLIB_EXCEPTION->value = value;
			EXCLIB_EXCEPTION->name = __exclib_names[value];
			EXCLIB_EXCEPTION->description = msg;
		}
		return;
    }
    sprintf((char *)&__exclib_strbuf, "Tried to THROW Exception %d but had no exception context. (Called outside of TRY block, or thrown while TRY was setting up?)", value);
    exclib_print_exception_stack((char *)&__exclib_strbuf, file, func, line);
    exit(value);
}


int exclib_new_exc_frame(struct exclib_status *es, char *file, char *function, int line)
{
    int i = 0;
    exclib_init();
    if ( !es )
	return 1;
    if (EXCLIB_EXCEPTION && EXCLIB_EXCEPTION != es) {
	EXCLIB_EXCEPTION->next = es;
	es->prev = EXCLIB_EXCEPTION;
    } else if (!EXCLIB_EXCEPTION) {
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
    EXCLIB_EXCEPTION = es;
    return 0;
}

int exclib_clear_exc_frame()
{
    struct exclib_status *es = EXCLIB_EXCEPTION;
    if ( !es ) {
	exclib_print_exception_stack("exclib_clear_exc_frame was called but there were no exception frames to clear!",
			   __FILE__, (char *)__func__, __LINE__);
	exit(1);
    }
    if ( es->prev )
      es->prev->next = NULL;
    if ( es->thrown && !es->caught ) {
	/* thrown exception was unhandled - do we have anywhere else to go? */
        if ( !es->prev ) {
	  /* No frame above us to propagate this into, stacktrace and kill ourselves */
	  exclib_print_exception_stack("Uncaught exception", es->file, es->function, es->line);
	  exit(es->value);
	} else if ( es->tried && es->prev && (es->catching == 0)) {
	  /* copy this exception up into the upper frame and siglongjmp back to that */
	  EXCLIB_EXCEPTION = es->prev;
	  EXCLIB_EXCEPTION->caught = 0;
	  EXCLIB_EXCEPTION->name = es->name;
	  EXCLIB_EXCEPTION->description = es->description;
	  int val = es->value;
	  THROW_EXPLICIT(val, 
			 EXCLIB_EXCEPTION->description, 
			 EXCLIB_EXCEPTION->file,
			 EXCLIB_EXCEPTION->function,
			 EXCLIB_EXCEPTION->line,
			 0);
	}
    } else {
      if ( es->prev) {
	EXCLIB_EXCEPTION = es->prev;
      }
      memset((void *)es, 0x00, sizeof(struct exclib_status));
    }
    return 0;
}

