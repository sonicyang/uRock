#ifndef CLIB_H
#define CLIB_H

/* Altough there already has string_utils
 * i would like seperate to my version of c library and the original
 * NOTE: string-utils.c use string.h's declarations
 */

#include <stdint.h>

/* fprintf for fio */
size_t fio_printf(int fd, const char *format, ...);
int sprintf(char *, const char *, ...);

/* I would like to rename itoa as to_string (idea from C++11) 
 * however c doesn't allow function overloading */
char *itoa(const char *numbox, int i, unsigned int base);
char *utoa(const char *numbox, unsigned int i, unsigned int base);

char *strcat(char * restrict dest, const char * restrict source);
size_t strlen(const char *str);


#endif
