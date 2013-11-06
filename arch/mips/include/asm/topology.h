/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 2007 by Ralf Baechle
 */
#ifndef __ASM_TOPOLOGY_H
#define __ASM_TOPOLOGY_H

#include <topology.h>

#ifndef topology_physical_package_id
#define topology_physical_package_id(cpu)	(cpu_data[cpu].package)
#endif
#ifndef topology_core_id
#define topology_core_id(cpu)			(cpu_data[cpu].core)
#endif
#ifndef topology_core_cpumask
#define topology_core_cpumask(cpu)		(&cpu_core_map[cpu])
#endif
#ifndef topology_thread_cpumask
#define topology_thread_cpumask(cpu)		(&cpu_sibling_map[cpu])
#endif

#endif /* __ASM_TOPOLOGY_H */
