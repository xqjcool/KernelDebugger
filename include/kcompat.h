#ifndef _KCOMPAT_H_
#define _KCOMPAT_H_

#ifndef LINUX_VERSION_CODE
#include <linux/version.h>
#else
#define KERNEL_VERSION(a,b,c) (((a) << 16) + ((b) << 8) + (c))
#endif

typedef unsigned long (*kallsyms_lookup_name_t)(const char *name);

#if (LINUX_VERSION_CODE > KERNEL_VERSION(5,7,0))
#define KALLSYMS_MODPARAM_USAGE "Usage: insmod hook_fput.ko lookup_func_addr=0xffffffff80408080" 
#define KALLSYMS_MODPARAM_DEF() \
       unsigned long lookup_func_addr = 0; \
       module_param(lookup_func_addr, ulong, 0444); \
       MODULE_PARM_DESC(lookup_func_addr, "The function address of kallsyms_lookup_name");

extern unsigned long lookup_func_addr;
#define kallsyms_lookup_name_func lookup_func_addr

static inline int kallsyms_modparam_check(void)
{
	return (lookup_func_addr == 0) ? -1 : 0;
}

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
		const void *opcode, size_t len, void *emulate)
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
#define KALLSYMS_MODPARAM_USAGE ""
#define KALLSYMS_MODPARAM_DEF() 
#define kallsyms_modparam_check() 0
#define kallsyms_lookup_name_func kallsyms_lookup_name
static inline void text_poke_loc_init(struct text_poke_loc *tp, void *addr,
		const void *opcode, size_t len, void *emulate)
{
        tp->addr = addr;
        tp->len = len;
	tp->detour = emulate;
        memcpy((void *)tp->opcode, opcode, len);
}
#endif

#ifdef CONFIG_PROC_FS

#define pde_data	PDE_DATA

#if (LINUX_VERSION_CODE < KERNEL_VERSION(5,17,0))

#define proc_ops	file_operations
#define proc_open	open
#define proc_read	read
#define proc_write	write
#define proc_lseek	llseek
#define proc_release	release

#endif

#endif

#endif
