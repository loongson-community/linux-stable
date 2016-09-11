/*
 * Format of an instruction in memory.
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 1996, 2000 by Ralf Baechle
 * Copyright (C) 2006 by Thiemo Seufer
 */
#ifndef _ASM_INST_H
#define _ASM_INST_H

#include <asm/asm.h>
#include <uapi/asm/inst.h>

/* HACHACHAHCAHC ...  */

/* In case some other massaging is needed, keep MIPSInst as wrapper */

#define MIPSInst(x) x

#define I_OPCODE_SFT	26
#define MIPSInst_OPCODE(x) (MIPSInst(x) >> I_OPCODE_SFT)

#define I_JTARGET_SFT	0
#define MIPSInst_JTARGET(x) (MIPSInst(x) & 0x03ffffff)

#define I_RS_SFT	21
#define MIPSInst_RS(x) ((MIPSInst(x) & 0x03e00000) >> I_RS_SFT)

#define I_RT_SFT	16
#define MIPSInst_RT(x) ((MIPSInst(x) & 0x001f0000) >> I_RT_SFT)

#define I_IMM_SFT	0
#define MIPSInst_SIMM(x) ((int)((short)(MIPSInst(x) & 0xffff)))
#define MIPSInst_UIMM(x) (MIPSInst(x) & 0xffff)

#define I_CACHEOP_SFT	18
#define MIPSInst_CACHEOP(x) ((MIPSInst(x) & 0x001c0000) >> I_CACHEOP_SFT)

#define I_CACHESEL_SFT	16
#define MIPSInst_CACHESEL(x) ((MIPSInst(x) & 0x00030000) >> I_CACHESEL_SFT)

#define I_RD_SFT	11
#define MIPSInst_RD(x) ((MIPSInst(x) & 0x0000f800) >> I_RD_SFT)

#define I_RE_SFT	6
#define MIPSInst_RE(x) ((MIPSInst(x) & 0x000007c0) >> I_RE_SFT)

#define I_FUNC_SFT	0
#define MIPSInst_FUNC(x) (MIPSInst(x) & 0x0000003f)

#define I_FFMT_SFT	21
#define MIPSInst_FFMT(x) ((MIPSInst(x) & 0x01e00000) >> I_FFMT_SFT)

#define I_FT_SFT	16
#define MIPSInst_FT(x) ((MIPSInst(x) & 0x001f0000) >> I_FT_SFT)

#define I_FS_SFT	11
#define MIPSInst_FS(x) ((MIPSInst(x) & 0x0000f800) >> I_FS_SFT)

#define I_FD_SFT	6
#define MIPSInst_FD(x) ((MIPSInst(x) & 0x000007c0) >> I_FD_SFT)

#define I_FR_SFT	21
#define MIPSInst_FR(x) ((MIPSInst(x) & 0x03e00000) >> I_FR_SFT)

#define I_FMA_FUNC_SFT	2
#define MIPSInst_FMA_FUNC(x) ((MIPSInst(x) & 0x0000003c) >> I_FMA_FUNC_SFT)

#define I_FMA_FFMT_SFT	0
#define MIPSInst_FMA_FFMT(x) (MIPSInst(x) & 0x00000003)

typedef unsigned int mips_instruction;

/* microMIPS instruction decode structure. Do NOT export!!! */
struct mm_decoded_insn {
	mips_instruction insn;
	mips_instruction next_insn;
	int pc_inc;
	int next_pc_inc;
	int micro_mips_mode;
};

/* Recode table from 16-bit register notation to 32-bit GPR. Do NOT export!!! */
extern const int reg16to32[];

#define STR(x)	__STR(x)
#define __STR(x)  #x

#ifdef __BIG_ENDIAN
#define     LoadHW(addr, value, res)  \
		__asm__ __volatile__ (".set\tnoat\n"        \
			"1:\tlb\t%0, 0(%2)\n"               \
			"2:\tlbu\t$1, 1(%2)\n\t"            \
			"sll\t%0, 0x8\n\t"                  \
			"or\t%0, $1\n\t"                    \
			"li\t%1, 0\n"                       \
			"3:\t.set\tat\n\t"                  \
			".insn\n\t"                         \
			".section\t.fixup,\"ax\"\n\t"       \
			"4:\tli\t%1, %3\n\t"                \
			"j\t3b\n\t"                         \
			".previous\n\t"                     \
			".section\t__ex_table,\"a\"\n\t"    \
			STR(PTR)"\t1b, 4b\n\t"              \
			STR(PTR)"\t2b, 4b\n\t"              \
			".previous"                         \
			: "=&r" (value), "=r" (res)         \
			: "r" (addr), "i" (-EFAULT));

#define     LoadW(addr, value, res)   \
		__asm__ __volatile__ (                      \
			"1:\tlwl\t%0, (%2)\n"               \
			"2:\tlwr\t%0, 3(%2)\n\t"            \
			"li\t%1, 0\n"                       \
			"3:\n\t"                            \
			".insn\n\t"                         \
			".section\t.fixup,\"ax\"\n\t"       \
			"4:\tli\t%1, %3\n\t"                \
			"j\t3b\n\t"                         \
			".previous\n\t"                     \
			".section\t__ex_table,\"a\"\n\t"    \
			STR(PTR)"\t1b, 4b\n\t"              \
			STR(PTR)"\t2b, 4b\n\t"              \
			".previous"                         \
			: "=&r" (value), "=r" (res)         \
			: "r" (addr), "i" (-EFAULT));

#define     LoadHWU(addr, value, res) \
		__asm__ __volatile__ (                      \
			".set\tnoat\n"                      \
			"1:\tlbu\t%0, 0(%2)\n"              \
			"2:\tlbu\t$1, 1(%2)\n\t"            \
			"sll\t%0, 0x8\n\t"                  \
			"or\t%0, $1\n\t"                    \
			"li\t%1, 0\n"                       \
			"3:\n\t"                            \
			".insn\n\t"                         \
			".set\tat\n\t"                      \
			".section\t.fixup,\"ax\"\n\t"       \
			"4:\tli\t%1, %3\n\t"                \
			"j\t3b\n\t"                         \
			".previous\n\t"                     \
			".section\t__ex_table,\"a\"\n\t"    \
			STR(PTR)"\t1b, 4b\n\t"              \
			STR(PTR)"\t2b, 4b\n\t"              \
			".previous"                         \
			: "=&r" (value), "=r" (res)         \
			: "r" (addr), "i" (-EFAULT));

#define     LoadWU(addr, value, res)  \
		__asm__ __volatile__ (                      \
			"1:\tlwl\t%0, (%2)\n"               \
			"2:\tlwr\t%0, 3(%2)\n\t"            \
			"dsll\t%0, %0, 32\n\t"              \
			"dsrl\t%0, %0, 32\n\t"              \
			"li\t%1, 0\n"                       \
			"3:\n\t"                            \
			".insn\n\t"                         \
			"\t.section\t.fixup,\"ax\"\n\t"     \
			"4:\tli\t%1, %3\n\t"                \
			"j\t3b\n\t"                         \
			".previous\n\t"                     \
			".section\t__ex_table,\"a\"\n\t"    \
			STR(PTR)"\t1b, 4b\n\t"              \
			STR(PTR)"\t2b, 4b\n\t"              \
			".previous"                         \
			: "=&r" (value), "=r" (res)         \
			: "r" (addr), "i" (-EFAULT));

#define     LoadDW(addr, value, res)  \
		__asm__ __volatile__ (                      \
			"1:\tldl\t%0, (%2)\n"               \
			"2:\tldr\t%0, 7(%2)\n\t"            \
			"li\t%1, 0\n"                       \
			"3:\n\t"                            \
			".insn\n\t"                         \
			"\t.section\t.fixup,\"ax\"\n\t"     \
			"4:\tli\t%1, %3\n\t"                \
			"j\t3b\n\t"                         \
			".previous\n\t"                     \
			".section\t__ex_table,\"a\"\n\t"    \
			STR(PTR)"\t1b, 4b\n\t"              \
			STR(PTR)"\t2b, 4b\n\t"              \
			".previous"                         \
			: "=&r" (value), "=r" (res)         \
			: "r" (addr), "i" (-EFAULT));

#define     StoreHW(addr, value, res) \
		__asm__ __volatile__ (                      \
			".set\tnoat\n"                      \
			"1:\tsb\t%1, 1(%2)\n\t"             \
			"srl\t$1, %1, 0x8\n"                \
			"2:\tsb\t$1, 0(%2)\n\t"             \
			".set\tat\n\t"                      \
			"li\t%0, 0\n"                       \
			"3:\n\t"                            \
			".insn\n\t"                         \
			".section\t.fixup,\"ax\"\n\t"       \
			"4:\tli\t%0, %3\n\t"                \
			"j\t3b\n\t"                         \
			".previous\n\t"                     \
			".section\t__ex_table,\"a\"\n\t"    \
			STR(PTR)"\t1b, 4b\n\t"              \
			STR(PTR)"\t2b, 4b\n\t"              \
			".previous"                         \
			: "=r" (res)                        \
			: "r" (value), "r" (addr), "i" (-EFAULT));

#define     StoreW(addr, value, res)  \
		__asm__ __volatile__ (                      \
			"1:\tswl\t%1,(%2)\n"                \
			"2:\tswr\t%1, 3(%2)\n\t"            \
			"li\t%0, 0\n"                       \
			"3:\n\t"                            \
			".insn\n\t"                         \
			".section\t.fixup,\"ax\"\n\t"       \
			"4:\tli\t%0, %3\n\t"                \
			"j\t3b\n\t"                         \
			".previous\n\t"                     \
			".section\t__ex_table,\"a\"\n\t"    \
			STR(PTR)"\t1b, 4b\n\t"              \
			STR(PTR)"\t2b, 4b\n\t"              \
			".previous"                         \
		: "=r" (res)                                \
		: "r" (value), "r" (addr), "i" (-EFAULT));

#define     StoreDW(addr, value, res) \
		__asm__ __volatile__ (                      \
			"1:\tsdl\t%1,(%2)\n"                \
			"2:\tsdr\t%1, 7(%2)\n\t"            \
			"li\t%0, 0\n"                       \
			"3:\n\t"                            \
			".insn\n\t"                         \
			".section\t.fixup,\"ax\"\n\t"       \
			"4:\tli\t%0, %3\n\t"                \
			"j\t3b\n\t"                         \
			".previous\n\t"                     \
			".section\t__ex_table,\"a\"\n\t"    \
			STR(PTR)"\t1b, 4b\n\t"              \
			STR(PTR)"\t2b, 4b\n\t"              \
			".previous"                         \
		: "=r" (res)                                \
		: "r" (value), "r" (addr), "i" (-EFAULT));
#endif

#ifdef __LITTLE_ENDIAN
#define     LoadHW(addr, value, res)  \
		__asm__ __volatile__ (".set\tnoat\n"        \
			"1:\tlb\t%0, 1(%2)\n"               \
			"2:\tlbu\t$1, 0(%2)\n\t"            \
			"sll\t%0, 0x8\n\t"                  \
			"or\t%0, $1\n\t"                    \
			"li\t%1, 0\n"                       \
			"3:\t.set\tat\n\t"                  \
			".insn\n\t"                         \
			".section\t.fixup,\"ax\"\n\t"       \
			"4:\tli\t%1, %3\n\t"                \
			"j\t3b\n\t"                         \
			".previous\n\t"                     \
			".section\t__ex_table,\"a\"\n\t"    \
			STR(PTR)"\t1b, 4b\n\t"              \
			STR(PTR)"\t2b, 4b\n\t"              \
			".previous"                         \
			: "=&r" (value), "=r" (res)         \
			: "r" (addr), "i" (-EFAULT));

#define     LoadW(addr, value, res)   \
		__asm__ __volatile__ (                      \
			"1:\tlwl\t%0, 3(%2)\n"              \
			"2:\tlwr\t%0, (%2)\n\t"             \
			"li\t%1, 0\n"                       \
			"3:\n\t"                            \
			".insn\n\t"                         \
			".section\t.fixup,\"ax\"\n\t"       \
			"4:\tli\t%1, %3\n\t"                \
			"j\t3b\n\t"                         \
			".previous\n\t"                     \
			".section\t__ex_table,\"a\"\n\t"    \
			STR(PTR)"\t1b, 4b\n\t"              \
			STR(PTR)"\t2b, 4b\n\t"              \
			".previous"                         \
			: "=&r" (value), "=r" (res)         \
			: "r" (addr), "i" (-EFAULT));

#define     LoadHWU(addr, value, res) \
		__asm__ __volatile__ (                      \
			".set\tnoat\n"                      \
			"1:\tlbu\t%0, 1(%2)\n"              \
			"2:\tlbu\t$1, 0(%2)\n\t"            \
			"sll\t%0, 0x8\n\t"                  \
			"or\t%0, $1\n\t"                    \
			"li\t%1, 0\n"                       \
			"3:\n\t"                            \
			".insn\n\t"                         \
			".set\tat\n\t"                      \
			".section\t.fixup,\"ax\"\n\t"       \
			"4:\tli\t%1, %3\n\t"                \
			"j\t3b\n\t"                         \
			".previous\n\t"                     \
			".section\t__ex_table,\"a\"\n\t"    \
			STR(PTR)"\t1b, 4b\n\t"              \
			STR(PTR)"\t2b, 4b\n\t"              \
			".previous"                         \
			: "=&r" (value), "=r" (res)         \
			: "r" (addr), "i" (-EFAULT));

#define     LoadWU(addr, value, res)  \
		__asm__ __volatile__ (                      \
			"1:\tlwl\t%0, 3(%2)\n"              \
			"2:\tlwr\t%0, (%2)\n\t"             \
			"dsll\t%0, %0, 32\n\t"              \
			"dsrl\t%0, %0, 32\n\t"              \
			"li\t%1, 0\n"                       \
			"3:\n\t"                            \
			".insn\n\t"                         \
			"\t.section\t.fixup,\"ax\"\n\t"     \
			"4:\tli\t%1, %3\n\t"                \
			"j\t3b\n\t"                         \
			".previous\n\t"                     \
			".section\t__ex_table,\"a\"\n\t"    \
			STR(PTR)"\t1b, 4b\n\t"              \
			STR(PTR)"\t2b, 4b\n\t"              \
			".previous"                         \
			: "=&r" (value), "=r" (res)         \
			: "r" (addr), "i" (-EFAULT));

#define     LoadDW(addr, value, res)  \
		__asm__ __volatile__ (                      \
			"1:\tldl\t%0, 7(%2)\n"              \
			"2:\tldr\t%0, (%2)\n\t"             \
			"li\t%1, 0\n"                       \
			"3:\n\t"                            \
			".insn\n\t"                         \
			"\t.section\t.fixup,\"ax\"\n\t"     \
			"4:\tli\t%1, %3\n\t"                \
			"j\t3b\n\t"                         \
			".previous\n\t"                     \
			".section\t__ex_table,\"a\"\n\t"    \
			STR(PTR)"\t1b, 4b\n\t"              \
			STR(PTR)"\t2b, 4b\n\t"              \
			".previous"                         \
			: "=&r" (value), "=r" (res)         \
			: "r" (addr), "i" (-EFAULT));

#define     StoreHW(addr, value, res) \
		__asm__ __volatile__ (                      \
			".set\tnoat\n"                      \
			"1:\tsb\t%1, 0(%2)\n\t"             \
			"srl\t$1,%1, 0x8\n"                 \
			"2:\tsb\t$1, 1(%2)\n\t"             \
			".set\tat\n\t"                      \
			"li\t%0, 0\n"                       \
			"3:\n\t"                            \
			".insn\n\t"                         \
			".section\t.fixup,\"ax\"\n\t"       \
			"4:\tli\t%0, %3\n\t"                \
			"j\t3b\n\t"                         \
			".previous\n\t"                     \
			".section\t__ex_table,\"a\"\n\t"    \
			STR(PTR)"\t1b, 4b\n\t"              \
			STR(PTR)"\t2b, 4b\n\t"              \
			".previous"                         \
			: "=r" (res)                        \
			: "r" (value), "r" (addr), "i" (-EFAULT));

#define     StoreW(addr, value, res)  \
		__asm__ __volatile__ (                      \
			"1:\tswl\t%1, 3(%2)\n"              \
			"2:\tswr\t%1, (%2)\n\t"             \
			"li\t%0, 0\n"                       \
			"3:\n\t"                            \
			".insn\n\t"                         \
			".section\t.fixup,\"ax\"\n\t"       \
			"4:\tli\t%0, %3\n\t"                \
			"j\t3b\n\t"                         \
			".previous\n\t"                     \
			".section\t__ex_table,\"a\"\n\t"    \
			STR(PTR)"\t1b, 4b\n\t"              \
			STR(PTR)"\t2b, 4b\n\t"              \
			".previous"                         \
		: "=r" (res)                                \
		: "r" (value), "r" (addr), "i" (-EFAULT));

#define     StoreDW(addr, value, res) \
		__asm__ __volatile__ (                      \
			"1:\tsdl\t%1, 7(%2)\n"              \
			"2:\tsdr\t%1, (%2)\n\t"             \
			"li\t%0, 0\n"                       \
			"3:\n\t"                            \
			".insn\n\t"                         \
			".section\t.fixup,\"ax\"\n\t"       \
			"4:\tli\t%0, %3\n\t"                \
			"j\t3b\n\t"                         \
			".previous\n\t"                     \
			".section\t__ex_table,\"a\"\n\t"    \
			STR(PTR)"\t1b, 4b\n\t"              \
			STR(PTR)"\t2b, 4b\n\t"              \
			".previous"                         \
		: "=r" (res)                                \
		: "r" (value), "r" (addr), "i" (-EFAULT));
#endif

#endif /* _ASM_INST_H */
