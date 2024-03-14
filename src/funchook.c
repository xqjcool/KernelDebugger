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
#include <linux/delay.h>

#include "kcompat.h"

#define INSTRSIZE	5

typedef void *(*text_poke_bp_batch_t)(struct text_poke_loc *tp, unsigned int nr_entries);

static char default_instr[INSTRSIZE] = {0x0f, 0x1f, 0x44, 0x00, 0x00};
static kallsyms_lookup_name_t ptr_kallsyms_lookup_name;
static text_poke_bp_batch_t ptr_text_poke_bp_batch;

int hook_register(const char *func_name, void **pptr_func, void **pptr_orig, 
		void *hook_func, void *stub_func)
{
	int hook_offset;
	int orig_offset;
	char jump_to_orig[INSTRSIZE] = {0xe9, 0, 0, 0, 0};
	char jump_to_hook[INSTRSIZE] = {0xe9, 0, 0, 0, 0};
	struct text_poke_loc tp_orig, tp_stub;

	if (!ptr_kallsyms_lookup_name)
	{
		ptr_kallsyms_lookup_name = (kallsyms_lookup_name_t)kallsyms_lookup_name_func;
	}

	if (!ptr_text_poke_bp_batch)
	{
		ptr_text_poke_bp_batch = (text_poke_bp_batch_t)ptr_kallsyms_lookup_name("text_poke_bp_batch");
		if (!ptr_text_poke_bp_batch) {
			return -EPERM;
		}
	}

	*pptr_func = (void *)ptr_kallsyms_lookup_name(func_name);
	if (!*pptr_func) {
		return -ENOENT;
	}

	hook_offset = (int)((long)hook_func - ((long)*pptr_func + INSTRSIZE));
	(*(int *)(&jump_to_hook[1])) = hook_offset;

	orig_offset = (int)((long)*pptr_func - (long)stub_func);
	(*(int *)(&jump_to_orig[1])) = orig_offset;	

	text_poke_loc_init(&tp_stub, stub_func, jump_to_orig, INSTRSIZE, (void *)stub_func + INSTRSIZE);
	text_poke_loc_init(&tp_orig, *pptr_func, jump_to_hook, INSTRSIZE, *pptr_func + INSTRSIZE);

	cpus_read_lock();

	ptr_text_poke_bp_batch(&tp_stub, 1);
	*pptr_orig = stub_func;

	barrier();

	ptr_text_poke_bp_batch(&tp_orig, 1);

	cpus_read_unlock();

	return 0;
}

void hook_unregister(void *ptr_func, void *stub_func)
{
	struct text_poke_loc tp_orig, tp_stub;

	text_poke_loc_init(&tp_orig, ptr_func, default_instr, INSTRSIZE, (void *)ptr_func + INSTRSIZE);
	text_poke_loc_init(&tp_stub, stub_func, default_instr, INSTRSIZE, (void *)stub_func + INSTRSIZE);

	cpus_read_lock();
	ptr_text_poke_bp_batch(&tp_orig, 1);
	ptr_text_poke_bp_batch(&tp_stub, 1);
	barrier();
	cpus_read_unlock();	
}

