/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 2008 David Daney
 */

#include <linux/sched.h>

#include <asm/processor.h>
#include <asm/watch.h>

#define DRSEG_BASE	(0xffffffffff300000ul)

static inline void mips_ejtag_watch_trigger(unsigned long cmd, void *arg)
{
	register unsigned long _cmd asm("$5") = cmd;
	register unsigned long _arg asm("$6") = (unsigned long) arg;

	asm volatile (
		"sdbbp\n\t"
		::"r"(_cmd), "r"(_arg)
		:"memory"
	);
}

/*
 * Install the watch registers for the current thread.	A maximum of
 * four registers are installed although the machine may have more.
 */
void mips_install_watch_registers(struct task_struct *t)
{
	struct mips3264_watch_reg_state *watches = &t->thread.watch.mips3264;

	if (current_cpu_data.ejtag_watch) {
		mips_ejtag_watch_trigger(3, t);
		return;
	}

	switch (current_cpu_data.watch_reg_use_cnt) {
	default:
		BUG();
	case 4:
		write_c0_watchlo3(watches->watchlo[3]);
		/* Write 1 to the I, R, and W bits to clear them, and
		   1 to G so all ASIDs are trapped. */
		write_c0_watchhi3(MIPS_WATCHHI_G | MIPS_WATCHHI_IRW |
				  watches->watchhi[3]);
	case 3:
		write_c0_watchlo2(watches->watchlo[2]);
		write_c0_watchhi2(MIPS_WATCHHI_G | MIPS_WATCHHI_IRW |
				  watches->watchhi[2]);
	case 2:
		write_c0_watchlo1(watches->watchlo[1]);
		write_c0_watchhi1(MIPS_WATCHHI_G | MIPS_WATCHHI_IRW |
				  watches->watchhi[1]);
	case 1:
		write_c0_watchlo0(watches->watchlo[0]);
		write_c0_watchhi0(MIPS_WATCHHI_G | MIPS_WATCHHI_IRW |
				  watches->watchhi[0]);
	}
}

/*
 * Read back the watchhi registers so the user space debugger has
 * access to the I, R, and W bits.  A maximum of four registers are
 * read although the machine may have more.
 */
void mips_read_watch_registers(void)
{
	struct mips3264_watch_reg_state *watches =
		&current->thread.watch.mips3264;

	if (current_cpu_data.ejtag_watch) {
		mips_ejtag_watch_trigger(2, NULL);
		return;
	}

	switch (current_cpu_data.watch_reg_use_cnt) {
	default:
		BUG();
	case 4:
		watches->watchhi[3] = (read_c0_watchhi3() &
				       (MIPS_WATCHHI_MASK | MIPS_WATCHHI_IRW));
	case 3:
		watches->watchhi[2] = (read_c0_watchhi2() &
				       (MIPS_WATCHHI_MASK | MIPS_WATCHHI_IRW));
	case 2:
		watches->watchhi[1] = (read_c0_watchhi1() &
				       (MIPS_WATCHHI_MASK | MIPS_WATCHHI_IRW));
	case 1:
		watches->watchhi[0] = (read_c0_watchhi0() &
				       (MIPS_WATCHHI_MASK | MIPS_WATCHHI_IRW));
	}
	if (current_cpu_data.watch_reg_use_cnt == 1 &&
	    (watches->watchhi[0] & MIPS_WATCHHI_IRW) == 0) {
		/* Pathological case of release 1 architecture that
		 * doesn't set the condition bits.  We assume that
		 * since we got here, the watch condition was met and
		 * signal that the conditions requested in watchlo
		 * were met.  */
		watches->watchhi[0] |= (watches->watchlo[0] & MIPS_WATCHHI_IRW);
	}
 }

/*
 * Disable all watch registers.	 Although only four registers are
 * installed, all are cleared to eliminate the possibility of endless
 * looping in the watch handler.
 */
void mips_clear_watch_registers(void)
{
	if (current_cpu_data.ejtag_watch) {
		mips_ejtag_watch_trigger(4, NULL);
		return;
	}

	switch (current_cpu_data.watch_reg_count) {
	default:
		BUG();
	case 8:
		write_c0_watchlo7(0);
	case 7:
		write_c0_watchlo6(0);
	case 6:
		write_c0_watchlo5(0);
	case 5:
		write_c0_watchlo4(0);
	case 4:
		write_c0_watchlo3(0);
	case 3:
		write_c0_watchlo2(0);
	case 2:
		write_c0_watchlo1(0);
	case 1:
		write_c0_watchlo0(0);
	}
}

void mips_probe_watch_registers(struct cpuinfo_mips *c)
{
	unsigned int t;

	if ((c->options & MIPS_CPU_WATCH) == 0)
		return;
	c->ejtag_watch = 0;

	/*
	 * Check which of the I,R and W bits are supported, then
	 * disable the register.
	 */
	write_c0_watchlo0(MIPS_WATCHLO_IRW);
	back_to_back_c0_hazard();
	t = read_c0_watchlo0();
	write_c0_watchlo0(0);
	c->watch_reg_masks[0] = t & MIPS_WATCHLO_IRW;

	/* Write the mask bits and read them back to determine which
	 * can be used. */
	c->watch_reg_count = 1;
	c->watch_reg_use_cnt = 1;
	t = read_c0_watchhi0();
	write_c0_watchhi0(t | MIPS_WATCHHI_MASK);
	back_to_back_c0_hazard();
	t = read_c0_watchhi0();
	c->watch_reg_masks[0] |= (t & MIPS_WATCHHI_MASK);
	if ((t & MIPS_WATCHHI_M) == 0)
		return;

	write_c0_watchlo1(MIPS_WATCHLO_IRW);
	back_to_back_c0_hazard();
	t = read_c0_watchlo1();
	write_c0_watchlo1(0);
	c->watch_reg_masks[1] = t & MIPS_WATCHLO_IRW;

	c->watch_reg_count = 2;
	c->watch_reg_use_cnt = 2;
	t = read_c0_watchhi1();
	write_c0_watchhi1(t | MIPS_WATCHHI_MASK);
	back_to_back_c0_hazard();
	t = read_c0_watchhi1();
	c->watch_reg_masks[1] |= (t & MIPS_WATCHHI_MASK);
	if ((t & MIPS_WATCHHI_M) == 0)
		return;

	write_c0_watchlo2(MIPS_WATCHLO_IRW);
	back_to_back_c0_hazard();
	t = read_c0_watchlo2();
	write_c0_watchlo2(0);
	c->watch_reg_masks[2] = t & MIPS_WATCHLO_IRW;

	c->watch_reg_count = 3;
	c->watch_reg_use_cnt = 3;
	t = read_c0_watchhi2();
	write_c0_watchhi2(t | MIPS_WATCHHI_MASK);
	back_to_back_c0_hazard();
	t = read_c0_watchhi2();
	c->watch_reg_masks[2] |= (t & MIPS_WATCHHI_MASK);
	if ((t & MIPS_WATCHHI_M) == 0)
		return;

	write_c0_watchlo3(MIPS_WATCHLO_IRW);
	back_to_back_c0_hazard();
	t = read_c0_watchlo3();
	write_c0_watchlo3(0);
	c->watch_reg_masks[3] = t & MIPS_WATCHLO_IRW;

	c->watch_reg_count = 4;
	c->watch_reg_use_cnt = 4;
	t = read_c0_watchhi3();
	write_c0_watchhi3(t | MIPS_WATCHHI_MASK);
	back_to_back_c0_hazard();
	t = read_c0_watchhi3();
	c->watch_reg_masks[3] |= (t & MIPS_WATCHHI_MASK);
	if ((t & MIPS_WATCHHI_M) == 0)
		return;

	/* We use at most 4, but probe and report up to 8. */
	c->watch_reg_count = 5;
	t = read_c0_watchhi4();
	if ((t & MIPS_WATCHHI_M) == 0)
		return;

	c->watch_reg_count = 6;
	t = read_c0_watchhi5();
	if ((t & MIPS_WATCHHI_M) == 0)
		return;

	c->watch_reg_count = 7;
	t = read_c0_watchhi6();
	if ((t & MIPS_WATCHHI_M) == 0)
		return;

	c->watch_reg_count = 8;
}

static inline unsigned long read_ibs (void)
{
	return *(volatile unsigned long *)(DRSEG_BASE + 0x1000ul);
}

static inline void write_ibs (unsigned long v)
{
	*(volatile unsigned long *)(DRSEG_BASE + 0x1000ul) = v;
}

static inline unsigned long read_iba (unsigned int i)
{
	return *(volatile unsigned long *)(DRSEG_BASE + 0x1100ul + 0x100 * i);
}

static inline void write_iba (unsigned int i, unsigned long v)
{
	*(volatile unsigned long *)(DRSEG_BASE + 0x1100ul + 0x100 * i) = v;
}

static inline unsigned long read_ibm (unsigned int i)
{
	return *(volatile unsigned long *)(DRSEG_BASE + 0x1108ul + 0x100 * i);
}

static inline void write_ibm (unsigned int i, unsigned long v)
{
	*(volatile unsigned long *)(DRSEG_BASE + 0x1108ul + 0x100 * i) = v;
}

static inline unsigned long read_ibc (unsigned int i)
{
	return *(volatile unsigned long *)(DRSEG_BASE + 0x1118ul + 0x100 * i);
}

static inline void write_ibc (unsigned int i, unsigned long v)
{
	*(volatile unsigned long *)(DRSEG_BASE + 0x1118ul + 0x100 * i) = v;
}

static inline unsigned long read_dbs (void)
{
	return *(volatile unsigned long *)(DRSEG_BASE + 0x2000ul);
}

static inline void write_dbs (unsigned long v)
{
	*(volatile unsigned long *)(DRSEG_BASE + 0x2000ul) = v;
}

static inline unsigned long read_dba (unsigned int i)
{
	return *(volatile unsigned long *)(DRSEG_BASE + 0x2100ul + 0x100 * i);
}

static inline void write_dba (unsigned int i, unsigned long v)
{
	*(volatile unsigned long *)(DRSEG_BASE + 0x2100ul + 0x100 * i) = v;
}

static inline unsigned long read_dbm (unsigned int i)
{
	return *(volatile unsigned long *)(DRSEG_BASE + 0x2108ul + 0x100 * i);
}

static inline void write_dbm (unsigned int i, unsigned long v)
{
	*(volatile unsigned long *)(DRSEG_BASE + 0x2108ul + 0x100 * i) = v;
}

static inline unsigned long read_dbc (unsigned int i)
{
	return *(volatile unsigned long *)(DRSEG_BASE + 0x2118ul + 0x100 * i);
}

static inline void write_dbc (unsigned int i, unsigned long v)
{
	*(volatile unsigned long *)(DRSEG_BASE + 0x2118ul + 0x100 * i) = v;
}

void mips_install_ejtag_watch_registers(struct task_struct *t)
{
	struct mips3264_watch_reg_state *watches = &t->thread.watch.mips3264;
	unsigned int i;

	write_ibs(read_ibs() & ~0x7ffful);
	write_dbs(read_dbs() & ~0x7ffful);

	for (i = 0; i < current_cpu_data.watch_reg_use_cnt; i++) {
		if (watches->watchlo[i] & MIPS_WATCHHI_I) {
			write_iba(i, watches->watchlo[i] & ~MIPS_WATCHHI_IRW);
			write_ibm(i, watches->watchhi[i] & MIPS_WATCHHI_MASK);
			write_ibc(i, 1);
		}

		if (watches->watchlo[i] & (MIPS_WATCHHI_R | MIPS_WATCHHI_W)) {
			unsigned int dbc = 0x3c3ff1u;

			write_dba(i, watches->watchlo[i] & ~MIPS_WATCHHI_IRW);
			write_dbm(i, watches->watchhi[i] & MIPS_WATCHHI_MASK);
			if (watches->watchlo[i] & MIPS_WATCHHI_R)
				dbc &= ~0x1000u;
			if (watches->watchlo[i] & MIPS_WATCHHI_W)
				dbc &= ~0x2000u;
			write_dbc(i, dbc);
		}
	}
}

void mips_read_ejtag_watch_registers(void)
{
	struct mips3264_watch_reg_state *watches =
		&current->thread.watch.mips3264;
	unsigned int i;

	for (i = 0; i < current_cpu_data.watch_reg_use_cnt; i++) {
		if (watches->watchlo[i] & MIPS_WATCHHI_I) {
			watches->watchhi[i] = read_ibm(i);
			if (read_ibs() & (1u << i))
				watches->watchhi[i] |= MIPS_WATCHHI_I;
		}

		if (watches->watchlo[i] & (MIPS_WATCHHI_R | MIPS_WATCHHI_W)) {
			watches->watchhi[i] |= read_dbm(i);
			if (read_dbs() & (1u << i)) {
				watches->watchhi[i] |= MIPS_WATCHHI_R | MIPS_WATCHHI_W;
				if (read_dbc(i) & 0x1000u)
					watches->watchhi[i] &= ~MIPS_WATCHHI_R;
				if (read_dbc(i) & 0x2000u)
					watches->watchhi[i] &= ~MIPS_WATCHHI_W;
			}
		}
	}
}

void mips_clear_ejtag_watch_registers(void)
{
	unsigned int i;

	for (i = 0; i < current_cpu_data.watch_reg_count; i++) {
		write_ibs(read_ibs() & ~0x7ffful);
		write_iba(i, 0);
		write_ibm(i, 0);
		write_ibc(i, 0);

		write_dbs(read_dbs() & ~0x7ffful);
		write_dba(i, 0);
		write_dbm(i, 0);
		write_dbc(i, 0);
	}
}

void mips_probe_ejtag_watch_registers_trigger(struct cpuinfo_mips *c)
{
	if ((c->options & MIPS_CPU_WATCH) != 0)
		return;

	if (!cpu_has_ejtag)
		return;

	c->ejtag_watch = 1;
	mips_ejtag_watch_trigger(1, c);
}

void mips_probe_ejtag_watch_registers(struct cpuinfo_mips *c)
{
	unsigned int ibcn = (read_ibs() >> 24) & 0xf;
	unsigned int dbcn = (read_dbs() >> 24) & 0xf;
	unsigned int i;

	c->watch_reg_count = (ibcn < dbcn) ? ibcn : dbcn;
	c->watch_reg_use_cnt = (c->watch_reg_count < 4) ? c->watch_reg_count : 4;

	for (i = 0; i < c->watch_reg_use_cnt; i++)
		c->watch_reg_masks[i] = MIPS_WATCHHI_MASK | MIPS_WATCHLO_IRW;
}
