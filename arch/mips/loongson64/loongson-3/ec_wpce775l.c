/*
 * EC (Embedded Controller) WPCE775L device function for Linux.
 * Author  : Wang Rui <wangr@lemote.com>
 * Author  : Huang Wei <huangw@lemote.com>
 * Date    : 2011-02-21
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <linux/module.h>
#include <linux/spinlock.h>
#include <linux/delay.h>

#include <ec_wpce775l.h>

/* This spinlock is dedicated for 62&66 ports and super io port access. */
extern spinlock_t i8042_lock;
#define index_access_lock i8042_lock
DEFINE_SPINLOCK(port_access_lock);

static int send_ec_command(unsigned char command)
{
	int timeout, ret = 0;

	timeout = EC_SEND_TIMEOUT;
	while ((inb(EC_STS_PORT) & EC_IBF) && --timeout)
		;
	if (!timeout) {
		printk(KERN_ERR "Timeout while sending command 0x%02x to EC!\n", command);
		ret = -1;
		goto out;
	}

	outb(command, EC_CMD_PORT);

out:
	return ret;
}

static int send_ec_data(unsigned char data)
{
	int timeout, ret = 0;

	timeout = EC_SEND_TIMEOUT;
	while ((inb(EC_STS_PORT) & EC_IBF) && --timeout)
		;
	if (!timeout) {
		printk(KERN_ERR "Timeout while sending data 0x%02x to EC!\n", data);
		ret = -1;
		goto out;
	}

	outb(data, EC_DAT_PORT);

out:
	return ret;
}

static unsigned char recv_ec_data(void)
{
	int timeout;
	unsigned char data;

	timeout = EC_RECV_TIMEOUT;
	while (!(inb(EC_STS_PORT) & EC_OBF) && --timeout)
		;
	if (!timeout) {
		printk(KERN_ERR "Timeout while receiving data from EC! status 0x%x.\n", inb(EC_STS_PORT));
		data = 0;
		goto skip_data;
	}

	data = inb(EC_DAT_PORT);

skip_data:
	return data;
}

void clean_ec_event_status(void)
{
	unsigned long flags;

	spin_lock_irqsave(&port_access_lock, flags);
	outl(0x404000, 0x810);
	spin_unlock_irqrestore(&port_access_lock, flags);
}
EXPORT_SYMBOL(clean_ec_event_status);

unsigned char ec_read(unsigned char index)
{
	unsigned char value = 0;
	unsigned long flags;
	int ret = 0;

	spin_lock_irqsave(&index_access_lock, flags);
	ret = send_ec_command(CMD_READ_EC);
	if (ret < 0) {
		printk(KERN_ERR "Send command fail!\n");
		value = 0;
		goto out;
	}
	ret = send_ec_data(index);
	if (ret < 0) {
		printk(KERN_ERR "Send data fail!\n");
		value = 0;
		goto out;
	}
	value = recv_ec_data();
out:
	spin_unlock_irqrestore(&index_access_lock, flags);

	return value;
}
EXPORT_SYMBOL(ec_read);

unsigned char ec_read_all(unsigned char command, unsigned char index)
{
	unsigned char value = 0;
	unsigned long flags;
	int ret = 0;

	spin_lock_irqsave(&index_access_lock, flags);
	ret = send_ec_command(command);
	if (ret < 0) {
		printk(KERN_ERR "Send command fail!\n");
		goto out;
	}
	ret = send_ec_data(index);
	if (ret < 0) {
		printk(KERN_ERR "Send data fail!\n");
		goto out;
	}
	value = recv_ec_data();
out:
	spin_unlock_irqrestore(&index_access_lock, flags);

	return value;
}
EXPORT_SYMBOL(ec_read_all);

unsigned char ec_read_noindex(unsigned char command)
{
	unsigned char value = 0;
	unsigned long flags;
	int ret = 0;

	spin_lock_irqsave(&index_access_lock, flags);
	ret = send_ec_command(command);
	if (ret < 0) {
		printk(KERN_ERR "Send command fail!\n");
		goto out;
	}
	value = recv_ec_data();
out:
	spin_unlock_irqrestore(&index_access_lock, flags);

	return value;
}
EXPORT_SYMBOL(ec_read_noindex);

int ec_write(unsigned char index, unsigned char data)
{
	int ret = 0;
	unsigned long flags;

	spin_lock_irqsave(&index_access_lock, flags);
	ret = send_ec_command(CMD_WRITE_EC);
	if (ret < 0) {
		printk(KERN_ERR "Send command 0x81 fail!\n");
		goto out;
	}
	ret = send_ec_data(index);
	if (ret < 0) {
		printk(KERN_ERR "Send index 0x%x fail!\n", index);
		goto out;
	}

	ret = send_ec_data(data);
	if (ret < 0) {
		printk(KERN_ERR "Send data 0x%x fail!\n", data);
	}
out:
	spin_unlock_irqrestore(&index_access_lock, flags);

	return ret;
}
EXPORT_SYMBOL(ec_write);

int ec_write_all(unsigned char command, unsigned char index, unsigned char data)
{
	unsigned long flags;

	spin_lock_irqsave(&index_access_lock, flags);
	send_ec_command(command);
	send_ec_data(index);
	send_ec_data(data);
	spin_unlock_irqrestore(&index_access_lock, flags);

	return 0;
}
EXPORT_SYMBOL(ec_write_all);

int ec_write_noindex(unsigned char command, unsigned char data)
{
	unsigned long flags;

	spin_lock_irqsave(&index_access_lock, flags);
	send_ec_command(command);
	send_ec_data(data);
	spin_unlock_irqrestore(&index_access_lock, flags);

	return 0;
}
EXPORT_SYMBOL(ec_write_noindex);

int ec_query_get_event_num(void)
{
	unsigned char value = 0;
	unsigned long flags;
	int ret = 0;
	unsigned int timeout;

	spin_lock_irqsave(&index_access_lock, flags);
	ret = send_ec_command(CMD_GET_EVENT_NUM);
	if (ret < 0) {
		printk(KERN_ERR "Send command fail!\n");
		goto out;
	}

	/* check if the command is received by ec */
	timeout = EC_CMD_TIMEOUT;
	while ((inb(EC_STS_PORT) & EC_IBF) && timeout--)
		;
	if (timeout <= 0) {
		printk(KERN_ERR "EC QUERY SEQ: deadable error : timeout...\n");
		ret = -EINVAL;
		goto out;
	}

	value = recv_ec_data();
out:
	spin_unlock_irqrestore(&index_access_lock, flags);

	return value;
}
EXPORT_SYMBOL(ec_query_get_event_num);
