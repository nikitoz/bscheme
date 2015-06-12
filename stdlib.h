#ifndef __STDLIB__BSCHEME__
#define __STDLIB__BSCHEME__

void fatal_error(const char* message);
int  is_numeric(const char* str);
int  is_boolean(const char* str);
void assert(int value, const char* message);
int  is_whitespace(char ch);
int  charcmp(void* ch1, void* ch2);
#endif
