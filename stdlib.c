#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void fatal_error(const char* message) {
	if (message)
		printf(message);
	exit(1);
}

int is_numeric(const char* str) {
	int i = 0;
	if (str[0] == '-') ++i;
	for (; str[i]; ++i)
		if (str[i] < '0' || str[i] > '9')
			return 0;
	return 1;
}

int is_boolean(const char* str) {
	return 0 == strcmp("f", str) || 0 == strcmp("t", str);
}

void assert(int value, const char* message) {
	if (!value)
		fatal_error(message);
}

int is_whitespace(char ch) {
	return ch == '\n' || ch == ' ' || ch == '\t' || ch == '\r';
}

int charcmp(void* ch1, void* ch2) {
	return strcmp(ch1, ch2);
}
