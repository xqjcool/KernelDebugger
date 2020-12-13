/*
 * Copyright (c) 2020 
 *
 * funchook.c - function hook implementation
 * author: Xing Qingjie <xqjcool@gmail.com>
 * version: 1.0.0
 * history: 12/14/2020	created
 */
#include <linux/cpu.h>
#include <linux/kallsyms.h>

#define INSTRSIZE	5

typedef void *(*text_poke_bp_t)(void *addr, const void *opcode, size_t len,
		void *handler);


static char default_instr[INSTRSIZE] = {0x0f, 0x1f, 0x44, 0x00, 0x00};
static text_poke_bp_t ptr_text_poke_bp;

int hook_register(const char *func_name, void **pptr_func, void **pptr_orig, 
		void *hook_func, void *stub_func)
{
	int hook_offset;
	int orig_offset;
	char jump_to_orig[INSTRSIZE] = {0xe9, 0, 0, 0, 0};
	char jump_to_hook[INSTRSIZE] = {0xe9, 0, 0, 0, 0};

	if (!ptr_text_poke_bp)
	{
		ptr_text_poke_bp = (text_poke_bp_t)kallsyms_lookup_name("text_poke_bp");
		if (!ptr_text_poke_bp) {
			return -EPERM;
		}
	}

	*pptr_func = (void *)kallsyms_lookup_name(func_name);
	if (!*pptr_func) {
		return -ENOENT;
	}

	hook_offset = (int)((long)hook_func - ((long)*pptr_func + INSTRSIZE));
	(*(int *)(&jump_to_hook[1])) = hook_offset;

	orig_offset = (int)((long)*pptr_func - (long)stub_func);
	(*(int *)(&jump_to_orig[1])) = orig_offset;	

	get_online_cpus();

	ptr_text_poke_bp(stub_func, jump_to_orig, INSTRSIZE, (void *)stub_func + INSTRSIZE);
	*pptr_orig = stub_func;

	barrier();

	ptr_text_poke_bp(*pptr_func, jump_to_hook, INSTRSIZE, *pptr_func + INSTRSIZE);

	put_online_cpus();

	return 0;
}

void hook_unregister(void *ptr_func, void *stub_func)
{
	get_online_cpus();
	ptr_text_poke_bp(ptr_func, default_instr, INSTRSIZE, (void *)ptr_func + INSTRSIZE);
	ptr_text_poke_bp(stub_func, default_instr, INSTRSIZE, (void *)stub_func + INSTRSIZE);
	barrier();
	put_online_cpus();	
}

