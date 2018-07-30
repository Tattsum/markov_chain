#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>

extern void eprintf(char *, ...);
extern char *estrdup(char *);
extern char *progname(void);
extern void *emalloc(size_t);
