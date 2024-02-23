#ifndef _KCOMPAT_H_
#define _KCOMPAT_H_

#ifndef LINUX_VERSION_CODE
#include <linux/version.h>
#else
#define KERNEL_VERSION(a,b,c) (((a) << 16) + ((b) << 8) + (c))
#endif

#ifdef CONFIG_PROC_FS

#if ( LINUX_VERSION_CODE < KERNEL_VERSION(5,17,0) )

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
