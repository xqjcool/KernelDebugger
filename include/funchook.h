/*
 * Copyright (c) 2020 
 *
 * funchook.h - function hook header
 * author: Xing Qingjie <xqjcool@gmail.com>
 * version: 1.0.0
 * history: 12/14/2020	created
 */
#ifndef _FUNCHOOK_H_
#define _FUNCHOOK_H_

#include <linux/kallsyms.h>
#include <linux/moduleparam.h>
#include <linux/printk.h>

typedef unsigned long (*kallsyms_lookup_name_t)(const char *name);

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

/*
 * call the original function in hook function
 * @_name_: name of the original function
 * @...: params to be passed
 */
#define CALL_ORIG_FUNCION(_name_, ...)   orig_##_name_(__VA_ARGS__)

/*
 * define the hook function and related definitions
 * @_name_: name of the original function
 * @_retype_: return value type of the original function
 * @...: params definition of the original function
 */
#define HOOK_DEFINE(_name_, _retype_, ...) \
	static _retype_ (*ptr_##_name_)(__VA_ARGS__); \
static _retype_ (*orig_##_name_)(__VA_ARGS__); \
static _retype_ stub_##_name_(__VA_ARGS__) \
{ \
	return (_retype_)0; \
} \
static _retype_ hook_##_name_(__VA_ARGS__) \

/*
 * register the hook function
 * @_name_: name of the original function
 */
#define HOOK_REGISTER(_name_) \
	hook_register(#_name_, (void **)&ptr_##_name_, (void **)&orig_##_name_, \
			(void *)hook_##_name_, (void *)stub_##_name_)

/*
 * unregister the hook function
 * @_name_: name of the original function
 */
#define HOOK_UNREGISTER(_name_) hook_unregister(ptr_##_name_, stub_##_name_)


int hook_register(const char *func_name, void **pptr_func, void **pptr_orig, 
		void *hook_func, void *stub_func);

void hook_unregister(void *ptr_func, void *stub_func);

#endif
