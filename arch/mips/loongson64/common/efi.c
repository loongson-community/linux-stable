#include <linux/fs.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kobject.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>

static struct kobject *efi_kobj;
struct proc_dir_entry *proc_efi;
extern unsigned int has_systab;
extern unsigned long systab_addr;

static int show_systab(struct seq_file *m, void *v)
{
	seq_printf(m, "SMBIOS=0x%lx\n", systab_addr);

	return 0;
}

static void *systab_start(struct seq_file *m, loff_t *pos)
{
	unsigned long i = *pos;

	return i ? NULL : (void *)0x1;
}

static void *systab_next(struct seq_file *m, void *v, loff_t *pos)
{
	++*pos;

	return systab_start(m, pos);
}

static void systab_stop(struct seq_file *m, void *v)
{
}

const struct seq_operations systab_op = {
	.start	= systab_start,
	.next	= systab_next,
	.stop	= systab_stop,
	.show	= show_systab,
};


static int systab_open(struct inode *inode, struct file *file)

{
	return seq_open(file, &systab_op);
}

static const struct file_operations proc_systab_operations = {
	.open		= systab_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= seq_release,
};

/**
  * efi_init_kernfs - Create efi in procfs/sysfs
  */
int __init efi_init_kernfs(void)
{
	efi_kobj = kobject_create_and_add("efi", firmware_kobj);
	if (!efi_kobj)
		return -ENOMEM;

	proc_efi = proc_mkdir("efi", NULL);
	if (!proc_efi)
		return -ENOMEM;

	if (!proc_create("systab", 0, proc_efi, &proc_systab_operations))
		return -ENOMEM;

	return 0;
}

/**
  * efi_exit_kernfs - Remove efi from procfs/sysfs
  */
void __exit efi_exit_kernfs(void)
{
	remove_proc_entry("systab", proc_efi);
	remove_proc_entry("efi", NULL);
	kobject_put(efi_kobj);
}

static int __init init_efi(void)
{
	if (!has_systab)
		return 0;
	else
		return efi_init_kernfs();
}

static void __exit exit_efi(void)
{
	if (!has_systab)
		return;
	else
		efi_exit_kernfs();
}

subsys_initcall(init_efi);
module_exit(exit_efi);
