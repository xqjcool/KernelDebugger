#ifndef _KCOMPAT_H_
#define _KCOMPAT_H_

#ifndef LINUX_VERSION_CODE
#include <linux/version.h>
#else
#define KERNEL_VERSION(a,b,c) (((a) << 16) + ((b) << 8) + (c))
#endif

typedef unsigned long (*kallsyms_lookup_name_t)(const char *name);

#if (LINUX_VERSION_CODE > KERNEL_VERSION(5,7,0))
#define KALLSYMS_MACRO() \
       unsigned long lookup_func_addr; \
       module_param(lookup_func_addr, ulong, 0444); \
       MODULE_PARM_DESC(lookup_func_addr, "The function address of kallsyms_lookup_name");
extern unsigned long lookup_func_addr;
#define kallsyms_lookup_name lookup_func_addr

#define POKE_MAX_OPCODE_SIZE 5
struct text_poke_loc {
        /* addr := _stext + rel_addr */
        s32 rel_addr;
        s32 disp;
        u8 len;
        u8 opcode;
        const u8 text[POKE_MAX_OPCODE_SIZE];
        /* see text_poke_bp_batch() */
        u8 old;
};

static inline void text_poke_loc_init(struct text_poke_loc *tp, void *addr,
		const void *opcode, size_t len, const void *emulate)
{
	kallsyms_lookup_name_t ptr_lookup_func_addr = (kallsyms_lookup_name_t)lookup_func_addr;
        memcpy((void *)tp->text, opcode, len);

        tp->rel_addr = addr - (void *)ptr_lookup_func_addr("_stext");
        tp->len = len;
        tp->opcode = *(u8 *)opcode;
	tp->disp = *(s32 *)(opcode + 1);
}
#else
#include <asm/text-patching.h>
#define KALLSYMS_MACRO() 
static inline void text_poke_loc_init(struct text_poke_loc *tp, void *addr,
		const void *opcode, size_t len, const void *emulate)
{
        tp->addr = addr;
        tp->len = len;
	tp->detour = emulate;
        memcpy((void *)tp->opcode, opcode, len);
}
#endif

#ifdef CONFIG_PROC_FS

#if (LINUX_VERSION_CODE < KERNEL_VERSION(5,17,0))

#define pde_data	PDE_DATA
#define proc_ops	file_operations
#define proc_open	open
#define proc_read	read
#define proc_write	write
#define proc_lseek	llseek
#define proc_release	release

#endif

#endif

#endif
