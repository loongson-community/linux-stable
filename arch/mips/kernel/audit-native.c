#include <linux/init.h>
#include <linux/types.h>
#include <linux/audit.h>
#include <asm/unistd.h>

static unsigned dir_class[] = {
#include <asm-generic/audit_dir_write.h>
~0U
};

static unsigned read_class[] = {
#include <asm-generic/audit_read.h>
~0U
};

static unsigned write_class[] = {
#include <asm-generic/audit_write.h>
~0U
};

static unsigned chattr_class[] = {
#include <asm-generic/audit_change_attr.h>
~0U
};

static unsigned signal_class[] = {
#include <asm-generic/audit_signal.h>
~0U
};


/*
 * Pretend to be a single architecture
 */
int audit_classify_arch(int arch)
{
	return 0;
}

extern int audit_classify_syscall_o32(int abi, unsigned syscall);
extern int audit_classify_syscall_n32(int abi, unsigned syscall);

int audit_classify_syscall(int abi, unsigned syscall)
{
	int res;

	switch (syscall) {
	case __NR_open:
		res = 2;
		break;

	case __NR_openat:
		res = 3;
		break;

#ifdef __NR_socketcall		/* Only exists on O32 */
	case __NR_socketcall:
		res = 4;
		break;
#endif
	case __NR_execve:
		res = 5;
		break;
	default:
#ifdef CONFIG_AUDITSYSCALL_O32
		res = audit_classify_syscall_o32(abi, syscall);
		if (res)
			break;
#endif
#ifdef CONFIG_AUDITSYSCALL_N32
		res = audit_classify_syscall_n32(abi, syscall);
		if (res)
			break;
#endif
		if (abi == AUDIT_ARCH_MIPS || abi == AUDIT_ARCH_MIPSEL)
			res = 1;
		else if (abi == AUDIT_ARCH_MIPS64 || abi == AUDIT_ARCH_MIPSEL64)
			res = 0;
		else if (abi == AUDIT_ARCH_MIPS64N32 || abi == AUDIT_ARCH_MIPSEL64N32)
			res = 6;
	}

	return res;
}

static int __init audit_classes_init(void)
{
	audit_register_class(AUDIT_CLASS_WRITE, write_class);
	audit_register_class(AUDIT_CLASS_READ, read_class);
	audit_register_class(AUDIT_CLASS_DIR_WRITE, dir_class);
	audit_register_class(AUDIT_CLASS_CHATTR, chattr_class);
	audit_register_class(AUDIT_CLASS_SIGNAL, signal_class);

	return 0;
}

__initcall(audit_classes_init);
