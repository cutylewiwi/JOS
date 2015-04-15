#ifndef BASTARD_H
#define BASTARD_H
/* stripped-down combination bastard.h and bdb.h for libdisasm */
struct addr_exp {  
    int     scale;
    int     index;
    int     base;
    int     disp;
    int     flags;
    int     used;
};

struct code { 
    long rva;
    long func; /* unused in libdisasm */
    char    mnemonic[16];
    long    dest;
    long    src;
    long    aux;
    int     mnemType;
    int     destType;
    int     srcType;
    int     auxType;
    unsigned long    destConst;
    unsigned long    srcConst;
    unsigned long    auxConst;
};
struct code_effect {  /* size 16 */
    unsigned long    id;
    unsigned long    rva;
    int     reg;
    int     change;
};

#define BIG_ENDIAN_ORD       0
#define LITTLE_ENDIAN_ORD    1
 /* Operand and instruction types */
/*                   Permissions: */
#define OP_R         0x001      /* operand is READ */
#define OP_W         0x002      /* operand is WRITTEN */
#define OP_X         0x004      /* operand is EXECUTED */
/*                   Types: */
#define OP_UNK       0x000      /* unknown operand */
#define OP_REG       0x100      /* register */
#define OP_IMM       0x200      /* immediate value */
#define OP_REL       0x300      /* relative Address [offset] */
#define OP_ADDR      0x400      /* Absolute Address */
#define OP_EXPR      0x500      /* Address Expression [e.g. SIB byte] */
#define OP_PTR       0x600      /* Operand is an Address containing a Pointer */
#define OP_OFF       0x700      /* Operand is an offset from a seg/selector */
/*                   Modifiers: */
#define OP_SIGNED    0x001000   /* operand is signed */
#define OP_STRING    0x002000   /* operand a string */
#define OP_CONST     0x004000   /* operand is a constant */
#define OP_EXTRASEG  0x010000   /* seg overrides */
#define OP_CODESEG   0x020000
#define OP_STACKSEG  0x030000
#define OP_DATASEG   0x040000
#define OP_DATA1SEG  0x050000
#define OP_DATA2SEG  0x060000
/*                   Size: */
#define OP_BYTE      0x100000   /* operand is  8 bits/1 byte  */
#define OP_WORD      0x200000   /* operand is 16 bits/2 bytes */
#define OP_DWORD     0x300000   /* operand is 32 bits/4 bytes */
#define OP_QWORD     0x400000   /* operand is 64 bits/8 bytes */
/* operand masks */
#define OP_PERM_MASK 0x0000007  /* perms are NOT mutually exclusive */
#define OP_TYPE_MASK 0x0000F00  /* types are mututally exclusive */
#define OP_MOD_MASK  0x00FF000  /* mods are NOT mutual;y exclusive */
#define OP_SEG_MASK  0x00F0000  /* segs are NOT mutually exclusive */
#define OP_SIZE_MASK 0x0F00000  /* sizes are mutually exclusive */

#define OP_REG_MASK    0x0000FFFF /* lower WORD is register ID */
#define OP_REGTBL_MASK 0xFFFF0000 /* higher word is register type [gen/dbg] */

#define INS_BRANCH   0x01        /* Unconditional branch */
#define INS_COND     0x02     /* Conditional branch */
#define INS_SUB      0x04     /* Jump to subroutine */
#define INS_RET      0x08     /* Return from subroutine */
    /* modify ( 'w' ) instructions */
#define INS_ARITH    0x10       /* Arithmetic inst */
#define INS_LOGIC    0x20       /* logical inst */
#define INS_FPU      0x40       /* Floating Point inst */
#define INS_FLAG     0x80       /* Modify flags */
    /* misc Instruction Types */
#define INS_MOVE     0x0100
#define INS_ARRAY    0x0200   /* String and XLAT ops */
#define INS_PTR      0x0400   /* Load EA/pointer */
#define INS_STACK    0x1000   /* PUSH, POP, etc */
#define INS_FRAME    0x2000   /* ENTER, LEAVE, etc */
#define INS_SYSTEM   0x4000   /* CPUID, WBINVD, etc */
   /* instruction size */
#define INS_BYTE      0x10000   /* operand is  8 bits/1 byte  */
#define INS_WORD      0x20000   /* operand is 16 bits/2 bytes */
#define INS_DWORD      0x40000   /* operand is 32 bits/4 bytes */
#define INS_QWORD      0x80000   /* operand is 64 bits/8 bytes */
   /* instruction modifiers */
#define INS_REPZ     0x0100000
#define INS_REPNZ    0x0200000  
#define INS_LOCK     0x0400000 /* lock bus */
#define INS_DELAY    0x0800000 /* branch delay slot */


#define ADDEXP_SCALE_MASK  0x000000FF
#define ADDEXP_INDEX_MASK  0x0000FF00
#define ADDEXP_BASE_MASK   0x00FF0000
#define ADDEXP_DISP_MASK   0xFF000000
#define ADDEXP_SCALE_OFFSET 0
#define ADDEXP_INDEX_OFFSET 8
#define ADDEXP_BASE_OFFSET  16
#define ADDEXP_DISP_OFFSET  24
#define ADDREXP_BYTE    0x01
#define ADDREXP_WORD    0x02
#define ADDREXP_DWORD   0x03
#define ADDREXP_QWORD   0x04
#define ADDREXP_REG     0x10 /*0x00 implies non-register */

#define AddrExp_ScaleType(x) x & ADDEXP_SCALE_MASK
#define AddrExp_IndexType(x) (x & ADDEXP_INDEX_MASK) >> 8
#define AddrExp_BaseType(x) (x & ADDEXP_BASE_MASK) >> 16
#define AddrExp_DispType(x) (x & ADDEXP_DISP_MASK) >> 24



#define CODE_RVA 1
#define FUNCTION_PROLOGUE 1
#define FUNCTION_EPILOGUE 2

/* these are originally defined in bastard.h and libbastard; they are
 * provided here as a necessity for compatibility purposes, but are defined
 * lib libdisasm.c
 */


#endif
int AddRegTableEntry( int index, char *name, int size);
int addrexp_new(int scale,int index,int base,int disp,int flags);
int db_index_find(int, void *, void *);
int db_index_next(int, void *);
