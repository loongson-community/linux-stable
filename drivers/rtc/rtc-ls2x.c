/*
 * Loongson-2H Real Time Clock interface for Linux
 *
 * Author: Shaozong Liu <liushaozong@loongson.cn>
 *	   Huacai Chen <chenhc@lemote.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version
 * 2 of the License, or (at your option) any later version.
 */

#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/ioport.h>
#include <linux/delay.h>
#include <linux/rtc.h>
#include <linux/bcd.h>
#include <linux/platform_device.h>
#include <linux/time.h>
#include <linux/spinlock.h>
#include <asm/io.h>
#include <asm/time.h>
#include <loongson-pch.h>

/**
 * Loongson-2H rtc register
 */

#define TOY_TRIM_REG   0x20
#define TOY_WRITE0_REG 0x24
#define TOY_WRITE1_REG 0x28
#define TOY_READ0_REG  0x2c
#define TOY_READ1_REG  0x30
#define TOY_MATCH0_REG 0x34
#define TOY_MATCH1_REG 0x38
#define TOY_MATCH2_REG 0x3c
#define RTC_CTRL_REG   0x40
#define RTC_TRIM_REG   0x60
#define RTC_WRITE0_REG 0x64
#define RTC_READE0_REG 0x68
#define RTC_MATCH0_REG 0x6c
#define RTC_MATCH1_REG 0x70
#define RTC_MATCH2_REG 0x74

/**
 * shift bits and filed mask
 */
#define TOY_MON_MASK   0x3f
#define TOY_DAY_MASK   0x1f
#define TOY_HOUR_MASK  0x1f
#define TOY_MIN_MASK   0x3f
#define TOY_SEC_MASK   0x3f
#define TOY_MSEC_MASK  0xf

#define TOY_MON_SHIFT  26
#define TOY_DAY_SHIFT  21
#define TOY_HOUR_SHIFT 16
#define TOY_MIN_SHIFT  10
#define TOY_SEC_SHIFT  4
#define TOY_MSEC_SHIFT 0

#define rtc_write(val, addr)   writel(val, rtc_reg_base + (addr))
#define rtc_read(addr)         readl(rtc_reg_base + (addr))

struct ls2h_rtc_info {
	struct platform_device *pdev;
	struct rtc_device *rtc_dev;
	struct resource *mem_res;
	void __iomem *rtc_base;
	int irq_base;
};

static void __iomem *rtc_reg_base;

static int ls2h_rtc_read_time(struct device *dev, struct rtc_time *tm)
{
	unsigned int val;
	unsigned long flags;

	spin_lock_irqsave(&rtc_lock, flags);

	val = rtc_read(TOY_READ1_REG);
	tm->tm_year = val;
	val = rtc_read(TOY_READ0_REG);
	tm->tm_sec = (val >> TOY_SEC_SHIFT) & TOY_SEC_MASK;
	tm->tm_min = (val >> TOY_MIN_SHIFT) & TOY_MIN_MASK;
	tm->tm_hour = (val >> TOY_HOUR_SHIFT) & TOY_HOUR_MASK;
	tm->tm_mday = (val >> TOY_DAY_SHIFT) & TOY_DAY_MASK;
	tm->tm_mon = ((val >> TOY_MON_SHIFT) & TOY_MON_MASK) - 1;

	spin_unlock_irqrestore(&rtc_lock, flags);

	return 0;
}

static int ls2h_rtc_set_time(struct device *dev, struct rtc_time *tm)
{
	unsigned int val = 0;
	unsigned long flags;

	spin_lock_irqsave(&rtc_lock, flags);

	val |= (tm->tm_sec << TOY_SEC_SHIFT);
	val |= (tm->tm_min << TOY_MIN_SHIFT);
	val |= (tm->tm_hour << TOY_HOUR_SHIFT);
	val |= (tm->tm_mday << TOY_DAY_SHIFT);
	val |= ((tm->tm_mon + 1) << TOY_MON_SHIFT);
	rtc_write(val, TOY_WRITE0_REG);
	val = tm->tm_year;
	rtc_write(val, TOY_WRITE1_REG);

	spin_unlock_irqrestore(&rtc_lock, flags);

	return 0;
}

static struct rtc_class_ops ls2h_rtc_ops = {
	.read_time = ls2h_rtc_read_time,
	.set_time = ls2h_rtc_set_time,
};

static int ls2h_rtc_probe(struct platform_device *pdev)
{
	struct resource *res, *mem;
	struct rtc_device *rtc;
	struct ls2h_rtc_info *info;

	info = kzalloc(sizeof(struct ls2h_rtc_info), GFP_KERNEL);
	if (!info) {
		pr_debug("%s: no enough memory\n", pdev->name);
		return -ENOMEM;
	}

	info->pdev = pdev;
	info->irq_base = platform_get_irq(pdev, 0);
	if (info->irq_base <= 0) {
		pr_debug("%s: no irq?\n", pdev->name);
		return -ENOENT;
	}

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!res) {
		pr_debug("%s: RTC resource data missing\n", pdev->name);
		return -ENOENT;
	}

	mem = request_mem_region(res->start, resource_size(res), pdev->name);
	if (!mem) {
		pr_debug("%s: RTC registers at %x are not free\n",
			 pdev->name, (unsigned int)res->start);
		return -EBUSY;
	}
	info->mem_res = mem;

	info->rtc_base = ioremap(res->start, resource_size(res));
	if (!info->rtc_base) {
		pr_debug("%s: RTC registers can't be mapped\n", pdev->name);
		goto fail;
	}
	rtc_reg_base = info->rtc_base;

	rtc = info->rtc_dev = rtc_device_register(pdev->name, &pdev->dev,
						  &ls2h_rtc_ops, THIS_MODULE);
	if (IS_ERR(info->rtc_dev)) {
		pr_debug("%s: can't register RTC device, err %ld\n",
			 pdev->name, PTR_ERR(rtc));
		goto fail0;
	}
	platform_set_drvdata(pdev, info);
	dev_set_drvdata(&rtc->dev, info);
	return 0;

      fail0:
	iounmap(info->rtc_base);
      fail:
	release_resource(mem);
	kfree(info);
	return -EIO;
}

static int ls2h_rtc_remove(struct platform_device *pdev)
{
	struct ls2h_rtc_info *info = platform_get_drvdata(pdev);
	struct rtc_device *rtc = info->rtc_dev;

	iounmap(info->rtc_base);
	release_resource(dev_get_drvdata(&rtc->dev));
	release_resource(info->mem_res);
	rtc_device_unregister(rtc);
	kfree(info);
	return 0;
}

#ifdef CONFIG_OF
static struct of_device_id ls2h_rtc_id_table[] = {
	{.compatible = "loongson,ls2h-rtc"},
	{},
};
#endif

static struct platform_driver ls2h_rtc_driver = {
	.probe		= ls2h_rtc_probe,
	.remove		= ls2h_rtc_remove,
	.driver		= {
		.name	= "ls2h-rtc",
		.owner	= THIS_MODULE,
#ifdef CONFIG_OF
		.of_match_table = of_match_ptr(ls2h_rtc_id_table),
#endif
	},
};

static int __init rtc_init(void)
{
	return platform_driver_register(&ls2h_rtc_driver);
}

static void __exit rtc_exit(void)
{
	platform_driver_unregister(&ls2h_rtc_driver);
}

module_init(rtc_init);
module_exit(rtc_exit);

MODULE_AUTHOR("Liu Shaozong");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:ls2h-rtc");
