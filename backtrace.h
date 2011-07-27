#ifndef __BACKTRACE_H__
#define __BACKTRACE_H__

extern int backtrace(void **buffer, int size);
extern char **backtrace_symbols(void *const *buffer, int size);

#endif // __BACKTRACE_H__
