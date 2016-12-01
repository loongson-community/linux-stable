#ifndef _ASM_SPINLOCK_TYPES_H
#define _ASM_SPINLOCK_TYPES_H

#ifndef __LINUX_SPINLOCK_TYPES_H
# error "please don't include this file directly"
#endif

#include <linux/types.h>

#include <asm/byteorder.h>

#ifndef CONFIG_CPU_LOONGSON3
typedef union {
	/*
	 * bits	 0..15 : serving_now
	 * bits 16..31 : ticket
	 */
	u32 lock;
	struct {
#ifdef __BIG_ENDIAN
		u16 ticket;
		u16 serving_now;
#else
		u16 serving_now;
		u16 ticket;
#endif
	} h;
} arch_spinlock_t;
#else
typedef union {
	/*
	 * bits	 0..15 : serving_now
	 * bits 16..31 : ticket
	 */
	u32 lock;
	struct {
#ifdef __BIG_ENDIAN
		u16 ticket;
		u16 serving_now;
#else
		u16 serving_now;
		u16 ticket;
#endif
		u32 padding;
	} h;
} __attribute__((aligned(8))) arch_spinlock_t;
#endif

#define __ARCH_SPIN_LOCK_UNLOCKED	{ .lock = 0 }

#ifndef CONFIG_CPU_LOONGSON3
typedef struct {
	volatile unsigned int lock;
} arch_rwlock_t;
#else
typedef struct {
	volatile unsigned int lock;
	volatile unsigned int padding;
} __attribute__((aligned(8))) arch_rwlock_t;
#endif

#define __ARCH_RW_LOCK_UNLOCKED		{ 0 }

#endif
