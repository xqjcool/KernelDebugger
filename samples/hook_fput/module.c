#include <linux/module.h>
#include <funchook.h>
#include "procfs.h"
#include "kcompat.h"

/* heades needed by patched function */
#include <net/sock.h>

/* module param defines */

/*
 * Need the mod_param lookup_func_addr for Linux-5.7 and later version
 * For example: "insmod sample.ko lookup_func_addr=0xffffffff803782c0"
 */
KALLSYMS_MODPARAM_DEF(); 

/*
 * 1.use HOOK_DEFINE macro to define the hook function.
 *  1.1 if you need to call the original function in the hook function,
 *      you need to use CALL_ORIG_FUNCION macro.
 *  1.2 if you need to call other kernel exported function, just include the
 *      related header(s).
 *  1.3 if you need to call other kernel static function, you need to make a
 *      duplicate copy here.
 */
HOOK_DEFINE(fput, void, struct file *file)
{
	pr_info("%s: BEFORE file=%p\n", __func__, file);

	CALL_ORIG_FUNCION(fput, file);

	pr_info("%s: AFTER file=%p\n", __func__, file);
}

static __init int funchook_init(void)
{
	int ret = 0;

	ret = kallsyms_modparam_check();
	if (ret != 0) {
		pr_info("%s\n", KALLSYMS_MODPARAM_USAGE);
		return ret;
	}

	ret = procfs_init();
	if (ret != 0) {
		pr_info("procfs_init failed ret=%d\n", ret);
		return ret;
	}

	/*
	 * 2. use HOOK_REGISTER macro to replace original function with hook function
	 */
	ret = HOOK_REGISTER(fput);
	if (ret != 0)
	{
		pr_info("fput failed ret=%d\n", ret);
		goto CommonReturn;
	}
CommonReturn:
	if (ret) {
		procfs_exit();
	}

	return ret;
}
module_init(funchook_init);

static __exit void funchook_exit(void)
{
	/*
	 * 3. use HOOK_UNREGISTER macro to restore original function.
	 */
	HOOK_UNREGISTER(fput);

	procfs_exit();
}
module_exit(funchook_exit);

MODULE_DESCRIPTION("hook sample");
MODULE_LICENSE("GPL");
MODULE_VERSION("1.1");
