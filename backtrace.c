/*
  Based on mingw32 "backtrace" library provided by Cloud Wu, here:
  http://code.google.com/p/backtrace-mingw/

  Modified to provide the same interface that execinfo.h does on GNU/Unix hosts.
*/

#ifdef WIN32

#include <windows.h>
#include <excpt.h>
#include <imagehlp.h>
#include <bfd.h>
#include <psapi.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdbool.h>

#define BUFFER_MAX 1024
char **symbols;
char scratch[BUFFER_MAX];

struct bfd_ctx {
	bfd * handle;
	asymbol ** symbol;
};

struct bfd_set {
	char * name;
	struct bfd_ctx * bc;
	struct bfd_set *next;
};

struct find_info {
	asymbol **symbol;
	bfd_vma counter;
	const char *file;
	const char *func;
	unsigned line;
};

void lookup_section(bfd *abfd, asection *sec, void *opaque_data)
{
	struct find_info *data = opaque_data;

	if (data->func)
		return;

	if (!(bfd_get_section_flags(abfd, sec) & SEC_ALLOC)) 
		return;

	bfd_vma vma = bfd_get_section_vma(abfd, sec);
	if (data->counter < vma || vma + bfd_get_section_size(sec) <= data->counter) 
		return;

	bfd_find_nearest_line(abfd, sec, data->symbol, data->counter - vma, &(data->file), &(data->func), &(data->line));
}

void find(struct bfd_ctx * b, DWORD offset, const char **file, const char **func, unsigned *line)
{
	struct find_info data;
	data.func = NULL;
	data.symbol = b->symbol;
	data.counter = offset;
	data.file = NULL;
	data.func = NULL;
	data.line = 0;

	bfd_map_over_sections(b->handle, &lookup_section, &data);
	if (file) {
		*file = data.file;
	}
	if (func) {
		*func = data.func;
	}
	if (line) {
		*line = data.line;
	}
}

int init_bfd_ctx(struct bfd_ctx *bc, const char * procname)
{
	bc->handle = NULL;
	bc->symbol = NULL;

	bfd *b = bfd_openr(procname, 0);
	if (!b) {
		return 1;
	}

	int r1 = bfd_check_format(b, bfd_object);
	int r2 = bfd_check_format_matches(b, bfd_object, NULL);
	int r3 = bfd_get_file_flags(b) & HAS_SYMS;

	if (!(r1 && r2 && r3)) {
		bfd_close(b);
		return 1;
	}

	void *symbol_table;

	unsigned dummy = 0;
	if (bfd_read_minisymbols(b, FALSE, &symbol_table, &dummy) == 0) {
		if (bfd_read_minisymbols(b, TRUE, &symbol_table, &dummy) < 0) {
			free(symbol_table);
			bfd_close(b);
			return 1;
		}
	}

	bc->handle = b;
	bc->symbol = symbol_table;

	return 0;
}

void close_bfd_ctx(struct bfd_ctx *bc)
{
	if (bc) {
		if (bc->symbol) {
			free(bc->symbol);
		}
		if (bc->handle) {
			bfd_close(bc->handle);
		}
	}
}

struct bfd_ctx *get_bc(struct bfd_set *set , const char *procname)
{
	while(set->name) {
		if (strcmp(set->name , procname) == 0) {
			return set->bc;
		}
		set = set->next;
	}
	struct bfd_ctx bc;
	if (init_bfd_ctx(&bc, procname)) {
		return NULL;
	}
	set->next = calloc(1, sizeof(*set));
	set->bc = malloc(sizeof(struct bfd_ctx));
	memcpy(set->bc, &bc, sizeof(bc));
	set->name = strdup(procname);

	return set->bc;
}

void release_set(struct bfd_set *set)
{
	while(set) {
		struct bfd_set * temp = set->next;
		free(set->name);
		close_bfd_ctx(set->bc);
		free(set);
		set = temp;
	}
}

int backtrace(void **buffer, int size)
{
  if ( !buffer )
    return 1;
  bfd_init();
  struct bfd_set *set = calloc(1,sizeof(*set));
  _backtrace(buffer, size, set, NULL);
  release_set(set);
  return 0;
}

char **backtrace_symbols(void **buffer, int size)
{
  return symbols;
}

int _backtrace(void **buffer, int depth, struct bfd_set *set)
{
  char procname[MAX_PATH];
  LPCONTEXT context;
  struct bfd_ctx *bc = NULL;
  int i = depth;
  STACKFRAME frame;
  HANDLE process;
  HANDLE thread;
  char symbol_buffer[sizeof(IMAGEHLP_SYMBOL) + 255];
  char module_name_raw[MAX_PATH];
  IMAGEHLP_SYMBOL *symbol;
  DWORD module_base;
  const char * module_name;
  const char * file = NULL;
  const char * func = NULL;
  char tmpbuf[512];
  unsigned line = 0; 
  DWORD dummy;
  char *spos = &scratch;

  if ( symbols )
    free(symbols);
  symbols = (void **)malloc((sizeof(void *) * depth));
  if (!symbols)
    return 1;
  memset(symbols, 0, sizeof(void *) * depth);

  memset(spos, 0x00, BUFFER_MAX);

  memset(&frame,0,sizeof(frame));
  GetModuleFileNameA(NULL, procname, sizeof procname);
  process = GetCurrentProcess();
  thread = GetCurrentThread();
  GetThreadContext(thread, context);

  frame.AddrPC.Offset = context->Eip;
  frame.AddrPC.Mode = AddrModeFlat;
  frame.AddrStack.Offset = context->Esp;
  frame.AddrStack.Mode = AddrModeFlat;
  frame.AddrFrame.Offset = context->Ebp;
  frame.AddrFrame.Mode = AddrModeFlat;
  
  while(StackWalk(IMAGE_FILE_MACHINE_I386, 
		  process, 
		  thread, 
		  &frame, 
		  context, 
		  0, 
		  SymFunctionTableAccess, 
		  SymGetModuleBase, 0)) {
    
    i++;
    if ( i >= depth )
      break;

    symbol = (IMAGEHLP_SYMBOL *)symbol_buffer;
    symbol->SizeOfStruct = (sizeof *symbol) + 255;
    symbol->MaxNameLength = 254;
    
    module_base = SymGetModuleBase(process, frame.AddrPC.Offset);
    
    module_name = "[unknown module]";
    if (module_base && 
	GetModuleFileNameA((HINSTANCE)module_base, module_name_raw, MAX_PATH)) {
      module_name = module_name_raw;
      bc = get_bc(set, module_name);
    }
    
    file = NULL;
    func = NULL;
    line = 0;
      
    if (bc) {
      find(bc,frame.AddrPC.Offset,&file,&func,&line);
    }
    
    if (file == NULL) {
      dummy = 0;
      if (SymGetSymFromAddr(process, frame.AddrPC.Offset, &dummy, symbol)) {
	file = symbol->Name;
      }
      else {
	file = "[unknown file]";
      }
    }
    
    if (func == NULL) {
      buffer[i] = frame.AddrPC.Offset;
      sprintf((char *)&tmpbuf, "%s(NULL+0x??)[0x%x]\0",
	      module_name,
	      frame.AddrPC.Offset);
    }
    else {
      buffer[i] = frame.AddrPC.Offset;
      sprintf((char *)&tmpbuf, "%s(%s+0x??)[0x%x]\0",
	      module_name,
	      frame.AddrPC.Offset,
	      func);
    }
    strcpy(spos, (char *)&tmpbuf);
    symbols[i] = spos;
    spos += strlen((char *)&tmpbuf);
  }
}

#endif // WIN32
