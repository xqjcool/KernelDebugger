/*
 * Copyright (c) 2020 
 *
 * funchook.c - function hook implementation
 * author: Xing Qingjie <xqjcool@gmail.com>
 * version: 1.0.0
 * history: 12/14/2020	created
 */
#include <linux/cpu.h>
#include <linux/delay.h>
#include <linux/printk.h>

#include "funchook.h"

#define INSTRSIZE	5

typedef void (*smp_text_poke_single_t)(void *addr, const void *opcode, size_t len,
				       const void *emulate);

static char default_instr[INSTRSIZE] = {0x0f, 0x1f, 0x44, 0x00, 0x00};
static kallsyms_lookup_name_t ptr_kallsyms_lookup_name;
static smp_text_poke_single_t ptr_smp_text_poke_single;

int hook_register(const char *func_name, void **pptr_func, void **pptr_orig, 
		void *hook_func, void *stub_func)
{
	int hook_offset;
	int orig_offset;
	char jump_to_orig[INSTRSIZE] = {0xe9, 0, 0, 0, 0};
	char jump_to_hook[INSTRSIZE] = {0xe9, 0, 0, 0, 0};
	if (!ptr_kallsyms_lookup_name) {
		ptr_kallsyms_lookup_name = (kallsyms_lookup_name_t)kallsyms_lookup_name_func;
	}

	if (!ptr_smp_text_poke_single) {
		ptr_smp_text_poke_single =
			(smp_text_poke_single_t)ptr_kallsyms_lookup_name("smp_text_poke_single");
		if (!ptr_smp_text_poke_single) {
			return -EPERM;
		}
	}

	*pptr_func = (void *)ptr_kallsyms_lookup_name(func_name);
	if (!*pptr_func) {
		return -ENOENT;
	}
	pr_info("hook_register: %s=%px hook=%px stub=%px\n", func_name, *pptr_func, hook_func, stub_func);

	hook_offset = (int)((long)hook_func - ((long)*pptr_func + INSTRSIZE));
	(*(int *)(&jump_to_hook[1])) = hook_offset;

	orig_offset = (int)((long)*pptr_func - (long)stub_func);
	(*(int *)(&jump_to_orig[1])) = orig_offset;	

	cpus_read_lock();
	ptr_smp_text_poke_single(stub_func, jump_to_orig, INSTRSIZE, NULL);
	*pptr_orig = stub_func;
	barrier();
	ptr_smp_text_poke_single(*pptr_func, jump_to_hook, INSTRSIZE, NULL);
	cpus_read_unlock();
	return 0;
}

void hook_unregister(void *ptr_func, void *stub_func)
{
	cpus_read_lock();
	ptr_smp_text_poke_single(ptr_func, default_instr, INSTRSIZE, NULL);
	ptr_smp_text_poke_single(stub_func, default_instr, INSTRSIZE, NULL);
	barrier();
	cpus_read_unlock();	
}
