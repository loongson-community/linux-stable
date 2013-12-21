#ifndef __ASM_MACH_LOONGSON_SPACES_H_
#define __ASM_MACH_LOONGSON_SPACES_H_

#ifndef CAC_BASE
#if defined(CONFIG_64BIT)
#if defined(CONFIG_DMA_NONCOHERENT) || defined(CONFIG_CPU_LOONGSON3)
#define CAC_BASE        _AC(0x9800000000000000, UL)
#else
#define CAC_BASE        _AC(0xa800000000000000, UL)
#endif /* CONFIG_DMA_NONCOHERENT || CONFIG_CPU_LOONGSON3 */
#endif /* CONFIG_64BIT */
#endif /* CONFIG_CAC_BASE */

#include <asm/mach-generic/spaces.h>
#endif
