#include "libdis.h"

struct addr_exp exp[3]; /* one for dest, src, and aux in struct code */
int assembler_format;
struct EXT__ARCH ext_arch;

int disassemble_init(int options, int format){
   assembler_format = format;
   ext_arch.options = options;
   ext_arch_init( &ext_arch );
   return(1);
}
   
int disassemble_cleanup(void){
   ext_arch_cleanup();
   return(1);
}

char * get_reg_name(int index){
   if (index >= ext_arch.sz_regtable) return(0);
   return( ext_arch.reg_table[index].mnemonic );
}

int fmt_expr_op( int operand, int flags, char *buf, int len){
   if ( ! operand  && flags != ADDREXP_REG ) {
      buf[0] = '\0';
      return(0);
   }
   switch (flags) {
      case ADDREXP_REG:
         if (assembler_format == ATT_SYNTAX)
            snprintf(buf, len, "%%%s", get_reg_name(operand));
         else
            strncpy(buf, get_reg_name(operand), len);
         break;
      case ADDREXP_WORD:
         if (operand) snprintf(buf, len, "%4X", (short) operand);
         break;
      case ADDREXP_DWORD:
         if (operand) snprintf(buf, len, "%8X", operand);
         break;
      case ADDREXP_QWORD:
         if (operand) snprintf(buf, len, "%012X", operand);
         break;
      case ADDREXP_BYTE:
      default:
         if (operand) snprintf(buf, len, "%02X", (char) operand);
   }

   return(strlen(buf));
}

int sprint_addrexp(char *str, int len, struct addr_exp *e){
   char scale[32]={0}, index[32]={0}, base[32]={0}, disp[32]={0};
   char sd = '+', idx[16]={0}, tmp[32]={0};
   
   /* do scale */
   fmt_expr_op( e->scale, AddrExp_ScaleType(e->flags), scale, 32);
   /* do index */
   fmt_expr_op( e->index, AddrExp_IndexType(e->flags), index, 32);
   /* do byte */
   fmt_expr_op( e->base, AddrExp_BaseType(e->flags), base, 32);
   /* do disp */
   fmt_expr_op( e->disp, AddrExp_DispType(e->flags), disp, 32);

   switch (assembler_format) {
      case ATT_SYNTAX:
         if (disp[0]){
            snprintf(str, 32, "%c%s", sd, disp);
         }
         strncat(str, "(", 1);
         if (scale[0]) {
            strcat(str, scale);
         } 
         if (index[0]) {
            snprintf(str, 32, ", %s", index);
         } else if (disp[0]) {
            strncat(str, ",", 1);
         } 
         if (disp[0]) {
            snprintf(str, 32, ", %s", disp);
         }
         strncat(str, ")", 1);
         break;
      case INTEL_SYNTAX:
      case NATIVE_SYNTAX:
      default:
         if (e->disp  < 0) sd = '-';

         if (scale[0] && index[0])
            snprintf(idx, 16, "(%s * [%s])", scale, index);
         else if (index[0])
            snprintf(idx, 16, "[%s]", index);

         if (base[0]) {
            snprintf(str, len, "[%s]", base);
            if (idx[0]) {
               strncat(str, "+", len);
               strncat(str, idx, len);
            }
            if (disp[0]){
               snprintf(tmp, 32, "%c%s", sd, disp);
               strncat(str, tmp, len);
            }
         } else if (idx[0]) {
            snprintf(str, len, "%s%c%s", idx, sd, disp);
         } else {
            snprintf(str, len, "%c%s", sd, disp);
         }

   }
   return(strlen(str));
}


int sprint_seg(char *str, int len, int seg) {
   seg = seg >> 16;
   if (assembler_format == ATT_SYNTAX)
      snprintf( str, len, "%%%s:", get_reg_name(ext_arch.reg_seg + seg - 1));
   else
      snprintf( str, len, "%s:", get_reg_name(ext_arch.reg_seg + seg - 1));
   return(strlen(str));
}

int sprint_op(char *str, int len, int op, int type){
   int seg;

   if (! type) {
      memset(str, 0, len);
      return(0);
   }

   seg = type & OP_SEG_MASK; /* segment override for operand */

   switch (type & OP_TYPE_MASK) {
      case OP_REG:
         if (seg) str += sprint_seg(str, len, seg);
         if (assembler_format == ATT_SYNTAX) 
            snprintf(str, len, "%%%s", get_reg_name(op));
         else
            snprintf(str, len, "%s", get_reg_name(op));
         break;
      case OP_REL:
         if ( op < 0 ) strncat(str, "-", 1);
         else strncat(str, "+", 1);
         str++;
         snprintf(str, 31, "%X", op);
         break;
      case OP_OFF:
         if (seg) str += sprint_seg(str, len, seg);
         else str += sprint_seg(str, len, OP_DATASEG);
         snprintf(str, 31, "%X", op);
         break;
      case OP_PTR:
      case OP_ADDR:
         if (assembler_format == ATT_SYNTAX) 
            snprintf(str, len, "*0x%08X", op);
         else
            snprintf(str, len, "0x%08X", op);
         break;
      case OP_EXPR:
         if (seg) str += sprint_seg(str, len, seg);
         else str += sprint_seg(str, len, OP_DATASEG);
         sprint_addrexp(str, len, &exp[op]);
         break;
      case OP_IMM:
      case OP_UNK:
      default:
         if (assembler_format == ATT_SYNTAX) 
            snprintf(str, len, "$0x%X", op);
         else
            snprintf(str, len, "0x%X", op);
         break;
   }
   return(strlen(str));
}

int disassemble_address(char *buf, struct instr *i){
   struct code c = {0};
   int size;

   /* clear all 3 addr_exp's */
   memset(exp, 0, sizeof(struct addr_exp) * 3);
   memset(i, 0, sizeof(struct instr));
   size = disasm_addr((unsigned char *)buf, 0, &c, 0);
   /* copy the bastard 'native' code struct to a more general instr struct */
   strncpy(i->mnemonic, c.mnemonic, 16);
   sprint_op(i->dest, 32, c.dest, c.destType);
   sprint_op(i->src, 32, c.src, c.srcType);
   sprint_op(i->aux, 32, c.aux, c.auxType);
   i->mnemType = c.mnemType;
   i->destType = c.destType;
   i->srcType = c.srcType;
   i->auxType = c.auxType;
   i->size = size;
   return(size);
}

int sprint_address(char *str, int len, char *buf){
   struct instr i;
   int size;

   size = disassemble_address(buf, &i);
   snprintf(str, len, "%s\t%s", i.mnemonic, i.dest);
   if (i.src[0])
      snprintf(str, len - strlen(str), "%s, %s", str, i.src);
   if (i.aux[0])
      snprintf(str, len - strlen(str), "%s, %s", str, i.aux);
   return(size);
}

int AddRegTableEntry( int index, char *name, int size){
   if (index >= ext_arch.sz_regtable) return(0);
   ext_arch.reg_table[index].size = size;
   strncpy(ext_arch.reg_table[index].mnemonic, name, 8);
   return(1);
}

int addrexp_new(int scale,int index,int base,int disp,int flags){
   int id;
   if (!exp[0].used) 
      id = 0;
   else if (!exp[1].used)
      id = 1;
   else id = 2;
   exp[id].used = 1;

   exp[id].scale = scale;
   exp[id].index = index;
   exp[id].base = base;
   exp[id].disp = disp;
   exp[id].flags = flags;

   return(id);
}


int db_index_find(int a, void *b, void *c){
	cprintf("you shouldn't be using this outside of the bastard!!!\n");
	return(0);
}
int db_index_next(int a, void  *b){
	cprintf("you shouldn't be using this outside of the bastard!!!\n");
	return(0);
}

