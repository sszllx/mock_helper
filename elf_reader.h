#ifndef ELF_READER_H
#define ELF_READER_H

#include <stdio.h>
#include <assert.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>

#include <elf.h>

#define SYMBOL_LENGTH 256

int parse_elf_sym(const char *name);

#endif // ELF_READER_H