#ifndef K_PATH_H
#define K_PATH_H

#include "k_defs.h"

uint32_t k_fs_ParsePath(struct k_fs_ptrace_t *trace, char *path);

void k_fs_FreePathTrace(struct k_fs_ptrace_t *trace);

#endif