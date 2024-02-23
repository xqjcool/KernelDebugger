#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/uaccess.h>

#include "kcompat.h"

/* struct */
typedef struct _addrdata {
        uint64_t *addr;
        uint32_t len;
} addrdata_t;

/* micro */
#define ADDRDATA_NAME "addrdata"
#define PROC_WRITELEN 31

/* global */
static addrdata_t addrdata;
static struct proc_dir_entry *addrdata_pde;

static inline char printable(char sym)
{
	if (sym >= 0x20 && sym <= 0x7E)
		return sym;
	else
		return '.';
}

static void *addrdata_seq_start(struct seq_file *s, loff_t *pos)
{
	addrdata_t *addrdatap = s->private;

	if (addrdatap->addr == 0)
		return NULL;

	if (*pos >= addrdatap->len)
		return NULL;

	return pos;
}

static void *addrdata_seq_next(struct seq_file *s, void *v, loff_t *pos)
{
	addrdata_t *addrdatap = s->private;

	(*pos) += 2;

	if (*pos >= addrdatap->len)
		return NULL;

	return pos;
}

static int addrdata_seq_show(struct seq_file *s, void *v)
{
	loff_t offset = *((loff_t *)v);
	addrdata_t *addrdatap = s->private;
	uint64_t *addr = addrdatap->addr + offset;
	char *p = (char *)addr;
	int i;

	seq_printf(s, "%016llx:  %016llx", (uint64_t)addr, *addr);

	if (offset + 1 < addrdatap->len) {
		seq_printf(s, " %016llx   ", *(addr + 1));

		for (i = 0; i < 16; i++) {
			seq_printf(s, "%c", printable(p[i]));
		}
	} else {
		for (i = 0; i < 20; i++) {
			seq_putc(s, ' ');
		}
	
				
		for (i = 0; i < 8; i++) {
			seq_printf(s, "%c", printable(p[i]));
		}
	
	}
	seq_putc(s, '\n');

	return 0;
}

static void addrdata_seq_stop(struct seq_file *s, void *v)
{
}

static const struct seq_operations addrdata_seq_ops = {
	.start	= addrdata_seq_start,
	.next	= addrdata_seq_next,
	.show	= addrdata_seq_show,
	.stop	= addrdata_seq_stop,
};

static int addrdata_open(struct inode *inode, struct file *file)
{
	int ret = seq_open(file, &addrdata_seq_ops);
	
	if (!ret) {
		struct seq_file *sf = file->private_data;
		sf->private = pde_data(inode);
	}

	return ret;
}

static ssize_t addrdata_write(struct file *file, const char __user *input, size_t size, loff_t *ofs)
{
	char buffer[PROC_WRITELEN + 1] = {0};
	char *p = buffer; 
	addrdata_t *addrdatap = pde_data(file_inode(file));
	int rc;

	if (size > PROC_WRITELEN)
		return -EIO;

	if (copy_from_user(buffer, input, size))
		return -EFAULT;

	p = strchr(buffer, ' ');
	*p = '\0';
	p++;

	rc = kstrtoull(buffer, 16, (uint64_t *)&addrdatap->addr);
	if (rc < 0)
		return rc;

	rc = kstrtou32(p, 10, &addrdatap->len);
	if (rc < 0)
		return rc;

	pr_emerg("[%s:%d] addr:%p len:%u\n", __func__, __LINE__, addrdatap->addr, addrdatap->len);
	return size;
}

static struct proc_ops addrdata_proc_fops = {
	.proc_open	= addrdata_open,
	.proc_read	= seq_read,
	.proc_lseek	= seq_lseek,
	.proc_write	= addrdata_write,
	.proc_release	= seq_release,
};

int procfs_init(void)
{
        addrdata_pde = proc_create_data(ADDRDATA_NAME, 0666, NULL, &addrdata_proc_fops, &addrdata);
        if (addrdata_pde == NULL) {
                pr_err("failed to create addrdata proc entry\n");
                return -EFAULT;
        }

	return 0;
}

void procfs_exit(void)
{
	proc_remove(addrdata_pde);
}
