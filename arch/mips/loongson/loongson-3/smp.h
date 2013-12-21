/* for Loongson-3A smp support */
extern unsigned long long smp_group[4];

/* 4 groups(nodes) in maximum in numa case */
#define smp_core_group0_base	(smp_group[0])
#define smp_core_group1_base	(smp_group[1])
#define smp_core_group2_base	(smp_group[2])
#define smp_core_group3_base	(smp_group[3])

/* 4 cores in each group(node) */
#define smp_core0_offset  0x000
#define smp_core1_offset  0x100
#define smp_core2_offset  0x200
#define smp_core3_offset  0x300

/* ipi registers offsets */
#define STATUS0  0x00
#define EN0      0x04
#define SET0     0x08
#define CLEAR0   0x0c
#define STATUS1  0x10
#define MASK1    0x14
#define SET1     0x18
#define CLEAR1   0x1c
#define BUF      0x20
