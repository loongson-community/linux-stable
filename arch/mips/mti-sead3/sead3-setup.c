/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 2012 MIPS Technologies, Inc.  All rights reserved.
 */
#include <linux/init.h>

const char *get_system_type(void)
{
	return "MIPS SEAD3";
}

void __init plat_mem_setup(void)
{
}
