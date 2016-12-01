/*
 * EC(Embedded Controller) WPCE775L misc device driver on Linux
 * Author	: Huang Wei <huangwei@lemote.com>
 * Date		: 2010-06-28
 *
 * NOTE :
 * 		1, The EC resources accessing and programming are supported.
 */

/*******************************************************************/

#include <linux/module.h>
#include <linux/poll.h>
#include <linux/slab.h>
#include <linux/proc_fs.h>
#include <linux/miscdevice.h>
#include <linux/apm_bios.h>
#include <linux/capability.h>
#include <linux/sched.h>
#include <linux/pm.h>
#include <linux/apm-emulation.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/init.h>
#include <linux/completion.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/timer.h>

#include <asm/delay.h>
#include <asm/io.h>	/* for ioremap(offset, size) */

#include "ec_wpce775l.h"

/*******************************************************************/

/* the register operation access struct */
struct ec_reg {
	u32	addr;	/* the address is EC flash address and ACPI command */
	u8	index;	/* the index is ACPI command's index */
	u16	val;	/* the val is EC flash data and EC space value */
	u8	flag;	/* Different access methods. */
};

const char *version = EC_VERSION;

/* Ec misc device name */
#define	EC_MISC_DEV		"ec_misc"

/* Ec misc device minor number */
#define	ECMISC_MINOR_DEV	MISC_DYNAMIC_MINOR

#define	EC_IOC_MAGIC		'E'

/* misc ioctl operations */
#define	IOCTL_RDREG		_IOR(EC_IOC_MAGIC, 1, int)
#define	IOCTL_WRREG		_IOW(EC_IOC_MAGIC, 2, int)

/* ioctl */
static int misc_ioctl(struct inode * inode, struct file *filp, u_int cmd, u_long arg)
{
	void __user *ptr = (void __user *)arg;
	struct ec_reg *ecreg = (struct ec_reg *)(filp->private_data);
	int ret = 0;

	switch (cmd) {
		case IOCTL_RDREG:
			ret = copy_from_user(ecreg, ptr, sizeof(struct ec_reg));
			if (ret) {
				printk(KERN_ERR "ACPI command read : copy from user error.\n");
				return -EFAULT;
			}

			if (ecreg->flag == 0) {
				/* has index read ACPI command */
				ecreg->val = (unsigned short)ec_read_all((unsigned char)ecreg->addr, ecreg->index);
			} else if (ecreg->flag == 1) {
				/* no index read ACPI command */
				ecreg->val = (unsigned short)ec_read_noindex((unsigned char)ecreg->addr);
			}
			ret = copy_to_user(ptr, ecreg, sizeof(struct ec_reg));
			if (ret) {
				printk(KERN_ERR "ACPI command read : copy to user error.\n");
				return -EFAULT;
			}
			break;
		case IOCTL_WRREG:
			ret = copy_from_user(ecreg, ptr, sizeof(struct ec_reg));
			if (ret) {
				printk(KERN_ERR "WCB and ACPI command write : copy from user error.\n");
				return -EFAULT;
			}

			if (ecreg->flag == 1) {
				/* write no index command */
				ec_write_noindex((unsigned char)ecreg->addr, (unsigned char)ecreg->val);
			} else {
				/* write has index command */
				ec_write_all((unsigned char)ecreg->addr, (unsigned char)ecreg->index, (unsigned char)ecreg->val);
			}
			break;

		default :
			break;
	}

	return 0;
}

static long misc_compat_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	return misc_ioctl(file->f_path.dentry->d_inode, file, cmd, arg);
}

static int misc_open(struct inode * inode, struct file * filp)
{
	struct ec_reg *ecreg = NULL;
	ecreg = kmalloc(sizeof(struct ec_reg), GFP_KERNEL);
	if (ecreg) {
		filp->private_data = ecreg;
	}

	return ecreg ? 0 : -ENOMEM;
}

static int misc_release(struct inode * inode, struct file * filp)
{
	struct ec_reg *ecreg = (struct ec_reg *)(filp->private_data);

	filp->private_data = NULL;
	kfree(ecreg);

	return 0;
}

static struct file_operations ecmisc_fops = {
	.owner		= THIS_MODULE,
	.open		= misc_open,
	.release	= misc_release,
	.read		= NULL,
	.write		= NULL,
#ifdef	CONFIG_64BIT
	.compat_ioctl	= misc_compat_ioctl,
#else
	.ioctl		= misc_ioctl,
#endif
};

/*********************************************************/

static struct miscdevice ecmisc_device = {
	.minor		= ECMISC_MINOR_DEV,
	.name		= EC_MISC_DEV,
	.fops		= &ecmisc_fops
};

static int __init ecmisc_init(void)
{
	int ret;

	ret = misc_register(&ecmisc_device);
	printk(KERN_INFO "EC misc device init V%s.\n", version);

	return ret;
}

static void __exit ecmisc_exit(void)
{
	misc_deregister(&ecmisc_device);
	printk(KERN_INFO "EC misc device exit.\n");
}

module_init(ecmisc_init);
module_exit(ecmisc_exit);

MODULE_AUTHOR("Huang Wei <huangw@lemote.com>");
MODULE_DESCRIPTION("WPCE775L resources misc Management");
MODULE_LICENSE("GPL");
