#ifndef VM_H
#define VM_H
struct REGTBL_ENTRY {
   int size;
   void *data;
   char mnemonic[8];
};
#endif

