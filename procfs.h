#ifndef _PROCFS_H_
#define _PROCFS_H_

#ifdef CONFIG_PROC_FS
int procfs_init(void);
void procfs_exit(void);
#else
static inline int procfs_init(void)
{
	return 0;
}
void procfs_exit(void)
{
}
#endif

#endif
