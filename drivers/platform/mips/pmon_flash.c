/*
 *  Copyright www.lemote.com
 *
 *  This program is free software; you can redistribute  it and/or modify it
 *  under  the terms of  the GNU General  Public License as published by the
 *  Free Software Foundation;  either version 2 of the  License, or (at your
 *  option) any later version.
 *
 *  THIS  SOFTWARE  IS PROVIDED   ``AS  IS'' AND   ANY  EXPRESS OR IMPLIED
 *  WARRANTIES,   INCLUDING, BUT NOT  LIMITED  TO, THE IMPLIED WARRANTIES OF
 *  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN
 *  NO  EVENT  SHALL   THE AUTHOR  BE    LIABLE FOR ANY   DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 *  NOT LIMITED   TO, PROCUREMENT OF  SUBSTITUTE GOODS  OR SERVICES; LOSS OF
 *  USE, DATA,  OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 *  ANY THEORY OF LIABILITY, WHETHER IN  CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 *  THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include <linux/module.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <asm/io.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/map.h>
#include <linux/mtd/partitions.h>
#include <linux/version.h>

#define FLASH_PHYS_ADDR 0x1fc00000
#define FLASH_SIZE 0x100000

#define FLASH_PARTITION0_ADDR 0x00000000
#define FLASH_PARTITION0_SIZE 0x00100000

struct map_info flash_map = {
	.name =	"flash device",
	.size =	FLASH_SIZE,
	.bankwidth = 1,
};

struct mtd_partition flash_parts[] = {
	{
		.name =		"Bootloader",
		.offset	=	FLASH_PARTITION0_ADDR,
		.size =		FLASH_PARTITION0_SIZE
	},
};

#define PARTITION_COUNT ARRAY_SIZE(flash_parts)

static struct mtd_info *mymtd;

int __init init_flash(void)
{
	printk(KERN_NOTICE "Flash flash device: %x at %x\n",
			FLASH_SIZE, FLASH_PHYS_ADDR);

	flash_map.phys = FLASH_PHYS_ADDR;
	flash_map.virt = ioremap(FLASH_PHYS_ADDR,
					FLASH_SIZE);

	if (!flash_map.virt) {
		printk("Failed to ioremap\n");
		return -EIO;
	}

	simple_map_init(&flash_map);

	mymtd = do_map_probe("jedec_probe", &flash_map);
	if (mymtd) {
		mtd_device_register(mymtd, flash_parts, PARTITION_COUNT);
		printk(KERN_NOTICE "pmon flash device initialized\n");
		return 0;
	}

	iounmap((void *)flash_map.virt);
	return -ENXIO;
}

static void __exit cleanup_flash(void)
{
	if (mymtd) {
		mtd_device_unregister(mymtd);
		map_destroy(mymtd);
	}
	if (flash_map.virt) {
		iounmap((void *)flash_map.virt);
		flash_map.virt = 0;
	}
}

module_init(init_flash);
module_exit(cleanup_flash);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Yanhua");
MODULE_DESCRIPTION("MTD map driver for pmon programming module");
