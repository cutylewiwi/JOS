#ifndef LIBDISASM_H
#define LIBDISASM_H

#include "bastard.h"
#include "extension.h"
#include "vm.h"
#include <inc/stdio.h>
#include <inc/string.h>

/* formats : */
#define NATIVE_SYNTAX 0x00
#define INTEL_SYNTAX  0x01
#define ATT_SYNTAX    0x02

/* i386.h options : */
#define IGNORE_NULLS    0x01  /* don't disassemble sequences of > 4 NULLs */
#define MODE_16_BIT     0x02  /* use useless 16bit mode */


struct instr {
    char    mnemonic[16];
    char    dest[32];
    char    src[32];
    char    aux[32];
    int     mnemType;
    int     destType;
    int     srcType;
    int     auxType;
    int     size;
};


int disassemble_init(int options, int format);
int disassemble_cleanup(void);
int sprint_addrexp(char *str, int len, struct addr_exp *e);
int disassemble_address(char *buf, struct instr *i);
int sprint_address(char *str, int len, char *buf);
void ext_arch_init(void *param);
void ext_arch_cleanup();
int disasm_addr(unsigned char *buf, int tbl, struct code *c, long rva);
#endif
