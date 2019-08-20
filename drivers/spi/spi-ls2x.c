/*
 * Loongson LS2H/LS2K/LS7A SPI driver
 *
 * Copyright (C) 2017 Juxin Gao <gaojuxin@loongson.cn>
 * Copyright (C) 2019 Huacai Chen <chenhc@lemote.com>
 * Copyright (C) 2019 Liangliang Huang <huangll@lemote.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/err.h>
#include <linux/spi/spi.h>
#include <linux/pci.h>
#include <linux/of.h>
/*define spi register */
#define	SPCR	0x00
#define	SPSR	0x01
#define FIFO	0x02
#define	SPER	0x03
#define	PARA	0x04
#define	SFCS	0x05
#define	TIMI	0x06

struct ls2x_spi {
	struct work_struct	work;
	spinlock_t		lock;

	struct	list_head	msg_queue;
	struct	spi_master	*master;
	void	__iomem		*base;
	int cs_active;
	unsigned int hz;
	unsigned char spcr, sper;
	struct workqueue_struct	*wq;
};

static inline int set_cs(struct ls2x_spi *ls2x_spi, struct spi_device *spi, int val);

static void ls2x_spi_write_reg(struct ls2x_spi *spi,
		unsigned char reg, unsigned char data)
{
	writeb(data, spi->base +reg);
}

static char ls2x_spi_read_reg(struct ls2x_spi *spi,
		unsigned char reg)
{
	return readb(spi->base + reg);
}

static int ls2x_spi_update_state(struct ls2x_spi *ls2x_spi,struct spi_device *spi,
		struct spi_transfer *t)
{
	unsigned char val;
	unsigned long clk;
	unsigned int hz, bit, div, div_tmp;
	const char rdiv[12] = {0, 1, 4, 2, 3, 5, 6, 7, 8, 9, 10, 11};

	hz  = t ? t->speed_hz : spi->max_speed_hz;

	if (!hz)
		hz = spi->max_speed_hz;

	if (hz && ls2x_spi->hz != hz) {
		clk = 100000000;
		div = DIV_ROUND_UP(clk, hz);

		if (div < 2)
			div = 2;

		if (div > 4096)
			div = 4096;

		bit = fls(div) - 1;
		if ((1<<bit) == div)
			bit--;
		div_tmp = rdiv[bit];

		dev_dbg(&spi->dev, "clk = %ld hz = %d div_tmp = %d bit = %d\n",
				clk, hz, div_tmp, bit);

		ls2x_spi->hz = hz;
		ls2x_spi->spcr = div_tmp & 3;
		ls2x_spi->sper = (div_tmp >> 2) & 3;

		val = ls2x_spi_read_reg(ls2x_spi, SPCR);
		ls2x_spi_write_reg(ls2x_spi, SPCR, (val & ~3) | ls2x_spi->spcr);
		val = ls2x_spi_read_reg(ls2x_spi, SPER);
		ls2x_spi_write_reg(ls2x_spi, SPER, (val & ~3) | ls2x_spi->sper);
	}

	return 0;
}



static int ls2x_spi_setup(struct spi_device *spi)
{
	struct ls2x_spi *ls2x_spi;

	ls2x_spi = spi_master_get_devdata(spi->master);
	if (spi->bits_per_word % 8)
		return -EINVAL;

	if(spi->chip_select >= spi->master->num_chipselect)
		return -EINVAL;

	ls2x_spi_update_state(ls2x_spi, spi, NULL);

	set_cs(ls2x_spi, spi, 1);

	return 0;
}

static int ls2x_spi_write_read_8bit( struct spi_device *spi,
		const u8 **tx_buf, u8 **rx_buf, unsigned int num)
{
	struct ls2x_spi *ls2x_spi;
	ls2x_spi = spi_master_get_devdata(spi->master);

	if (tx_buf && *tx_buf){
		ls2x_spi_write_reg(ls2x_spi, FIFO, *((*tx_buf)++));
		while((ls2x_spi_read_reg(ls2x_spi, SPSR) & 0x1) == 1);
	} else {
		ls2x_spi_write_reg(ls2x_spi, FIFO, 0);
		while((ls2x_spi_read_reg(ls2x_spi, SPSR) & 0x1) == 1);
	}

	if (rx_buf && *rx_buf) {
		*(*rx_buf)++ = ls2x_spi_read_reg(ls2x_spi, FIFO);
	} else {
		ls2x_spi_read_reg(ls2x_spi, FIFO);
	}

	return 1;
}


static unsigned int ls2x_spi_write_read(struct spi_device *spi, struct spi_transfer *xfer)
{
	struct ls2x_spi *ls2x_spi;
	unsigned int count;
	const u8 *tx = xfer->tx_buf;
	u8 *rx = xfer->rx_buf;

	ls2x_spi = spi_master_get_devdata(spi->master);
	count = xfer->len;

	do {
		if (ls2x_spi_write_read_8bit(spi, &tx, &rx, count) < 0)
			goto out;
		count--;
	} while (count);

out:
	return xfer->len - count;

}

static inline int set_cs(struct ls2x_spi *ls2x_spi, struct spi_device *spi, int val)
{
	int cs = ls2x_spi_read_reg(ls2x_spi, SFCS) & ~(0x11 << spi->chip_select);
	ls2x_spi_write_reg(ls2x_spi, SFCS, ( val ? (0x11 << spi->chip_select):(0x1 << spi->chip_select)) | cs);
	return 0;
}

static void ls2x_spi_work(struct work_struct *work)
{
	int param;
	struct ls2x_spi *ls2x_spi = container_of(work, struct ls2x_spi, work);

	spin_lock(&ls2x_spi->lock);
	param = ls2x_spi_read_reg(ls2x_spi, PARA);
	ls2x_spi_write_reg(ls2x_spi, PARA, param&~1);

	while (!list_empty(&ls2x_spi->msg_queue)) {
		struct spi_message *m;
		struct spi_device  *spi;
		struct spi_transfer *t = NULL;

		m = container_of(ls2x_spi->msg_queue.next, struct spi_message, queue);

		list_del_init(&m->queue);
		spin_unlock(&ls2x_spi->lock);

		spi = m->spi;

		/*setup spi clock*/
		ls2x_spi_update_state(ls2x_spi, spi, NULL);

		/*in here set cs*/
		set_cs(ls2x_spi, spi, 0);

		list_for_each_entry(t, &m->transfers, transfer_list) {
			if (t->len)
				m->actual_length +=
					ls2x_spi_write_read(spi, t);
		}

		set_cs(ls2x_spi, spi, 1);
		m->complete(m->context);

		spin_lock(&ls2x_spi->lock);
	}

	ls2x_spi_write_reg(ls2x_spi, PARA, param);
	spin_unlock(&ls2x_spi->lock);
}

static int ls2x_spi_transfer(struct spi_device *spi, struct spi_message *m)
{
	struct ls2x_spi	*ls2x_spi;
	struct spi_transfer *t = NULL;

	m->actual_length = 0;
	m->status	 = 0;
	if (list_empty(&m->transfers) || !m->complete)
		return -EINVAL;

	ls2x_spi = spi_master_get_devdata(spi->master);

	list_for_each_entry(t, &m->transfers, transfer_list) {
		if (t->tx_buf == NULL && t->rx_buf == NULL && t->len) {
			dev_err(&spi->dev,
					"message rejected : "
					"invalid transfer data buffers\n");
			goto msg_rejected;
		}

		/*other things not check*/

	}

	spin_lock(&ls2x_spi->lock);
	list_add_tail(&m->queue, &ls2x_spi->msg_queue);
	queue_work(ls2x_spi->wq, &ls2x_spi->work);
	spin_unlock(&ls2x_spi->lock);

	return 0;

msg_rejected:
	m->status = -EINVAL;
	if (m->complete)
		m->complete(m->context);

	return -EINVAL;
}

static int ls2x_spi_probe(struct platform_device *pdev)
{
	int ret;
	struct resource		*res;
	struct spi_master	*master;
	struct ls2x_spi		*spi;

	master = spi_alloc_master(&pdev->dev, sizeof(struct ls2x_spi));

	if (master == NULL) {
		dev_dbg(&pdev->dev, "master allocation failed\n");
		return -ENOMEM;
	}

	if (pdev->id != -1)
		master->bus_num	= pdev->id;

	master->setup = ls2x_spi_setup;
	master->transfer = ls2x_spi_transfer;
	master->num_chipselect = 4;
#ifdef CONFIG_OF
	master->dev.of_node = of_node_get(pdev->dev.of_node);
#endif
	dev_set_drvdata(&pdev->dev, master);

	spi = spi_master_get_devdata(master);

	spi->wq	= create_singlethread_workqueue(pdev->name);

	spi->master = master;

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (res == NULL) {
		dev_err(&pdev->dev, "Cannot get IORESOURCE_MEM\n");
		ret = -ENOENT;
		goto free_master;
	}

	spi->base = ioremap(res->start, (res->end - res->start) + 1);
	if (spi->base == NULL) {
		dev_err(&pdev->dev, "Cannot map IO\n");
		ret = -ENXIO;
		goto unmap_io;
	}

	ls2x_spi_write_reg(spi, SPCR, 0x51);
	ls2x_spi_write_reg(spi, SPER, 0x04);
	ls2x_spi_write_reg(spi, TIMI, 0x01);
	ls2x_spi_write_reg(spi, PARA, 0x40);
	INIT_WORK(&spi->work, ls2x_spi_work);

	spin_lock_init(&spi->lock);
	INIT_LIST_HEAD(&spi->msg_queue);

	ret = spi_register_master(master);
	if (ret < 0)
		goto unmap_io;

	return ret;

unmap_io:
	iounmap(spi->base);
free_master:
	kfree(master);
	spi_master_put(master);
	return ret;
}

#ifdef CONFIG_OF
static struct of_device_id ls2x_spi_id_table[] = {
	{ .compatible = "loongson,ls2h-spi", },
	{ .compatible = "loongson,ls2k-spi", },
	{ .compatible = "loongson,ls7a-spi", },
	{ },
};
#endif

static struct platform_driver ls2x_spi_driver = {
	.probe = ls2x_spi_probe,
	.driver	= {
		.name	= "ls2x-spi",
		.owner	= THIS_MODULE,
		.bus = &platform_bus_type,
#ifdef CONFIG_OF
		.of_match_table = of_match_ptr(ls2x_spi_id_table),
#endif
	},
};

static int __init ls2x_spi_init(void)
{
	return platform_driver_register(&ls2x_spi_driver);
}

static void __exit ls2x_spi_exit(void)
{
	platform_driver_unregister(&ls2x_spi_driver);
}

late_initcall(ls2x_spi_init);
module_exit(ls2x_spi_exit);

MODULE_AUTHOR("Juxin Gao <gaojuxin@loongson.cn>");
MODULE_AUTHOR("Huacai Chen <chenhc@lemote.com>");
MODULE_AUTHOR("Liangliang Huang <huangll@lemote.com>");
MODULE_DESCRIPTION("Loongson LS2H/LS2K/LS7A SPI driver");
MODULE_LICENSE("GPL");
