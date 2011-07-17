#ifndef __EXCLIB_H__
#define __EXCLIB_H__

#include <setjmp.h>
#include <execinfo.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>

/*
 * These defines create a primitive sort of exception handling for bare C. Some things of note:
 *
 * 1- There is no dynamic memory allocation, ever, unless we print backtrace (in which case it's not our code doing it, it's execinfo)
 * 2- We work in our own sort of exception context stack (__exc_statuses) to do this; we can only ever have EXC_BT_FRAMES number of frames tracked at any given time. This must be defined at compile time.
 * 3- The benefit of the 2nd point is that we don't do malloc/free in our exception handling, and we can also keep from overwriting the current context within multiple nested TRY/ETRY blocks (and yes I've already tried, just scoping the operators {} does not help)
 * 4- THROW() is smart enough to know when it is being used outside the context of a TRY block, and it will raise its own exception/traceback
 * 5- Uncaught exceptions print a stacktrace; will have file names if debug is compiled and symbols aren't mangled, otherwise addr2line is your friend
 * 6- Uncaught exceptions generate SIGKILL on the current process to help w/ graceful shutdown
 * 7- The current exception frame is always available as EXC_STATUS_LIST, and is of type (struct exc_status)
 * 8- All exceptions store a stacktrace at the time their TRY block is encountered, though it is not necessarily printed
 * 9- Each member of the exception stack is currently ~700 bytes, so beware of making EXC_BT_FRAMES too large (the default, 50, is already unimaginably deep and adds ~35kB to your memory usage automatically). Don't be afraid to make EXC_BT_FRAMES smaller; most programs won't need more than 10 or 15.
 * 10- The underlying mechanism behind this is sigsetjmp / siglongjmp, two "arcane and slightly dangerous" functions. Essentially this is used to treat your *entire* codebase as something we can GOTO between when there's an exception. I don't think it will, but if this does funny things to your code, I'm sorry.
 * 11- You should limit your involvement with this library to the all-capital #defines. The functions themselves are undocumented and may change without warning.
 * 12- I haven't tried it, but this should be safe for C++ as well. Beware of mangled symbols in tracebacks. (C++ already has exception handling, you shouldn't need this there, I'm just saying it should work.)
 * 13- It is impossible to THROW(0). A compile time error will occur complaining about duplicate case value. If by some miracle you do get it to compile, you will enter an infinite loop, as there is no exit path for this.
 *
 * Model usage:
 * TRY {
 *    ... do something here ...
 *    THROW(int, "descriptive message")
 * } CATCH(int) {
 *    ... do error handling here ...
 * } DEFAULT {
 *    ... Do something with an exception here without caring about its value ...
 * } FINALLY {
 *    ... Do something here ...
 * } ETRY;
 *
 * The syntax is fairly obvious. Code inside the TRY block is executed, and any exceptions are propagated out to the CATCH blocks.
 * All exceptions are integers, no exceptions (zing!). DEFAULT, if present, is executed when there is no CATCH block for the specific
 * integer exception. If DEFAULT is not present and there is no CATCH block, then the exception is propagated back up the stack to the
 * first TRY() block encountered in the program. (Unhandled exceptions behave as in #5,6 in the bullet list above.)
 *
 * The FINALLY clause is executed after the try block has executed, and after any applicable CATCH/DEFAULT blocks. If present, it will
 * be executed REGARDLESS of whether an exception was actually caught. This is useful to examine an exception as it passes through
 * without actually handling/modifying it.
 *
 * THROW_ZERO is a convenience function to replace blocks like this:
 *
 *     if ( !some_pointer )
 *         THROW(EXC_NULLPOINTER, "null pointer!");
 *
 * ... to become:
 *
 *     THROW_ZERO(some_pointer, EXC_NULLPOINTER, "null pointer!");
 *
 * THROW_NONZERO is a wrapper you can use around libc functions that return nonzero on failure. The usage is fairly obvious:
 *
 *     THROW_NONZERO(strcmp("one", "two"), 0, "not equal!");
 *
 * ... where the second argument is an integer which will be added to the return code of the expression, and can be used to base
 * more complex exception codes around old -1/0/1 return code structures. For example you could have:
 *
 * #define EXC_CSTR_LESS 999
 * #define EXC_CSTR_EQUAL 1000
 * #define EXC_CSTR_GREATER 1001
 *
 * TRY {
 *    THROW_NONZERO(strcmp("one", "two"), EXC_CSTR_EQUAL, "not equal!");
 * } CATCH (EXC_CSTR_LESS) {
 * } CATCH_GROUP (EXC_CSTR_GREATER) {
 *    ...
 * } CATCH (EXC_NULLPOINTER) {
 *    ...
 * }
 *
 * This example also highlights one of the problems of this library : You can't check for multiple exceptions on the same block,
 * because it's all actually a hidden 'switch' statement. But you can use a mechanism like shown above; when  you have multiple
 * conditions that could match the same situation, list the first one as CATCH, and the proceeding ones as CATCH_GROUP. CATCH_GROUP
 * does not provide a break in the flow between the previous case and the current one, so fallthrough is achieved. Just make
 * sure to use a CATCH on any proceeding exceptions that don't need to fall through (like the EXC_NULLPOINTER above).
 *
 */

#ifndef EXC_BT_FRAMES
#define EXC_BT_FRAMES      50
#endif // EXC_BT_FRAMES

#ifndef EXC_MAX_EXCEPTIONS
#define EXC_MAX_EXCEPTIONS  65535
#endif // EXC_MAX_EXCEPTIONS

#define TRY \
if (__exc_curidx >= EXC_BT_FRAMES) \
    exclib_print_stacktrace("No available exception stack context", __FILE__, (char *)__func__, __LINE__); \
exclib_new_exc_frame(&__exc_statuses[__exc_curidx++], __FILE__, (char *)__func__, __LINE__); \
EXC_STATUS_LIST->value = sigsetjmp(EXC_STATUS_LIST->buf, 1); \
switch( EXC_STATUS_LIST->value ) { case 0:

#define CATCH(x) \
        break; \
    case x: \
        EXC_STATUS_LIST->caught = 1;

#define CATCH_GROUP(x) \
    case x: \
        EXC_STATUS_LIST->caught = 1;

#define DEFAULT \
        break; \
    default: \
        EXC_STATUS_LIST->caught = 1;

#define ETRY \
}; \
exclib_clear_exc_frame(); \
__exc_curidx--;

#define FINALLY \
}; \
if ( EXC_STATUS_LIST->caught || EXC_STATUS_LIST->tried ) {

#define THROW_NONZERO(x, y, z) int rc = (x); if ( rc != 0 ) THROW(rc + y, z);
#define THROW_ZERO(x, y, z) if ( !x ); THROW(y, z);

#define THROW(x, y)	 \
if ( EXC_STATUS_LIST ) { \
    exclib_new_exc_frame(EXC_STATUS_LIST, __FILE__, (char *)__func__, __LINE__); \
    EXC_STATUS_LIST->value = x; \
    EXC_STATUS_LIST->name = __exc_names[x]; \
    EXC_STATUS_LIST->description = y;	    \
    siglongjmp(EXC_STATUS_LIST->buf, x); \
} else { \
    exclib_print_stacktrace("Tried to THROW Exception "#x" but had no exception context. (Called outside of TRY block?)", __FILE__, (char *)__func__, __LINE__); \
    kill(getpid(), SIGKILL); \
}

struct exc_status {
    struct exc_status *next;
    struct exc_status *prev;
    jmp_buf buf;
    int value;
    int caught;
    int tried;
    char *file;
    char *function;
    int line;
    char *name;
    char *description;
    void *bt_frames[EXC_BT_FRAMES];
    size_t bt_size;
};

extern char *__exc_names[EXC_MAX_EXCEPTIONS];
extern int __exc_curidx;
struct exc_status __exc_statuses[EXC_BT_FRAMES];
extern struct exc_status *EXC_STATUS_LIST;

extern void exclib_init_strings();
extern void exclib_name_exception(int value, char *name);
extern void exclib_print_stacktrace(char *msg, char *file, char *func, int line);
extern void exclib_print_exc_stacktrace(char *msg);
extern int exclib_new_exc_frame(struct exc_status *es, char *file, char *function, int line);
extern int exclib_clear_exc_frame();

#endif // __EXCLIB_H__
