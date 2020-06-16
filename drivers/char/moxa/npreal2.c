
/*
 *          npreal2.c  -- MOXA NPort Server family Real TTY driver.
 *
 *      Copyright (C) 1999-2012  Moxa Inc. (support@moxa.com.tw).
 *
 *      This code is loosely based on the Linux serial driver, written by
 *      Linus Torvalds, Theodore T'so and others.
 *
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation; either version 2 of the License, or
 *      (at your option) any later version.
 *
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 *
 *      You should have received a copy of the GNU General Public License
 *      along with this program; if not, write to the Free Software
 *      Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *
 */
#ifdef 		MODVERSIONS
#ifndef 	MODULE
#define 	MODULE
#endif
#endif

#include <linux/version.h>
#define VERSION_CODE(ver,rel,seq)	((ver << 16) | (rel << 8) | seq)
#ifndef _DEBIAN_
#ifdef MODULE
#ifdef MODVERSIONS
#include <linux/modversions.h>
#endif
#include <linux/module.h>
#else
#define	MOD_INC_USE_COUNT
#define MOD_DEC_USE_COUNT
#endif
#include <linux/init.h>
#include <linux/errno.h>
#include <linux/signal.h>
#include <linux/sched.h>
#include <linux/timer.h>
#include <linux/interrupt.h>
#include <linux/tty.h>
#include <linux/tty_flip.h>
#include <linux/serial.h>
#include <linux/serial_reg.h>
#include <linux/major.h>
#include <linux/string.h>
#include <linux/fcntl.h>
#include <linux/ptrace.h>
#include <linux/ioport.h>
#include <linux/mm.h>
#include <linux/proc_fs.h>
#else
#ifdef MODULE
#ifdef MODVERSIONS
#include <config/modversions.h>
#endif
#include <config/module.h>
#else
#define	MOD_INC_USE_COUNT
#define MOD_DEC_USE_COUNT
#endif
#include <config/init.h>
#include <config/errno.h>
#include <config/signal.h>
#include <config/sched.h>
#include <config/timer.h>
#include <config/interrupt.h>
#include <config/tty.h>
#include <config/tty_flip.h>
#include <config/serial.h>
#include <config/serial_reg.h>
#include <config/major.h>
#include <config/string.h>
#include <config/fcntl.h>
#include <config/ptrace.h>
#include <config/ioport.h>
#include <config/mm.h>
#include <config/proc_fs.h>
#endif

#include "np_ver.h"

/* include/linux/semaphore.h modification */
#define init_MUTEX(sem) sema_init(sem, 1)

#include <asm/uaccess.h>
#include <linux/poll.h>
#define put_to_user(arg1, arg2) put_user(arg1, arg2)
#define get_from_user(arg1, arg2) get_user(arg1, arg2)

#include "npreal2.h"
# if (LINUX_VERSION_CODE >= VERSION_CODE(3,10,0))
#include <linux/slab.h>
#endif

#define	NPREAL_EVENT_TXLOW	 1
#define	NPREAL_EVENT_HANGUP	 2

#define SERIAL_DO_RESTART

#define SERIAL_TYPE_NORMAL	1
#define SERIAL_TYPE_CALLOUT	2

#define WAKEUP_CHARS		256

#ifndef MAX_SCHEDULE_TIMEOUT
#define	MAX_SCHEDULE_TIMEOUT	((long)(~0UL>>1))
#endif

#define PORTNO(x)	((x)->index)

#define RELEVANT_IFLAG(iflag)	(iflag & (IGNBRK|BRKINT|IGNPAR|PARMRK|INPCK))

#ifndef MIN
#define MIN(a,b)	((a) < (b) ? (a) : (b))
#endif

#define		NPREALMAJOR		 33
#define		NPREALCUMAJOR	 38

static int ttymajor=NPREALMAJOR;
static int calloutmajor=NPREALCUMAJOR;
static int verbose=1;

int	MXDebugLevel = MX_DEBUG_ERROR;

#ifdef MODULE
/* Variables for insmod */

MODULE_AUTHOR("Moxa Tech.,www.moxa.com.tw");
MODULE_DESCRIPTION("MOXA Async/NPort Server Family Real TTY Driver");
module_param(ttymajor, int, 0);
module_param(calloutmajor, int, 0);
module_param(verbose, int, 0644);
MODULE_VERSION(NPREAL_VERSION);
#ifdef MODULE_LICENSE
MODULE_LICENSE("GPL");
# endif

#endif /* MODULE */

#define	NPREAL_PORTS	 256

#define	DE211	211
#define	DE311	311
#define	DE301	301
#define	DE302	302
#define	DE304	304
#define	DE331	331
#define	DE332	332
#define	DE334	334
#define	DE303	303
#define	DE308	308
#define	DE309	309
#define	CN2100	2100
#define	CN2500	2500

#ifndef B921600
#define	B921600	(B460800 + 1)
#endif

#define	NPREAL_ASPP_COMMAND_SET		1
#define	NPREAL_LOCAL_COMMAND_SET	2

// local command set
#define	LOCAL_CMD_TTY_USED			1
#define	LOCAL_CMD_TTY_UNUSED		2
#define NPREAL_NET_CONNECTED		3
#define NPREAL_NET_DISCONNECTED		4
#define NPREAL_NET_SETTING			5
#define NPREAL_NET_GET_TTY_STATUS	6

#define	NPREAL_CMD_TIMEOUT		10*HZ  // 10 seconds

#define	NPREAL_NET_CMD_RETRIEVE		1
#define	NPREAL_NET_CMD_RESPONSE		2


#define	NPREAL_NET_NODE_OPENED			0x01	/* proc node is opened */
#define	NPREAL_NET_NODE_CONNECTED		0x02
#define	NPREAL_NET_NODE_DISCONNECTED	0x04
#define	NPREAL_NET_DO_SESSION_RECOVERY	0x08
#define	NPREAL_NET_DO_INITIALIZE		0x10
#define	NPREAL_NET_TTY_INUSED			0x20	/* tty port is opened */

// ASPP command set

#define	ASPP_NOTIFY 			0x26

#define	ASPP_NOTIFY_PARITY 		0x01
#define	ASPP_NOTIFY_FRAMING 	0x02
#define	ASPP_NOTIFY_HW_OVERRUN 	0x04
#define	ASPP_NOTIFY_SW_OVERRUN 	0x08
#define	ASPP_NOTIFY_BREAK 		0x10
#define	ASPP_NOTIFY_MSR_CHG 	0x20

#define	ASPP_CMD_IOCTL			16
#define	ASPP_CMD_FLOWCTRL		17
#define	ASPP_CMD_LSTATUS		19
#define	ASPP_CMD_LINECTRL		18
#define	ASPP_CMD_FLUSH			20
#define	ASPP_CMD_OQUEUE			22
#define	ASPP_CMD_SETBAUD		23
#define	ASPP_CMD_START_BREAK	33
#define	ASPP_CMD_STOP_BREAK		34
#define	ASPP_CMD_START_NOTIFY	36
#define	ASPP_CMD_STOP_NOTIFY	37
#define	ASPP_CMD_HOST			43
#define	ASPP_CMD_PORT_INIT		44
#define	ASPP_CMD_WAIT_OQUEUE 	47

#define	ASPP_CMD_IQUEUE			21
#define	ASPP_CMD_XONXOFF		24
#define	ASPP_CMD_PORT_RESET		32
#define	ASPP_CMD_RESENT_TIME	46
#define	ASPP_CMD_TX_FIFO		48
#define ASPP_CMD_SETXON     	51
#define ASPP_CMD_SETXOFF    	52

#define	ASPP_FLUSH_RX_BUFFER	0
#define	ASPP_FLUSH_TX_BUFFER	1
#define	ASPP_FLUSH_ALL_BUFFER	2

#define	ASPP_IOCTL_B300			0
#define	ASPP_IOCTL_B600			1
#define	ASPP_IOCTL_B1200		2
#define	ASPP_IOCTL_B2400		3
#define	ASPP_IOCTL_B4800		4
#define	ASPP_IOCTL_B7200		5
#define	ASPP_IOCTL_B9600		6
#define	ASPP_IOCTL_B19200		7
#define	ASPP_IOCTL_B38400		8
#define	ASPP_IOCTL_B57600		9
#define	ASPP_IOCTL_B115200		10
#define	ASPP_IOCTL_B230400		11
#define	ASPP_IOCTL_B460800		12
#define	ASPP_IOCTL_B921600		13
#define	ASPP_IOCTL_B150			14
#define	ASPP_IOCTL_B134			15
#define	ASPP_IOCTL_B110			16
#define	ASPP_IOCTL_B75			17
#define	ASPP_IOCTL_B50			18

#define	ASPP_IOCTL_BITS8		3
#define	ASPP_IOCTL_BITS7		2
#define	ASPP_IOCTL_BITS6		1
#define	ASPP_IOCTL_BITS5		0

#define	ASPP_IOCTL_STOP1		0
#define	ASPP_IOCTL_STOP2		4

#define	ASPP_IOCTL_EVEN			8
#define	ASPP_IOCTL_ODD			16
#define	ASPP_IOCTL_MARK			24
#define	ASPP_IOCTL_SPACE		32
#define	ASPP_IOCTL_NONE			0

struct server_setting_struct
{
	int32_t	server_type;
	int32_t	disable_fifo;
};

struct npreal_struct
{
	int			port;
#if (LINUX_VERSION_CODE >= VERSION_CODE(3,7,0))
	struct tty_port ttyPort;
#endif	
	int			flags;		/* defined in tty.h */
	int			type;		/* UART type */
	struct tty_struct *	tty;
	int			xmit_fifo_size;
	int			custom_divisor;
	unsigned long   baud_base;
	int			x_char; 	/* xon/xoff character */
	int			close_delay;
	unsigned short		closing_wait;
	int			modem_control;	/* Modem control register */
	int			modem_status;	/* Line status */
	unsigned long		event;
	int			count;		/* # of fd on device */
	struct pid*         session;
	struct pid*         pgrp;
	unsigned char		*xmit_buf;
	int			xmit_head;
	int			xmit_tail;
	int			xmit_cnt;
	struct work_struct 	tqueue;
	struct work_struct	process_flip_tqueue;
	struct ktermios		normal_termios;
	struct ktermios		callout_termios;
	wait_queue_head_t 	open_wait;
	wait_queue_head_t 	close_wait;
	wait_queue_head_t 	delta_msr_wait;
	struct async_icount	icount; /* kernel counters for the 4 input interrupts */
	struct nd_struct  	*net_node;


	/* We use spin_lock_irqsave instead of semaphonre here.
       Reason: When we use pppd to dialout via Real TTY driver,
       some driver functions, such as npreal_write(), would be
       invoked under interrpute mode which causes warning in
       down/up tx_semaphore.
	 */
	//  struct semaphore    tx_lock;
	spinlock_t          tx_lock;
	struct semaphore    rx_semaphore;
};

struct nd_struct
{
	int32_t             server_type;
	wait_queue_head_t	initialize_wait;
	wait_queue_head_t	select_in_wait;
	wait_queue_head_t	select_out_wait;
	wait_queue_head_t	select_ex_wait;
	wait_queue_head_t	cmd_rsp_wait;
	int					cmd_rsp_flag;
	int			tx_ready;
	int			rx_ready;
	int			cmd_ready;
	int			wait_oqueue_responsed;
	int			oqueue;
	unsigned char 		cmd_buffer[84];
	unsigned char 		rsp_buffer[84];
	struct semaphore	cmd_semaphore;
	int			rsp_length;
	unsigned long		flag;
	struct proc_dir_entry 	*node_entry;
	struct npreal_struct 	*tty_node;
	struct semaphore	semaphore;
	int			do_session_recovery_len;

};

static struct tty_driver	*npvar_sdriver;

static struct npreal_struct	npvar_table[NPREAL_PORTS];
static struct nd_struct 	npvar_net_nodes[NPREAL_PORTS];
#if (LINUX_VERSION_CODE < VERSION_CODE(3,7,0))
static struct tty_struct *	npvar_tty[NPREAL_PORTS];
static struct ktermios * 	npvar_termios[NPREAL_PORTS];
#endif
#if (LINUX_VERSION_CODE < VERSION_CODE(3,3,0))
static struct ktermios * 	npvar_termios_locked[NPREAL_PORTS];
#endif
static int			npvar_diagflag;
static struct proc_dir_entry *  npvar_proc_root;
/*
 * npvar_tmp_buf is used as a temporary buffer by serial_write. We need
 * to lock it in case the memcpy_fromfs blocks while swapping in a page,
 * and some other program tries to do a serial write at the same time.
 * Since the lock will only come under contention when the system is
 * swapping and available memory is low, it makes sense to share one
 * buffer across all the serial ports, since it significantly saves
 * memory if large numbers of serial ports are open.
 */
static unsigned char *		npvar_tmp_buf;
static struct semaphore 	npvar_tmp_buf_sem;

static struct file_operations npreal_net_fops;

#ifdef MODULE
int		init_module(void);
void	cleanup_module(void);
#endif

static void npreal_disconnect(struct nd_struct *, char *, int *);
int		npreal_init(void);
static int 	npreal_init_tty(void);
static void	npreal_do_softint(struct work_struct *work);
static void npreal_flush_to_ldisc(struct work_struct *work);
static int	npreal_open(struct tty_struct *,struct file *);
static void	npreal_close(struct tty_struct *,struct file *);
static int	npreal_write(struct tty_struct *,const unsigned char *,int);
static int	npreal_write_room(struct tty_struct *);
static void	npreal_flush_buffer(struct tty_struct *);
static void	npreal_ldisc_flush_buffer(struct tty_struct *);
static int	npreal_chars_in_buffer(struct tty_struct *);
static void	npreal_flush_chars(struct tty_struct *);
static int	npreal_put_char(struct tty_struct *,unsigned char);
static int	npreal_ioctl(struct tty_struct *,uint,ulong);
static void	npreal_throttle(struct tty_struct *);
static void	npreal_unthrottle(struct tty_struct *);
static void	npreal_set_termios(struct tty_struct *,struct ktermios *);
static int	npreal_port_init(struct npreal_struct *,struct ktermios *);
static void	npreal_stop(struct tty_struct *);
static void	npreal_start(struct tty_struct *);
static void	npreal_hangup(struct tty_struct *);
static inline void npreal_check_modem_status(struct npreal_struct *,int);
static int	npreal_block_til_ready(struct tty_struct *,struct file *, struct npreal_struct *);
static int	npreal_startup(struct npreal_struct *,struct file *, struct tty_struct *);
static void	npreal_shutdown(struct npreal_struct *);
static int 	npreal_port_shutdown(struct npreal_struct *);
static int	npreal_get_serial_info(struct npreal_struct *, struct serial_struct *);
static int	npreal_set_serial_info(struct npreal_struct *, struct serial_struct *);
static int	npreal_get_lsr_info(struct npreal_struct *,unsigned int *);
static void	npreal_send_break(struct npreal_struct *,int);

static int 	npreal_tiocmget(struct tty_struct *);
static int 	npreal_tiocmset(struct tty_struct *, unsigned int, unsigned int);

static void	npreal_process_notify(struct nd_struct *,char *,int);
static void 	npreal_do_session_recovery(struct npreal_struct *);
static void 	npreal_wait_until_sent(struct tty_struct *,int);
static int 	npreal_wait_and_set_command(struct nd_struct *,char,char);
static int 	npreal_wait_command_completed(struct nd_struct *,char,char, long,char *,int *);
static long 	npreal_wait_oqueue(struct npreal_struct *,long);
static int 	npreal_linectrl(struct nd_struct *nd,int modem_control);
static int 	npreal_break(struct tty_struct * ttyinfo, int break_state);
static void 	npreal_start_break(struct nd_struct *nd);
static void 	npreal_stop_break(struct nd_struct *nd);
static int npreal_setxon_xoff(struct npreal_struct * info, int cmd);

static long _get_delta_giffies(long base);

/*
 *  File operation declarations
 */
static  int npreal_net_open(struct inode *, struct file * );
/* /include/linux/fs.h modification */
static  long npreal_net_ioctl(struct file *,unsigned int, unsigned long );
static int	npreal_net_close(struct inode *,struct file * );
static ssize_t	npreal_net_read (struct file *file,char *buf,size_t count, loff_t *ppos);
static ssize_t	npreal_net_write(struct file *file,const char *buf, size_t count,loff_t *ppos);
static  unsigned int  npreal_net_select(struct file *file, struct poll_table_struct *);
/*
 *  "proc" table manipulation functions
 */

#if (LINUX_VERSION_CODE < VERSION_CODE(3,10,0))
static struct proc_dir_entry *npreal_create_proc_entry(const char *, mode_t, struct proc_dir_entry *);
static void npreal_remove_proc_entry(struct proc_dir_entry *);
#else
static void npreal_remove_proc_entry(struct proc_dir_entry *, int idx);
#endif

static struct tty_operations mpvar_ops =
{
		.open = npreal_open,
		.close = npreal_close,
		.write = npreal_write,
		.put_char = npreal_put_char,
		.flush_chars = npreal_flush_chars,
		.write_room = npreal_write_room,
		.chars_in_buffer = npreal_chars_in_buffer,
		.flush_buffer = npreal_ldisc_flush_buffer,
		.wait_until_sent = npreal_wait_until_sent,
		.break_ctl = npreal_break,
		.ioctl = npreal_ioctl,
		.throttle = npreal_throttle,
		.unthrottle = npreal_unthrottle,
		.set_termios = npreal_set_termios,
		.stop = npreal_stop,
		.start = npreal_start,
		.hangup = npreal_hangup,
		.tiocmget = npreal_tiocmget,
		.tiocmset = npreal_tiocmset,
};

/*
 * The MOXA NPort server Real TTY driver boot-time initialization code!
 */
#ifdef MODULE
static int __init npreal2_module_init(void)
{
	int	ret;

	DBGPRINT(MX_DEBUG_INFO, "Loading module npreal major(%d), coutmajor(%d)...\n", ttymajor, calloutmajor);
	ret = npreal_init();
	DBGPRINT(MX_DEBUG_INFO, "Done.\n");
	return (ret);
}

static void __exit npreal2_module_exit(void)
{
	int i,err = 0;
	struct npreal_struct *info;
	struct proc_dir_entry *de;

	info = &npvar_table[0];
	for (i = 0; i < NPREAL_PORTS; i++,info++)
	{
		if (info->net_node)
		{
#if (LINUX_VERSION_CODE >= VERSION_CODE(3,7,0))
			/* Remove device node if it is opened*/
			if(info->net_node->flag & NPREAL_NET_NODE_OPENED)
				tty_unregister_device(DRV_VAR, i);
#endif        
			if ((de=((struct nd_struct *)(info->net_node))->node_entry))
#if (LINUX_VERSION_CODE < VERSION_CODE(3,10,0))
				npreal_remove_proc_entry(de);
#else
			npreal_remove_proc_entry(de, i);
#endif
			((struct nd_struct *)(info->net_node))->node_entry = NULL;
		}
	}
	if (npvar_proc_root)
	{
#if (LINUX_VERSION_CODE < VERSION_CODE(3,10,0))
		npreal_remove_proc_entry( npvar_proc_root);
#else
		npreal_remove_proc_entry( npvar_proc_root, 404);
#endif
		npvar_proc_root = NULL;
	}

	DBGPRINT(MX_DEBUG_INFO, "Unloading module npreal ...\n");
	if ((err |= tty_unregister_driver(DRV_VAR)))
	{
		DBGPRINT(MX_DEBUG_ERROR, "Couldn't unregister MOXA Async/NPort server family Real TTY driver\n");
	}
	put_tty_driver(DRV_VAR);

	DBGPRINT(MX_DEBUG_INFO, "Done.\n");

}
#endif

static int
npreal_init_tty(void)
{
	struct npreal_struct *tty_node;
	int	i;
	struct proc_dir_entry *de;
	struct nd_struct *net_node;
	char	buf[4];

	init_MUTEX(&npvar_tmp_buf_sem);
	//create "npreal2" dir
	npvar_proc_root = proc_mkdir("npreal2", NULL);
	//npvar_proc_root = npreal_create_proc_entry( "npreal2",S_IFDIR, &proc_root);
	if ( !npvar_proc_root  )
		return -ENOMEM;
	tty_node = &npvar_table[0];
	net_node = &npvar_net_nodes[0];

	for ( i=0; i<NPREAL_PORTS; i++, tty_node++,net_node++ )
	{
		sprintf(buf,"%d",i);

#if (LINUX_VERSION_CODE < VERSION_CODE(3,10,0))
		de = npreal_create_proc_entry(buf, S_IRUGO | S_IWUGO | S_IFREG,
				npvar_proc_root);
		if ( !de )
			return -ENOMEM;

		de->data = (void *) net_node;
		de->proc_fops = &npreal_net_fops;
		net_node->tty_node = tty_node;
		net_node->node_entry = de;
		net_node->flag = 0;
#else
		de = proc_create_data(buf, S_IRUGO | S_IWUGO | S_IFREG,
				npvar_proc_root, &npreal_net_fops, (void *) net_node);
		if ( !de )
			return -ENOMEM;
		//	PDE_DATA(de) = (void *) net_node;
		net_node->tty_node = tty_node;
		net_node->node_entry = de;
		net_node->flag = 0;
#endif /* 3,10, 0 */
		init_MUTEX(&net_node->semaphore);
		init_MUTEX(&net_node->cmd_semaphore);
		init_waitqueue_head(&net_node->initialize_wait);
		init_waitqueue_head(&net_node->select_in_wait);
		init_waitqueue_head(&net_node->select_out_wait);
		init_waitqueue_head(&net_node->select_ex_wait);
		init_waitqueue_head(&net_node->cmd_rsp_wait);
		net_node->cmd_rsp_flag = 0;

		tty_node->net_node = net_node;
		tty_node->port = i;
		tty_node->type = PORT_16550A;
		tty_node->flags = 0;
		tty_node->xmit_fifo_size = 16;
		tty_node->baud_base = 921600L;
		tty_node->close_delay = 5*HZ/10;
		tty_node->closing_wait = 30*HZ;
		INIT_WORK(&tty_node->tqueue, npreal_do_softint);
		INIT_WORK(&tty_node->process_flip_tqueue, npreal_flush_to_ldisc);
		tty_node->normal_termios = DRV_VAR_P(init_termios);
		//init_MUTEX(&tty_node->tx_lock);
		//tty_node->tx_lock = __SPIN_LOCK_UNLOCKED(tty_node->tx_lock);
		spin_lock_init(&tty_node->tx_lock);
		init_MUTEX(&tty_node->rx_semaphore);
		init_waitqueue_head(&tty_node->open_wait);
		init_waitqueue_head(&tty_node->close_wait);
		init_waitqueue_head(&tty_node->delta_msr_wait);
		tty_node->icount.rx = tty_node->icount.tx = 0;
		tty_node->icount.cts = tty_node->icount.dsr = tty_node->icount.dcd = 0;
		tty_node->icount.frame = tty_node->icount.overrun =
		tty_node->icount.brk = tty_node->icount.parity = 0;
	}
	return 0;
}

int
npreal_init(void)
{
	int	ret1, ret2;
#if (LINUX_VERSION_CODE >= VERSION_CODE(3,7,0))
	int i;
#endif

	npvar_sdriver = alloc_tty_driver(NPREAL_PORTS+1);
	if (!npvar_sdriver)
		return -ENOMEM;
	printk("MOXA Async/NPort server family Real TTY driver ttymajor %d calloutmajor %d verbose %d (%s)\n", ttymajor, calloutmajor, verbose, NPREAL_VERSION);

	/* Initialize the tty_driver structure */
	DRV_VAR_P(name) = "ttyr";
	DRV_VAR_P(major) = ttymajor;
	DRV_VAR_P(minor_start) = 0;
	DRV_VAR_P(type) = TTY_DRIVER_TYPE_SERIAL;
	DRV_VAR_P(subtype) = SERIAL_TYPE_NORMAL;
	DRV_VAR_P(init_termios) = tty_std_termios;
	DRV_VAR_P(init_termios.c_cflag) = B9600|CS8|CREAD|HUPCL|CLOCAL;
	DRV_VAR_P(flags) = TTY_DRIVER_REAL_RAW | TTY_DRIVER_DYNAMIC_DEV;

	tty_set_operations(DRV_VAR, &mpvar_ops);
#if (LINUX_VERSION_CODE < VERSION_CODE(3,7,0))
	DRV_VAR_P(ttys) = npvar_tty;
	DRV_VAR_P(termios) = npvar_termios;
#endif
#if (LINUX_VERSION_CODE < VERSION_CODE(3,3,0))
	DRV_VAR_P(termios_locked) = npvar_termios_locked;
#endif
	DBGPRINT(MX_DEBUG_INFO, "Tty devices major number = %d, callout devices major number = %d\n",ttymajor,calloutmajor);

	npvar_diagflag = 0;
	memset(npvar_table, 0, NPREAL_PORTS * sizeof(struct npreal_struct));

#if (LINUX_VERSION_CODE >= VERSION_CODE(3,7,0))
	for(i = 0; i < NPREAL_PORTS;i++)
	{
		tty_port_init(&npvar_table[i].ttyPort);
		tty_port_link_device(&npvar_table[i].ttyPort, npvar_sdriver, i);
	}
#endif


/*register driver*/
	ret1 = 0;
	ret2 = 0;
	if ( !(ret1=tty_register_driver(DRV_VAR)) )
	{
	}
	else
	{
		DBGPRINT(MX_DEBUG_ERROR, "Couldn't install MOXA Async/NPort server family driver !\n");
	}

	if (ret1 || ret2)
	{
		put_tty_driver(DRV_VAR);
		return -1;
	}

	/* Initialize the net node structure */
	memset(&npreal_net_fops,0,sizeof(struct file_operations));
	npreal_net_fops.read = npreal_net_read;
	npreal_net_fops.write = npreal_net_write;
	npreal_net_fops.unlocked_ioctl = npreal_net_ioctl;
	npreal_net_fops.open = npreal_net_open;
	npreal_net_fops.release = npreal_net_close;
	npreal_net_fops.poll = npreal_net_select;
	if (npreal_init_tty() != 0)
	{
		tty_unregister_driver(DRV_VAR);
		DBGPRINT(MX_DEBUG_ERROR, "Couldn't install MOXA Async/NPort server family Real TTY driver !\n");
	}
	return(0);
}

static void npreal_disconnect(struct nd_struct *nd, char *buf, int *size)
{
	nd->cmd_buffer[0] = 0;
	npreal_wait_and_set_command(nd,
			NPREAL_LOCAL_COMMAND_SET,
			LOCAL_CMD_TTY_UNUSED);
	nd->cmd_buffer[2] = 0;
	nd->cmd_ready = 1;
	if (waitqueue_active(&nd->select_ex_wait)) {
		wake_up_interruptible( &nd->select_ex_wait );
	}
	if (npreal_wait_command_completed(nd,
			NPREAL_LOCAL_COMMAND_SET,
			LOCAL_CMD_TTY_UNUSED,
			NPREAL_CMD_TIMEOUT,
			buf,
			size) != 0) {
		npreal_wait_and_set_command(nd,
				NPREAL_LOCAL_COMMAND_SET,
				LOCAL_CMD_TTY_UNUSED);
		nd->cmd_buffer[2] = 0;
		nd->cmd_ready = 1;
		if (waitqueue_active(&nd->select_ex_wait)) {
			wake_up_interruptible( &nd->select_ex_wait );
		}
		npreal_wait_command_completed(nd,
				NPREAL_LOCAL_COMMAND_SET,
				LOCAL_CMD_TTY_UNUSED,
				NPREAL_CMD_TIMEOUT,
				buf,
				size);
	}

}

static void
npreal_do_softint(struct work_struct *work)
{
	struct npreal_struct *	info =
			container_of(work, struct npreal_struct, tqueue);

	struct tty_struct *	tty;

	if (!info)
		goto done;

	tty = info->tty;
	if (tty)
	{
		if ( test_and_clear_bit(NPREAL_EVENT_TXLOW, &info->event) )
		{
			if ( (tty->flags & (1 << TTY_DO_WRITE_WAKEUP)) &&
					tty->ldisc->ops->write_wakeup)
				(tty->ldisc->ops->write_wakeup)(tty);
			wake_up_interruptible(&tty->write_wait);
		}
		if ( test_and_clear_bit(NPREAL_EVENT_HANGUP, &info->event) )
		{
			// Scott: 2005-09-05
			// Do it when entering npreal_hangup().
			// Scott info->flags |= ASYNC_CLOSING;
			tty_hangup(tty);
		}
	}

	done:
	;
}

/*
 * This routine is called whenever a serial port is opened.
 */

/* Scott: 2005/07/13
 * Note that on failure, we don't decrement the module use count - the tty driver
 * later will call npreal_close, which will decrement it for us as long as
 * tty->driver_data is set non-NULL.
 */
static int
npreal_open(
		struct tty_struct * tty,
		struct file * filp)
{
	struct npreal_struct 	*info;
	int			line;
	unsigned long		page;
	struct nd_struct  	*nd;

	DBGPRINT(MX_DEBUG_LOUD, "(Entry)\n");

	MX_MOD_INC;
	line = PORTNO(tty);

	if ( (line < 0) || (line >= NPREAL_PORTS) )
	{
		DBGPRINT(MX_DEBUG_ERROR, "invalid line (%d)\n", line);
		MX_MOD_DEC;
		return(-ENODEV);
	}
	info = npvar_table + line;

	nd = info->net_node;
	if ( !nd  || !(nd->flag&NPREAL_NET_NODE_OPENED))
	{
		DBGPRINT(MX_DEBUG_ERROR, "net device not ready\n");
		MX_MOD_DEC;
		return(-ENODEV);
	}


	if ( !npvar_tmp_buf )
	{
		page = GET_FPAGE(GFP_KERNEL);
		if ( !page )
		{
			DBGPRINT(MX_DEBUG_ERROR, "allocate npvar_tmp_buf failed\n");
			MX_MOD_DEC;
			return(-ENOMEM);
		}
		if ( npvar_tmp_buf )
			free_page(page);
		else
			npvar_tmp_buf = (unsigned char *)page;
	}

	/*
	 * Start up serial port
	 */
	// Scott: 2005/07/13
	// Set tty->driver_data before entering npreal_startup(), so that the tty driver
	// can decrease refcount if npreal_startup() failed, by calling npreal_close().
	tty->driver_data = info;
	info->count++;
	// Scott: end

	if (npreal_startup(info,filp,tty))
	{
		DBGPRINT(MX_DEBUG_ERROR, "npreal_startup failed\n");
		return(-EIO);
	}
	if (npreal_block_til_ready(tty, filp, info))
	{
		DBGPRINT(MX_DEBUG_ERROR, "npreal_block_til_ready failed\n");
		return(-EIO);
	}

	if ( (info->count == 1) && (info->flags & ASYNC_SPLIT_TERMIOS) )
	{
#if (LINUX_VERSION_CODE < VERSION_CODE(3,7,0))
		if ( MX_TTY_DRV(subtype) == SERIAL_TYPE_NORMAL )
			*tty->termios = info->normal_termios;
		else
			*tty->termios = info->callout_termios;
#else
		if ( MX_TTY_DRV(subtype) == SERIAL_TYPE_NORMAL )
			tty->termios = info->normal_termios;
else
		tty->termios = info->callout_termios;
#endif

		if (npreal_port_init(info, 0))
		{
			DBGPRINT(MX_DEBUG_ERROR, "npreal_port_init failed\n");
			return(-EIO);
		}
	}

	info->session = MX_SESSION();
	info->pgrp = MX_CGRP();

#if (LINUX_VERSION_CODE < VERSION_CODE(3,9,0))
#if (LINUX_VERSION_CODE < VERSION_CODE(3,2,0))
	/* It must be always on */
	tty->low_latency = 1;
#else
	tty->low_latency = 1; /* Read Issue */
#endif /* 3,2,0 */
#else
	info->ttyPort.low_latency = 1;
#endif /* 3,9,0 */
	return(0);
}

/*
 * This routine is called when the serial port gets closed.  First, we
 * wait for the last remaining data to be sent.
 */
static void
npreal_close(
		struct tty_struct * tty,
		struct file * filp)
{
	struct npreal_struct * info = (struct npreal_struct *)tty->driver_data;
	long	timeout,et,ret;
	int	cnt;

	DBGPRINT(MX_DEBUG_LOUD, "(Entry)\n");

	if ( !info ) {
		return;
	}
	if ( tty_hung_up_p(filp) )
	{
		info->count--;
		MX_MOD_DEC;
		return;
	}
	// Scott: 2005/07/13
	// Comment out the following two if's.
#if 0
	if (!(info->flags & ASYNC_INITIALIZED))
	{
		info->count--;
		MX_MOD_DEC;
		return;
	}
	if (info->flags & ASYNC_CLOSING)
	{
		info->count--;
		MX_MOD_DEC;
		return;
	}
#endif

#ifndef SP1
	if ( (tty->count == 1) && (info->count != 1) )
	{
#else
	if ( (atomic_read(&tty->count) == 1) && (info->count != 1) )
	{
#endif
		/*
		 * Uh, oh.	tty->count is 1, which means that the tty
		 * structure will be freed.  Info->count should always
		 * be one in these conditions.  If it's greater than
		 * one, we've got real problems, since it means the
		 * serial port won't be shutdown.
		 */
		DBGPRINT(MX_DEBUG_WARN, "[%d] npreal_close: bad serial port count; tty->count is 1, info->count is %d\n", current->pid, info->count);
		info->count = 1;
	}
	if ( --info->count < 0 )
	{
		DBGPRINT(MX_DEBUG_WARN, "npreal_close: bad serial port count for port %d: %d\n", info->port, info->count);
		info->count = 0;
	}
	if ( info->count )
	{
		MX_MOD_DEC;
		return;
	}

	// Scott: 2005-09-05
	// Prevent race condition on closing.
	if (info->flags & ASYNC_CLOSING) {
		return;
	}

	info->flags |= ASYNC_CLOSING;
	tty->closing = 1;
	/*
	 * Save the termios structure, since this port may have
	 * separate termios for callout and dialin.
	 */
#if (LINUX_VERSION_CODE < VERSION_CODE(3,7,0))
	if ( info->flags & ASYNC_NORMAL_ACTIVE )
		info->normal_termios = *tty->termios;
	if ( info->flags & ASYNC_CALLOUT_ACTIVE )
		info->callout_termios = *tty->termios;
#else
	if ( info->flags & ASYNC_NORMAL_ACTIVE )
		info->normal_termios = tty->termios;
	if ( info->flags & ASYNC_CALLOUT_ACTIVE )
		info->callout_termios = tty->termios;
#endif
	/*
	 * Now we wait for the transmit buffer to clear; and we notify
	 * the line discipline to only process XON/XOFF characters.
	 */
	// Scott: 2005-07-08
	// If the open mode is nonblocking mode, don't block on close.
	if ( !(filp->f_flags & O_NONBLOCK) && info->closing_wait != ASYNC_CLOSING_WAIT_NONE )
	{
		//	if ( info->closing_wait != ASYNC_CLOSING_WAIT_NONE ) {
		et = jiffies + info->closing_wait;
		tty_wait_until_sent(tty, info->closing_wait);
		cnt = 0;
		while ((timeout = et - jiffies) > 0)
		{
			if ((ret=npreal_wait_oqueue(info,timeout)) == 0)
			{
				if (++cnt >= 3)
				{
					break;
				}
			}
			else if (ret < 0)
				break;
			else
				cnt = 0;
			current->state = TASK_INTERRUPTIBLE;
			schedule_timeout(HZ/100);
		}
	}
	npreal_flush_buffer(tty);
	if ( tty->ldisc->ops->flush_buffer )
		tty->ldisc->ops->flush_buffer(tty);
	npreal_shutdown(info);
	tty->closing = 0;
	MX_MOD_DEC;
}

/*
 * copy data form AP-layer to kernel-layer,
 * and wake up net_read to read data to proc file system. (files in this folder /proc/npreal2)
 */
static int npreal_write(struct tty_struct * tty,
		const unsigned char * buf, int count)
{
	int c, total = 0, ret ;
	struct npreal_struct *info = (struct npreal_struct *)tty->driver_data;
	struct nd_struct  	*nd;
	unsigned long        flags;

	int from_user = 0;

	DBGPRINT(MX_DEBUG_LOUD, "(Entry)\n");

	// Scott: 2005-09-12
	if (!info)
		return 0;

	if ( !tty || !info->xmit_buf || !npvar_tmp_buf )
		return(0);

	nd = info->net_node;

	if (!nd)
		return 0;

	if ( from_user )
		down(&npvar_tmp_buf_sem);

	while ( 1 )
	{
		c = MIN(count, MIN(SERIAL_XMIT_SIZE - info->xmit_cnt - 1,
				SERIAL_XMIT_SIZE - info->xmit_head));

		if ( c <= 0 )
			break;

		if ( from_user )
		{
			ret = copy_from_user(npvar_tmp_buf, buf, c);
			if (!ret)
				memcpy(info->xmit_buf + info->xmit_head, npvar_tmp_buf, c);
		}
		else
			memcpy(info->xmit_buf + info->xmit_head, buf, c);

		DOWN(info->tx_lock, flags);
		info->xmit_head = (info->xmit_head + c) & (SERIAL_XMIT_SIZE - 1);
		info->xmit_cnt += c;
		UP(info->tx_lock, flags);

		buf += c;
		count -= c;
		total += c;
	}

	if (info->xmit_cnt )
	{
		nd->tx_ready = 1;
		if ( waitqueue_active(&nd->select_in_wait))
			wake_up_interruptible( &nd->select_in_wait );
	}

	if ( from_user )
		up(&npvar_tmp_buf_sem);

	return(total);
}
static int	npreal_put_char(struct tty_struct * tty, unsigned char ch)
{
	struct npreal_struct *info = (struct npreal_struct *)tty->driver_data;
	struct nd_struct  	*nd;
	unsigned long        flags;

	DBGPRINT(MX_DEBUG_LOUD, "(Entry)\n");

	// Scott: 2005-09-12
	if (!info)
		return 0;

	if ( !tty || !info->xmit_buf )
		return 0;

	//down(&info->tx_semaphore);
	DOWN(info->tx_lock, flags);

	// Scott: 2005-09-12
	if (!info->xmit_buf)
	{
		UP(info->tx_lock, flags);
		return 0;
	}

	if ( info->xmit_cnt >= SERIAL_XMIT_SIZE - 1 )
	{
		//up(&info->tx_semaphore);
		UP(info->tx_lock, flags);
		return 0;
	}

	nd = info->net_node;
	if (!nd)
	{
		UP(info->tx_lock, flags);
		return 0;
	}

	info->xmit_buf[info->xmit_head++] = ch;
	info->xmit_head &= SERIAL_XMIT_SIZE - 1;
	info->xmit_cnt++;

	nd->tx_ready = 1;
	if ( waitqueue_active(&nd->select_in_wait))
		wake_up_interruptible( &nd->select_in_wait );

	//up(&info->tx_semaphore);
	UP(info->tx_lock, flags);
	return 1;
}

static void npreal_flush_chars(struct tty_struct * tty)
{
	DBGPRINT(MX_DEBUG_LOUD, "(Entry)\n");
}

static void npreal_wait_until_sent(struct tty_struct * tty,int timeout)
{

	struct npreal_struct *info;

	DBGPRINT(MX_DEBUG_LOUD, "(Entry), timeout: %d\n", timeout);

	if ((info = (struct npreal_struct *)tty->driver_data))
		npreal_wait_oqueue(info,timeout);

}

static int npreal_write_room(struct tty_struct * tty)
{
	struct npreal_struct *info = (struct npreal_struct *)tty->driver_data;
	int	ret;

	DBGPRINT(MX_DEBUG_LOUD, "(Entry)\n");

	// Scott: 2005-09-12
	if (!info)
		return 0;

	ret = SERIAL_XMIT_SIZE - info->xmit_cnt - 1;
	if ( ret < 0 )
		ret = 0;
	return(ret);
}

static int npreal_chars_in_buffer(struct tty_struct * tty)
{
	struct npreal_struct *info = (struct npreal_struct *)tty->driver_data;

	DBGPRINT(MX_DEBUG_LOUD, "(Entry)\n");

	if (!info)
		return (-EIO);
	DBGPRINT(MX_DEBUG_LOUD, "len: %d\n", info->xmit_cnt);
	return(info->xmit_cnt);

}


static void npreal_flush_buffer(struct tty_struct * tty)
{
	struct npreal_struct *info = (struct npreal_struct *)tty->driver_data;
	struct nd_struct	*nd;
	char rsp_buffer[8];
	int  rsp_length = sizeof(rsp_buffer);
	unsigned long   flags;

	if (!info)
		return;

	//down(&info->tx_semaphore);
	DOWN(info->tx_lock, flags);
	info->xmit_cnt = info->xmit_head = info->xmit_tail = 0;
	//up(&info->tx_semaphore);
	UP(info->tx_lock, flags);

	wake_up_interruptible(&tty->write_wait);
	if ( (tty->flags & (1 << TTY_DO_WRITE_WAKEUP)) &&
			tty->ldisc->ops->write_wakeup )
		(tty->ldisc->ops->write_wakeup)(tty);
	if (!(nd=info->net_node))
		return;
	nd->tx_ready = 0;
	if (npreal_wait_and_set_command(nd,NPREAL_ASPP_COMMAND_SET,ASPP_CMD_FLUSH) < 0)
		return;
	nd->cmd_buffer[2] = 1;
	nd->cmd_buffer[3] = ASPP_FLUSH_ALL_BUFFER;
	nd->cmd_ready = 1;
	if ( waitqueue_active(&nd->select_ex_wait))
	{
		wake_up_interruptible( &nd->select_ex_wait );
	}
	npreal_wait_command_completed(nd,
			NPREAL_ASPP_COMMAND_SET,
			ASPP_CMD_FLUSH,NPREAL_CMD_TIMEOUT,
			rsp_buffer,
			&rsp_length);
}

/* This function may be call from dispatch function,in that case we
will unable to complete the NPPI function in single CPU platform,
so we just fire the command ASAP and don't wait its completion */

static void npreal_ldisc_flush_buffer(struct tty_struct * tty)
{
	struct npreal_struct *info = (struct npreal_struct *)tty->driver_data;
	struct nd_struct	*nd;
	unsigned long   flags;

	DBGPRINT(MX_DEBUG_LOUD, "(Entry)\n");

	if (!info)
		return;

	//down(&info->tx_semaphore);
	DOWN(info->tx_lock, flags);
	info->xmit_cnt = info->xmit_head = info->xmit_tail = 0;
	//up(&info->tx_semaphore);
	UP(info->tx_lock, flags);

	wake_up_interruptible(&tty->write_wait);
	if ( (tty->flags & (1 << TTY_DO_WRITE_WAKEUP)) &&
			tty->ldisc->ops->write_wakeup )
		(tty->ldisc->ops->write_wakeup)(tty);
	if (!(nd=info->net_node))
		return;
	nd->tx_ready = 0;
	if (nd->flag & NPREAL_NET_DO_SESSION_RECOVERY)
		return;
	nd->cmd_buffer[0] = NPREAL_ASPP_COMMAND_SET;
	nd->cmd_buffer[1] = ASPP_CMD_FLUSH;
	nd->cmd_buffer[2] = 1;
	nd->cmd_buffer[3] = ASPP_FLUSH_ALL_BUFFER;
	nd->cmd_ready = 1;
	if ( waitqueue_active(&nd->select_ex_wait))
	{
		wake_up_interruptible( &nd->select_ex_wait );
	}
	current->state = TASK_INTERRUPTIBLE;
	schedule_timeout(HZ/100);
}

static int npreal_ioctl(struct tty_struct * tty,
		unsigned int cmd, unsigned long arg)
{
	int			error;
	struct npreal_struct *	info = (struct npreal_struct *)tty->driver_data;
	int			retval;
	struct async_icount	cprev, cnow;	    /* kernel counter temps */
	struct serial_icounter_struct *p_cuser;     /* user space */
	unsigned long 		templ;

	DBGPRINT(MX_DEBUG_LOUD, "(Entry) cmd: 0x%X\n", cmd);

	// Scott: 2005-09-12
	if (!info)
		return -ENODEV;

	if ( (cmd != TIOCGSERIAL) && (cmd != TIOCMIWAIT) &&
			(cmd != TIOCGICOUNT) )
	{
		if ( tty->flags & (1 << TTY_IO_ERROR) )
		{
			return(-EIO);
		}
	}
	switch ( cmd )
	{
	case TCFLSH:
		retval = tty_check_change(tty);
		if (retval)
		{
			return retval;
		}


		switch (arg)
		{
		case TCIFLUSH:
			if (tty->ldisc->ops->flush_buffer)
				tty->ldisc->ops->flush_buffer(tty);
			break;
		case TCIOFLUSH:
			if (tty->ldisc->ops->flush_buffer)
				tty->ldisc->ops->flush_buffer(tty);
			/* fall through */
		case TCOFLUSH:
			npreal_flush_buffer(tty);
			break;
		default:
			return -EINVAL;
		}
		return 0;

	case TCSBRK:	/* SVID version: non-zero arg --> no break */
		retval = tty_check_change(tty);
		if ( retval )
			return(retval);
		tty_wait_until_sent(tty, 0);
		if ( !arg )
			npreal_send_break(info, HZ/4);		/* 1/4 second */
		return(0);

	case TCSBRKP:	/* support for POSIX tcsendbreak() */
		retval = tty_check_change(tty);
		if ( retval )
			return(retval);
		tty_wait_until_sent(tty, 0);
		npreal_send_break(info, arg ? arg*(HZ/10) : HZ/4);
		return(0);

	case TIOCGSOFTCAR:
#if (LINUX_VERSION_CODE >= VERSION_CODE(5,0,0))    
		error = access_ok((void *)arg, sizeof(long))?0:-EFAULT;
#else
		error = access_ok(VERIFY_WRITE, (void *)arg, sizeof(long))?0:-EFAULT;
#endif
		if ( error )
			return(error);
		put_to_user(C_CLOCAL(tty) ? 1 : 0, (unsigned long *)arg);
		return 0;

	case TIOCSSOFTCAR:
#if (LINUX_VERSION_CODE >= VERSION_CODE(5,0,0))    
		error = access_ok((void *)arg, sizeof(long))?0:-EFAULT;
#else
		error = access_ok(VERIFY_READ, (void *)arg, sizeof(long))?0:-EFAULT;
#endif
		if ( error )
			return(error);
		get_from_user(templ,(unsigned long *)arg);
		arg = templ;
#if (LINUX_VERSION_CODE < VERSION_CODE(3,7,0))		
		tty->termios->c_cflag = ((tty->termios->c_cflag & ~CLOCAL) |
				(arg ? CLOCAL : 0));
#else
		tty->termios.c_cflag = ((tty->termios.c_cflag & ~CLOCAL) |
				(arg ? CLOCAL : 0));
#endif
		return(0);

	case TIOCGSERIAL:
#if (LINUX_VERSION_CODE >= VERSION_CODE(5,0,0))    
		error = access_ok((void *)arg,
				sizeof(struct serial_struct))?0:-EFAULT;
#else
		error = access_ok(VERIFY_WRITE, (void *)arg,
				sizeof(struct serial_struct))?0:-EFAULT;
#endif
		if ( error )
			return(error);
		return(npreal_get_serial_info(info, (struct serial_struct *)arg));

	case TIOCSSERIAL:
#if (LINUX_VERSION_CODE >= VERSION_CODE(5,0,0))    
		error = access_ok((void *)arg,
				sizeof(struct serial_struct))?0:-EFAULT;
#else
		error = access_ok(VERIFY_READ, (void *)arg,
				sizeof(struct serial_struct))?0:-EFAULT;
#endif
		if ( error )
			return(error);
		return(npreal_set_serial_info(info, (struct serial_struct *)arg));

	case TIOCSERGETLSR: /* Get line status register */
#if (LINUX_VERSION_CODE >= VERSION_CODE(5,0,0))    
		error = access_ok((void *)arg,
				sizeof(unsigned int))?0:-EFAULT;
#else
		error = access_ok(VERIFY_WRITE, (void *)arg,
				sizeof(unsigned int))?0:-EFAULT;
#endif
		if ( error )
			return(error);
		else
			return(npreal_get_lsr_info(info, (unsigned int *)arg));

	/*
	 * Wait for any of the 4 modem inputs (DCD,RI,DSR,CTS) to change
	 * - mask passed in arg for lines of interest
	 *   (use |'ed TIOCM_RNG/DSR/CD/CTS for masking)
	 * Caller should use TIOCGICOUNT to see which one it was
	 */
	case TIOCMIWAIT:
	{
		DECLARE_WAITQUEUE(wait, current);
		int ret;
		cprev = info->icount;   /* note the counters on entry */
		add_wait_queue(&info->delta_msr_wait, &wait);
		while ( 1 )
		{
			/* see if a signal did it */
			cnow = info->icount;	/* atomic copy */
			set_current_state(TASK_INTERRUPTIBLE);
			if ( ((arg & TIOCM_RNG) && (cnow.rng != cprev.rng)) ||
					((arg & TIOCM_DSR) && (cnow.dsr != cprev.dsr)) ||
					((arg & TIOCM_CD)	&& (cnow.dcd != cprev.dcd)) ||
					((arg & TIOCM_CTS) && (cnow.cts != cprev.cts)) )
			{
				ret = 0;
				break;
			}
			if ( signal_pending(current) )
			{
				ret = -ERESTARTSYS;
				break;
			}
			cprev = cnow;
			// Scott: 2005-09-04 add begin
			schedule();
			// Scott: 2005-09-04 add end
		}
		current->state = TASK_RUNNING;
		remove_wait_queue(&info->delta_msr_wait, &wait);
		// Scott: 2005-09-04
		// Scott break;
		return ret;
	}

	/* NOTREACHED */
	/*
	 * Get counter of input serial line interrupts (DCD,RI,DSR,CTS)
	 * Return: write counters to the user passed counter struct
	 * NB: both 1->0 and 0->1 transitions are counted except for
	 *     RI where only 0->1 is counted.
	 */
	case TIOCGICOUNT:
#if (LINUX_VERSION_CODE >= VERSION_CODE(5,0,0))    
		error = access_ok((void *)arg,
				sizeof(struct serial_icounter_struct))?0:-EFAULT;
#else
		error = access_ok(VERIFY_WRITE, (void *)arg,
				sizeof(struct serial_icounter_struct))?0:-EFAULT;
#endif
		if ( error )
			return(error);
		cnow = info->icount;
		p_cuser = (struct serial_icounter_struct *)arg;
		/* modified by casper 1/11/2000 */
		if (put_user(cnow.frame, &p_cuser->frame))
			return -EFAULT;
		if (put_user(cnow.brk, &p_cuser->brk))
			return -EFAULT;
		if (put_user(cnow.overrun, &p_cuser->overrun))
			return -EFAULT;
		if (put_user(cnow.buf_overrun, &p_cuser->buf_overrun))
			return -EFAULT;
		if (put_user(cnow.parity, &p_cuser->parity))
			return -EFAULT;
		if (put_user(cnow.rx, &p_cuser->rx))
			return -EFAULT;
		if (put_user(cnow.tx, &p_cuser->tx))
			return -EFAULT;

		put_to_user(cnow.cts, &p_cuser->cts);
		put_to_user(cnow.dsr, &p_cuser->dsr);
		put_to_user(cnow.rng, &p_cuser->rng);
		put_to_user(cnow.dcd, &p_cuser->dcd);

		/* */
		return(0);

	case TCXONC:
		DBGPRINT(MX_DEBUG_LOUD, "(DBG)\n");
		retval = tty_check_change(tty);
		if ( retval ){
			DBGPRINT(MX_DEBUG_LOUD, "(DBG)\n");
			return(retval);
		}
		switch (arg)
		{
		case TCOOFF:
			DBGPRINT(MX_DEBUG_LOUD, "(DBG)\n");
			return npreal_setxon_xoff(info, ASPP_CMD_SETXOFF);
		case TCOON:
			DBGPRINT(MX_DEBUG_LOUD, "(DBG)\n");
			return npreal_setxon_xoff(info, ASPP_CMD_SETXON);
			/* fall through */
		default:
			DBGPRINT(MX_DEBUG_LOUD, "(DBG)\n");
			return -EINVAL;
		}
		return 0;
	default:
		return(-ENOIOCTLCMD);
	}

	return(0);
}

/*
 * This routine is called by the upper-layer tty layer to signal that
 * incoming characters should be throttled.
 * (tty driver buffer is full)
 */
static void npreal_throttle(struct tty_struct * tty)
{
	struct npreal_struct *info = (struct npreal_struct *)tty->driver_data;
	struct nd_struct *nd;

	DBGPRINT(MX_DEBUG_LOUD, "(Entry)\n");

	// Scott: 2005-09-12

	if (!info)
		return;

	nd = info->net_node;
	if (!nd)
		return;
	nd->rx_ready = 0;
}
/*
 * tty driver buffer have space to receiver data,
 * wake up net select_out_wait(net).
 */
static void npreal_unthrottle(struct tty_struct * tty)
{
	struct npreal_struct *info = (struct npreal_struct *)tty->driver_data;
	struct nd_struct *nd;

	DBGPRINT(MX_DEBUG_LOUD, "(Entry)\n");

	if (!info)
		return;
	nd = info->net_node;
	if (!nd)
		return;
	nd->rx_ready = 1;
	if ( waitqueue_active(&nd->select_out_wait))
		wake_up_interruptible( &nd->select_out_wait );
}

/*
 * This routine allows the tty driver to be notified when device's termios settings have changed.
 *
 * Note : that a well-designed tty driver should be prepared to accept the case
 * where old == NULL, and try to do something rational.
 */
static void npreal_set_termios(struct tty_struct * tty,
		struct ktermios * old_termios)
{
	struct npreal_struct *info = (struct npreal_struct *)tty->driver_data;

	DBGPRINT(MX_DEBUG_LOUD, "(Entry)\n");

	npreal_port_init(info, old_termios);
}

/*
 * npreal_stop() and npreal_start()
 *
 * This routines are called before setting or resetting tty->stopped.
 * They enable or disable transmitter interrupts, as necessary.
 */
static void npreal_stop(struct tty_struct * tty)
{
	DBGPRINT(MX_DEBUG_LOUD, "(Entry)\n");
}

static void npreal_start(struct tty_struct * tty)
{
	DBGPRINT(MX_DEBUG_LOUD, "(Entry)\n");
}

static void npreal_hangup(struct tty_struct *tty)
{
	struct npreal_struct *info = (struct npreal_struct *)tty->driver_data;

	DBGPRINT(MX_DEBUG_LOUD, "(Entry)\n");

	if (!info)
		return;

	// Scott: 2005-09-05
	// Prevent race condition on closing.
	if (info->flags & ASYNC_CLOSING)
		return;

	info->flags |=  ASYNC_CLOSING;
	//
	// do_tty_hangup() already do this
	//	npreal_flush_buffer(tty);
	npreal_shutdown(info);
}

static inline void npreal_check_modem_status(struct npreal_struct *info,
		int status)
{
	int	is_dcd_changed = 0;

	/* update input line counters */
	if ( (info->modem_status & UART_MSR_DSR) != (status & UART_MSR_DSR ))
		info->icount.dsr++;
	if ( (info->modem_status & UART_MSR_DCD) != (status & UART_MSR_DCD ))
	{
		info->icount.dcd++;
		is_dcd_changed = 1;
	}
	if ( (info->modem_status & UART_MSR_CTS) != (status & UART_MSR_CTS ))
		info->icount.cts++;
	info->modem_status = status;
	wake_up_interruptible(&info->delta_msr_wait);

	if ( (info->flags & ASYNC_CHECK_CD) && (is_dcd_changed))
	{

		if ( status & UART_MSR_DCD )
		{
			wake_up_interruptible(&info->open_wait);
		}
		else
		{
			set_bit(NPREAL_EVENT_HANGUP,&info->event);
			MXQ_TASK(&info->tqueue);
		}
		// Scott: 2005-09-06
#if 0
else if ( !((info->flags & ASYNC_CALLOUT_ACTIVE) &&
(info->flags & ASYNC_CALLOUT_NOHUP)) &&
!(info->flags &ASYNC_CLOSING) )
{
set_bit(NPREAL_EVENT_HANGUP,&info->event);
MXQ_TASK(&info->tqueue);
}
#endif
	}

}

static int npreal_block_til_ready(struct tty_struct *tty, struct file * filp,
		struct npreal_struct *info)
{
	DECLARE_WAITQUEUE(wait, current);
	int			retval;
	int			do_clocal = 0;
	struct nd_struct	*nd;


	if (!(nd=info->net_node))
		return (-EIO);
#if 0
	/*
	 * If the device is in the middle of being closed, then block
	 * until it's done, and then try again.
	 */
	if ( tty_hung_up_p(filp) || (info->flags & ASYNC_CLOSING) )
	{
		if ( !tty_hung_up_p(filp) )
			interruptible_sleep_on(&info->close_wait);
#ifdef SERIAL_DO_RESTART
		if ( info->flags & ASYNC_HUP_NOTIFY )
		{
			return(-EAGAIN);
		}
		else
			return(-ERESTARTSYS);
#else
return(-EAGAIN);
#endif
	}
#endif

	/*
	 * If this is a callout device, then just make sure the normal
	 * device isn't being used.
	 */
	if ( MX_TTY_DRV(subtype) == SERIAL_TYPE_CALLOUT )
	{
		if ( info->flags & ASYNC_NORMAL_ACTIVE )
			return(-EBUSY);
		if ( (info->flags & ASYNC_CALLOUT_ACTIVE) &&
				(info->flags & ASYNC_SESSION_LOCKOUT) &&
				(info->session != MX_SESSION()) )
			return(-EBUSY);
		if ( (info->flags & ASYNC_CALLOUT_ACTIVE) &&
				(info->flags & ASYNC_PGRP_LOCKOUT) &&
				(info->pgrp != MX_CGRP()) )
			return(-EBUSY);
		info->flags |= ASYNC_CALLOUT_ACTIVE;
		return(0);
	}

	/*
	 * If non-blocking mode is set, or the port is not enabled,
	 * then make the check up front and then exit.
	 */
	if ( (filp->f_flags & O_NONBLOCK) ||
			(tty->flags & (1 << TTY_IO_ERROR)) )
	{
		if ( info->flags & ASYNC_CALLOUT_ACTIVE )
		{
			return(-EBUSY);
		}
		info->flags |= ASYNC_NORMAL_ACTIVE;
		return(0);
	}

	if ( info->flags & ASYNC_CALLOUT_ACTIVE )
	{
		if ( info->normal_termios.c_cflag & CLOCAL )
			do_clocal = 1;
	}
	else
	{
#if (LINUX_VERSION_CODE < VERSION_CODE(3,7,0))	    
		if ( tty->termios->c_cflag & CLOCAL )
#else
			if ( tty->termios.c_cflag & CLOCAL )
#endif
				do_clocal = 1;
	}

	/*
	 * Block waiting for the carrier detect and the line to become
	 * free (i.e., not in use by the callout).  While we are in
	 * this loop, info->count is dropped by one, so that
	 * npreal_close() knows when to free things.  We restore it upon
	 * exit, either normal or abnormal.
	 */
	retval = 0;
	add_wait_queue(&info->open_wait, &wait);
	while ( 1 )
	{
		set_current_state(TASK_INTERRUPTIBLE);
		if ( tty_hung_up_p(filp) || (info->flags & ASYNC_CLOSING) )
		{
			if ( !tty_hung_up_p(filp) )
			{
#ifdef SERIAL_DO_RESTART
				if ( info->flags & ASYNC_HUP_NOTIFY )
				{
					retval = -EAGAIN;
				}
				else
					retval = -ERESTARTSYS;
#else
retval = -EAGAIN;
#endif
			}
			break;
		}
		if ( !(info->flags & ASYNC_CALLOUT_ACTIVE) &&
				!(info->flags & ASYNC_CLOSING) &&
				(do_clocal || (info->modem_status & UART_MSR_DCD)) )
			break;
		if ( signal_pending(current) )
		{
			retval = -EIO;
			break;
		}
		schedule();
	}
	current->state = TASK_RUNNING;
	remove_wait_queue(&info->open_wait, &wait);
	if ( retval )
		return(retval);
	info->flags |= ASYNC_NORMAL_ACTIVE;
	return(0);
}

static int npreal_startup(struct npreal_struct * info,struct file *filp,struct tty_struct *tty)
{
	unsigned long	page;
	struct	nd_struct *nd;
	char rsp_buffer[8];
	int  rsp_length = sizeof(rsp_buffer);
	int	cnt = 0;
	DECLARE_WAITQUEUE(wait, current);

	DBGPRINT(MX_DEBUG_LOUD, "(Entry)\n");

	if (!(nd=info->net_node))
	{
		// Scott info->count++;
		DBGPRINT(MX_DEBUG_ERROR, "info->net_node is null\n");
		return -EIO;
	}

	add_wait_queue(&nd->initialize_wait, &wait);
	while (test_and_set_bit(NPREAL_NET_DO_INITIALIZE,&nd->flag))
	{
		if ( signal_pending(current) )
		{
			DBGPRINT(MX_DEBUG_ERROR, "signal_pending break\n");
			break;
		}
		schedule();
	}
	current->state = TASK_RUNNING;
	remove_wait_queue(&nd->initialize_wait, &wait);

	// Scott: 2005/07/13
	// Set tty->driver_data before entering npreal_startup(), so that the tty driver
	// can decrease refcount if npreal_startup() failed, by calling npreal_close().
	// Scott	info->count++;
	// Scott	tty->driver_data = info;
	info->tty = tty;
	if ( signal_pending(current) )
	{
		DBGPRINT(MX_DEBUG_ERROR, "signal_pending occurred\n");
		if ( waitqueue_active(&nd->initialize_wait))
			wake_up_interruptible( &nd->initialize_wait );
		return -EIO;
	}
#if 0
	if ( tty_hung_up_p(filp) || (info->flags & ASYNC_CLOSING) )
	{
		clear_bit(NPREAL_NET_DO_INITIALIZE,&nd->flag);
		if ( waitqueue_active(&nd->initialize_wait))
			wake_up_interruptible( &nd->initialize_wait );
		if ( !tty_hung_up_p(filp) )
			interruptible_sleep_on(&info->close_wait);
#ifdef SERIAL_DO_RESTART
		if ( info->flags & ASYNC_HUP_NOTIFY )
			return(-EAGAIN);
		else
			return(-ERESTARTSYS);
#else
		return(-EAGAIN);
#endif
	}
#endif
	if ( info->flags & ASYNC_INITIALIZED )
	{
		clear_bit(NPREAL_NET_DO_INITIALIZE,&nd->flag);
		if ( waitqueue_active(&nd->initialize_wait))
			wake_up_interruptible( &nd->initialize_wait );
		return(0);
	}

	page = GET_FPAGE(GFP_KERNEL);
	if ( !page )
	{
		clear_bit(NPREAL_NET_DO_INITIALIZE,&nd->flag);
		if ( waitqueue_active(&nd->initialize_wait))
			wake_up_interruptible( &nd->initialize_wait );
		DBGPRINT(MX_DEBUG_ERROR, "allocate page failed\n");
		return(-ENOMEM);
	}

	if (!(nd->flag & NPREAL_NET_TTY_INUSED))
	{
		nd->cmd_buffer[0] = 0;
		npreal_wait_and_set_command(nd,NPREAL_LOCAL_COMMAND_SET,LOCAL_CMD_TTY_USED);
		nd->cmd_buffer[2] = 0;
		nd->cmd_ready = 1;
		if ( waitqueue_active(&nd->select_ex_wait))
		{
			wake_up_interruptible( &nd->select_ex_wait );
		}
		if (npreal_wait_command_completed(nd,
				NPREAL_LOCAL_COMMAND_SET,
				LOCAL_CMD_TTY_USED,
				NPREAL_CMD_TIMEOUT, // Scott MAX_SCHEDULE_TIMEOUT,
				rsp_buffer,
				&rsp_length) != 0)
		{

			DBGPRINT(MX_DEBUG_ERROR, "wait for LOCAL_CMD_TTY_USED response failed\n");
			npreal_wait_and_set_command(nd,NPREAL_LOCAL_COMMAND_SET,LOCAL_CMD_TTY_UNUSED);
			nd->cmd_buffer[2] = 0;
			nd->cmd_ready = 1;
			if ( waitqueue_active(&nd->select_ex_wait))
			{
				wake_up_interruptible( &nd->select_ex_wait );
			}
			npreal_wait_command_completed(nd,
					NPREAL_LOCAL_COMMAND_SET,
					LOCAL_CMD_TTY_UNUSED, // LOCAL_CMD_TTY_USED,
					NPREAL_CMD_TIMEOUT, // Scott MAX_SCHEDULE_TIMEOUT,
					rsp_buffer,
					&rsp_length);
			goto startup_err;
		}
#ifdef OFFLINE_POLLING
		else
		{
			/* Check connection fail */
			if(!rsp_buffer[2])
			{
				DBGPRINT(MX_DEBUG_ERROR, "LOCAL_CMD_TTY_USED shows connection failed\n");
				npreal_wait_and_set_command(nd,NPREAL_LOCAL_COMMAND_SET,LOCAL_CMD_TTY_UNUSED);
				nd->cmd_buffer[2] = 0;
				nd->cmd_ready = 1;
				if ( waitqueue_active(&nd->select_ex_wait))
				{
					wake_up_interruptible( &nd->select_ex_wait );
				}
				npreal_wait_command_completed(nd,
						NPREAL_LOCAL_COMMAND_SET,
						LOCAL_CMD_TTY_UNUSED, // LOCAL_CMD_TTY_USED,
						NPREAL_CMD_TIMEOUT, // Scott MAX_SCHEDULE_TIMEOUT,
						rsp_buffer,
						&rsp_length);
				goto startup_err;
			}
		}
#endif
		nd->flag |= NPREAL_NET_TTY_INUSED;
	}
	else
	{
		while ((nd->cmd_ready == 1)&&(cnt++ < 10))
		{
			current->state = TASK_INTERRUPTIBLE;
			schedule_timeout(HZ/100);
		}
	}
	/*
	 * and set the speed of the serial port
	 */
	nd->flag &= ~NPREAL_NET_DO_SESSION_RECOVERY;
	info->modem_status = 0;
	info->modem_control = 0;
#if (LINUX_VERSION_CODE < VERSION_CODE(3,7,0))		
	if (info->tty->termios->c_cflag & CBAUD)
#else
		if (info->tty->termios.c_cflag & CBAUD)
#endif
			info->modem_control = UART_MCR_DTR | UART_MCR_RTS;

	if (npreal_port_init(info, 0) != 0) {
		DBGPRINT(MX_DEBUG_ERROR, "npreal_port_init() failed\n");

		goto startup_err;
	}
	if (info->type == PORT_16550A)
	{
		if (nd->server_type == CN2500)
			info->xmit_fifo_size = 64;
		else
			info->xmit_fifo_size = 16;
	}
	else
		info->xmit_fifo_size = 1;

	if (npreal_wait_and_set_command(nd,NPREAL_ASPP_COMMAND_SET,ASPP_CMD_TX_FIFO) < 0)
	{
		DBGPRINT(MX_DEBUG_ERROR, "Set ASPP_CMD_TX_FIFO failed\n");
		goto startup_err;
	}
	nd->cmd_buffer[2] = 1;
	nd->cmd_buffer[3] = info->xmit_fifo_size;
	nd->cmd_ready = 1;
	if ( waitqueue_active(&nd->select_ex_wait))
	{
		wake_up_interruptible( &nd->select_ex_wait );
	}
	rsp_length = sizeof(rsp_buffer);
	if (npreal_wait_command_completed(nd,
			NPREAL_ASPP_COMMAND_SET,
			ASPP_CMD_TX_FIFO,NPREAL_CMD_TIMEOUT,
			rsp_buffer,
			&rsp_length) != 0)
	{
		DBGPRINT(MX_DEBUG_ERROR, "Wait for ASPP_CMD_TX_FIFO response failed\n");
		goto startup_err;
	}

	if ( info->xmit_buf )
		free_page(page);
	else
		info->xmit_buf = (unsigned char *)page;

	if ( info->tty )
		test_and_clear_bit(TTY_IO_ERROR, &info->tty->flags);
	info->xmit_cnt = info->xmit_head = info->xmit_tail = 0;

	info->flags |= ASYNC_INITIALIZED;
	clear_bit(NPREAL_NET_DO_INITIALIZE,&nd->flag);
	if ( waitqueue_active(&nd->initialize_wait))
		wake_up_interruptible( &nd->initialize_wait );
	return(0);
	startup_err:
	;
	npreal_disconnect(nd, rsp_buffer, &rsp_length);
	free_page(page);

	clear_bit(NPREAL_NET_DO_INITIALIZE,&nd->flag);
	if ( waitqueue_active(&nd->initialize_wait))
		wake_up_interruptible( &nd->initialize_wait );
	return -EIO;
}

/*
 * This routine will shutdown a serial port; interrupts maybe disabled, and
 * DTR is dropped if the hangup on close termio flag is on.
 */
static void npreal_shutdown(struct npreal_struct * info)
{
	struct nd_struct *nd = info->net_node;
	unsigned long   flags;
	//  char rsp_buffer[8];
	//	int  rsp_length = sizeof(rsp_buffer);

	// Scott: 2005-09-18
	// nd can't be null.
	if (!nd)
		DBGPRINT(MX_DEBUG_ERROR, "nd is null\n");

	while (test_and_set_bit(NPREAL_NET_DO_INITIALIZE,&nd->flag))
	{
		if ( signal_pending(current) )
			break;
		current->state = TASK_INTERRUPTIBLE;
		schedule_timeout(HZ/100);
	}


	if ( !(info->flags & ASYNC_INITIALIZED) )
	{
		goto shutdown_ok;
	}

	//down (&info->tx_semaphore);
	DOWN(info->tx_lock, flags);
	if ( info->xmit_buf )
	{
		free_page((unsigned long)info->xmit_buf);
		info->xmit_buf = 0;
	}
	//up (&info->tx_semaphore);
	UP(info->tx_lock, flags);

	if ( info->tty )
	{
		set_bit(TTY_IO_ERROR, &info->tty->flags);
		npreal_unthrottle(info->tty);
	}
#if (LINUX_VERSION_CODE < VERSION_CODE(3,7,0)) 
	if (!info->tty || (info->tty->termios->c_cflag & HUPCL))
#else
		if (!info->tty || (info->tty->termios.c_cflag & HUPCL))
#endif
		{
			info->modem_control &= ~(UART_MCR_DTR | UART_MCR_RTS);
		}
	shutdown_ok:
	;
	npreal_port_shutdown(info);
	/* Make sure to disconnect the socket ,race with ASYNC_INITIALIZED */
	//npreal_disconnect(nd, rsp_buffer, &rsp_length);
	info->flags &= ~(ASYNC_NORMAL_ACTIVE|ASYNC_CALLOUT_ACTIVE|ASYNC_INITIALIZED|ASYNC_CLOSING);
	down (&info->rx_semaphore);
	info->tty = 0;
	up (&info->rx_semaphore);
	clear_bit(NPREAL_NET_DO_INITIALIZE,&nd->flag);
	wake_up_interruptible(&info->open_wait);
	wake_up_interruptible(&info->close_wait);
	if ( waitqueue_active(&nd->initialize_wait))
		wake_up_interruptible( &nd->initialize_wait );
}
/*
 * set npreal serial state
 */
static int npreal_port_init(struct npreal_struct *info,
		struct ktermios *old_termios)
{
	struct 	ktermios	*termio;
	int32_t     baud,mode;
	int		baudIndex,modem_status;
	struct 	nd_struct	*nd;
	char rsp_buffer[8];
	int  rsp_length = sizeof(rsp_buffer);

	DBGPRINT(MX_DEBUG_LOUD, "(Entry)\n");

	nd = info->net_node;
	if ( !info->tty || !nd)
	{
		DBGPRINT(MX_DEBUG_ERROR, "info->tty or nd is null\n");
		return -EIO;
	}
#if (LINUX_VERSION_CODE < VERSION_CODE(3,7,0)) 	
	if (!(termio = info->tty->termios))
	{
		DBGPRINT(MX_DEBUG_ERROR, "info->tty->termios is null\n");
		return -EIO;
	}
#else
	termio = &(info->tty->termios);
#endif

	mode = termio->c_cflag & CSIZE;
	if (mode == CS5)
		mode = ASPP_IOCTL_BITS5;
	else if (mode == CS6)
		mode = ASPP_IOCTL_BITS6;
	else if (mode == CS7)
		mode = ASPP_IOCTL_BITS7;
	else if (mode == CS8)
		mode = ASPP_IOCTL_BITS8;

	if (termio->c_cflag & CSTOPB)
		mode |= ASPP_IOCTL_STOP2;
	else
		mode |= ASPP_IOCTL_STOP1;

	if (termio->c_cflag & PARENB)
	{
#ifdef CMSPAR
		if (termio->c_cflag & CMSPAR)
			if (termio->c_cflag & PARODD)
				mode |= ASPP_IOCTL_MARK;
			else
				mode |= ASPP_IOCTL_SPACE;
		else
#endif
			if (termio->c_cflag & PARODD)
				mode |= ASPP_IOCTL_ODD;
			else
				mode |= ASPP_IOCTL_EVEN;
	}
	else
		mode |= ASPP_IOCTL_NONE;


	switch ( termio->c_cflag & (CBAUD|CBAUDEX))
	{
	case B921600:
		baud = 921600L;
		baudIndex = ASPP_IOCTL_B921600;
		break;
	case B460800:
		baud = 460800;
		baudIndex = ASPP_IOCTL_B460800;
		break;
	case B230400:
		baud = 230400L;
		baudIndex = ASPP_IOCTL_B230400;
		break;
	case B115200:
		baud = 115200L;
		baudIndex = ASPP_IOCTL_B115200;
		break;
	case B57600:
		baud = 57600L;
		baudIndex = ASPP_IOCTL_B57600;
		break;
	case B38400:
		baud = 38400L;
		baudIndex = ASPP_IOCTL_B38400;
		if ( (info->flags & ASYNC_SPD_MASK) == ASYNC_SPD_HI )
		{
			baud = 57600L;
			baudIndex = ASPP_IOCTL_B57600;
		}
		if ( (info->flags & ASYNC_SPD_MASK) == ASYNC_SPD_VHI )
		{
			baud = 115200L;
			baudIndex = ASPP_IOCTL_B115200;
		}

#ifdef ASYNC_SPD_SHI
		if ((info->flags & ASYNC_SPD_MASK) == ASYNC_SPD_SHI)
		{
			baud = 230400L;
			baudIndex = ASPP_IOCTL_B230400;
		}
#endif

#ifdef ASYNC_SPD_WARP
		if ((info->flags & ASYNC_SPD_MASK) == ASYNC_SPD_WARP)
		{
			baud = 460800L;
			baudIndex = ASPP_IOCTL_B460800;
		}
#endif
		break;
	case B19200:
		baud = 19200L;
		baudIndex = ASPP_IOCTL_B19200;
		break;
	case B9600:
		baud = 9600L;
		baudIndex = ASPP_IOCTL_B9600;
		break;
	case B4800:
		baud = 4800L;
		baudIndex = ASPP_IOCTL_B4800;
		break;
	case B2400:
		baud = 2400L;
		baudIndex = ASPP_IOCTL_B2400;
		break;
	case B1800:
		baud = 1800L;
		baudIndex = 0xff;
		break;
	case B1200:
		baud = 1200L;
		baudIndex = ASPP_IOCTL_B1200;
		break;
	case B600:
		baud = 600L;
		baudIndex = ASPP_IOCTL_B600;
		break;
	case B300:
		baud = 300L;
		baudIndex = ASPP_IOCTL_B300;
		break;
	case B200:
		baud = 200L;
		baudIndex = 0xff;
		break;
	case B150:
		baud = 150L;
		baudIndex = ASPP_IOCTL_B150;
		break;
	case B134:
		baud = 134L;
		baudIndex = ASPP_IOCTL_B134;
		break;
	case B110:
		baud = 110L;
		baudIndex = ASPP_IOCTL_B110;
		break;
	case B75:
		baud = 75L;
		baudIndex = ASPP_IOCTL_B75;
		break;
	case B50:
		baud = 50L;
		baudIndex = ASPP_IOCTL_B50;
		break;
	default:
		baud = 0;
		baudIndex = 0xff;
	}
#ifdef ASYNC_SPD_CUST
	if ((info->flags & ASYNC_SPD_MASK) == ASYNC_SPD_CUST)
	{
		baudIndex = 0xff;           //force to use SET_BAUD later...
	}
#endif
	if (baud > 921600L)
	{
		termio->c_cflag &= ~(CBAUD|CBAUDEX);
		termio->c_cflag |=
				old_termios->c_cflag &(CBAUD|CBAUDEX);
	}
	if (npreal_wait_and_set_command(nd,NPREAL_ASPP_COMMAND_SET,ASPP_CMD_PORT_INIT) < 0)
	{
		DBGPRINT(MX_DEBUG_ERROR, "set ASPP_CMD_PORT_INIT failed\n");
		return (-EIO);
	}
	nd->cmd_buffer[2] = 8;
	//
	// baud rate
	//
	nd->cmd_buffer[3] = baudIndex;
	//
	// mode
	//
	nd->cmd_buffer[4] = mode;
	//
	// line control
	//
	if (info->modem_control & UART_MCR_DTR)
		nd->cmd_buffer[5] = 1;
	else
		nd->cmd_buffer[5] = 0;
	if (info->modem_control & UART_MCR_RTS)
		nd->cmd_buffer[6] = 1;
	else
		nd->cmd_buffer[6] = 0;
	//
	// flow control
	//
	if (termio->c_cflag & CRTSCTS)
	{
		nd->cmd_buffer[7] = 1;
		nd->cmd_buffer[8] = 1;
	}
	else
	{
		nd->cmd_buffer[7] = 0;
		nd->cmd_buffer[8] = 0;
	}
	if (termio->c_iflag & IXON)
	{
		nd->cmd_buffer[9] = 1;
	}
	else
	{
		nd->cmd_buffer[9] = 0;
	}
	if (termio->c_iflag & IXOFF)
	{
		nd->cmd_buffer[10] = 1;
	}
	else
	{
		nd->cmd_buffer[10] = 0;
	}
	nd->cmd_ready = 1;
	if ( waitqueue_active(&nd->select_ex_wait))
		wake_up_interruptible( &nd->select_ex_wait );

	if (npreal_wait_command_completed(nd,
			NPREAL_ASPP_COMMAND_SET,
			ASPP_CMD_PORT_INIT, NPREAL_CMD_TIMEOUT,
			rsp_buffer,
			&rsp_length))
	{
		DBGPRINT(MX_DEBUG_ERROR, "wait ASPP_CMD_PORT_INIT response failed\n");
		return(-EIO);
	}
	if (rsp_length != 6)
	{
		DBGPRINT(MX_DEBUG_ERROR, "invalid ASPP_CMD_PORT_INIT response1\n");
		return(-EIO);
	}
	if (rsp_buffer[2] != 3)
	{
		DBGPRINT(MX_DEBUG_ERROR, "invalid ASPP_CMD_PORT_INIT response2\n");
		return(-EIO);
	}
	modem_status = 0;

	if (((unsigned char)rsp_buffer[3]==0xff) &&
			((unsigned char)rsp_buffer[4]==0xff) &&
			((unsigned char)rsp_buffer[5]==0xff))
	{
		termio->c_cflag &= ~(CBAUD|CBAUDEX);
		termio->c_cflag |=
				old_termios->c_cflag &(CBAUD|CBAUDEX);
	}
	else
	{
		if (rsp_buffer[3])
			modem_status |= UART_MSR_DSR;
		if (rsp_buffer[4])
			modem_status |= UART_MSR_CTS;
		if (rsp_buffer[5])
		{
			modem_status |= UART_MSR_DCD;
		}
	}

	npreal_check_modem_status(info,modem_status);

	if ((baudIndex == 0xff)&&(baud != 0))
	{
		if (npreal_wait_and_set_command(nd,NPREAL_ASPP_COMMAND_SET,ASPP_CMD_SETBAUD) < 0)
		{
			DBGPRINT(MX_DEBUG_ERROR, "set ASPP_CMD_SETBAUD failed\n");
			return(-EIO);
		}
		if ((info->flags & ASYNC_SPD_MASK) == ASYNC_SPD_CUST)
		{
			if(info->custom_divisor)
				baud = info->baud_base/info->custom_divisor;
		}
		nd->cmd_buffer[2] = 4;
		memcpy(&nd->cmd_buffer[3],&baud,4);
		nd->cmd_ready = 1;
		if ( waitqueue_active(&nd->select_ex_wait))
			wake_up_interruptible( &nd->select_ex_wait );
		rsp_length = sizeof (rsp_buffer);
		if (npreal_wait_command_completed(nd,
				NPREAL_ASPP_COMMAND_SET,
				ASPP_CMD_SETBAUD,NPREAL_CMD_TIMEOUT,
				rsp_buffer,
				&rsp_length))
		{
			DBGPRINT(MX_DEBUG_ERROR, "wait ASPP_CMD_SETBAUD response failed\n");
			return(-EIO);
		}
		if (rsp_length != 4)
		{
			DBGPRINT(MX_DEBUG_ERROR, "invalid ASPP_CMD_SETBAUD response1\n");
			return(-EIO);
		}
		if ((rsp_buffer[2] != 'O') ||
				(rsp_buffer[3] != 'K') )
		{
			DBGPRINT(MX_DEBUG_ERROR, "invalid ASPP_CMD_SETBAUD response2\n");
			return(-EIO);
		}

	}

	if (termio->c_iflag & (IXON | IXOFF))
	{
		if (npreal_wait_and_set_command(nd,NPREAL_ASPP_COMMAND_SET,ASPP_CMD_XONXOFF))
		{
			DBGPRINT(MX_DEBUG_ERROR, "set ASPP_CMD_XONXOFF failed\n");
			return -EIO;
		}
		nd->cmd_buffer[2] = 2;
		nd->cmd_buffer[3] = termio->c_cc[VSTART];
		nd->cmd_buffer[4] = termio->c_cc[VSTOP];
		nd->cmd_ready = 1;
		if ( waitqueue_active(&nd->select_ex_wait))
			wake_up_interruptible( &nd->select_ex_wait );
		rsp_length = sizeof (rsp_buffer);
		if (npreal_wait_command_completed(nd,
				NPREAL_ASPP_COMMAND_SET,
				ASPP_CMD_XONXOFF,NPREAL_CMD_TIMEOUT,
				rsp_buffer,
				&rsp_length))
		{
			DBGPRINT(MX_DEBUG_ERROR, "wait ASPP_CMD_XONXOFF response failed\n");
			return(-EIO);
		}
		if (rsp_length != 4)
		{
			DBGPRINT(MX_DEBUG_ERROR, "invalid ASPP_CMD_XONXOFF response1\n");
			return(-EIO);
		}
		if ((rsp_buffer[2] != 'O') ||
				(rsp_buffer[3] != 'K') )
		{
			DBGPRINT(MX_DEBUG_ERROR, "invalid ASPP_CMD_XONXOFF response2\n");
			return(-EIO);
		}

	}

	if ( termio->c_cflag & CLOCAL )
	{
		info->flags &= ~ASYNC_CHECK_CD;
	}
	else
	{
		info->flags |= ASYNC_CHECK_CD;
	}
	if ( !info->tty)
	{
		DBGPRINT(MX_DEBUG_ERROR, "info->tty is null\n");
		return -EIO;
	}
	return(0);
}

#if 1 // Scott
static int npreal_port_shutdown(struct npreal_struct *info)
{
	struct 	nd_struct	*nd;
	char rsp_buffer[8];
	int  rsp_length = sizeof(rsp_buffer);

	nd = info->net_node;
	if ( !nd)
	{
		return -EIO;
	}

	//    nd->cmd_buffer[0] = NPREAL_LOCAL_COMMAND_SET;
	nd->cmd_buffer[0] = 0;

	npreal_wait_and_set_command(nd,
			NPREAL_LOCAL_COMMAND_SET,
			LOCAL_CMD_TTY_UNUSED);

	//nd->cmd_buffer[1] = LOCAL_CMD_TTY_UNUSED;
	nd->cmd_buffer[2] = 0;
	nd->cmd_ready = 1;
	if ( waitqueue_active(&nd->select_ex_wait))
		wake_up_interruptible( &nd->select_ex_wait );

	if (npreal_wait_command_completed(nd,
			NPREAL_LOCAL_COMMAND_SET,
			LOCAL_CMD_TTY_UNUSED,NPREAL_CMD_TIMEOUT,
			rsp_buffer,
			&rsp_length) != 0) {
		npreal_wait_and_set_command(nd,
				NPREAL_LOCAL_COMMAND_SET,
				LOCAL_CMD_TTY_UNUSED);
		nd->cmd_buffer[2] = 0;
		nd->cmd_ready = 1;

		if ( waitqueue_active(&nd->select_ex_wait))
			wake_up_interruptible( &nd->select_ex_wait );
		npreal_wait_command_completed(nd,
				NPREAL_LOCAL_COMMAND_SET,
				LOCAL_CMD_TTY_UNUSED,NPREAL_CMD_TIMEOUT,
				rsp_buffer,
				&rsp_length);
	}
	nd->flag &= ~NPREAL_NET_TTY_INUSED;
	return(0);
}
#else
static int npreal_port_shutdown(struct npreal_struct *info)
{
	struct 	termios	*termio;
	int32_t	baud,mode;
	int		baudIndex;
	struct 	nd_struct	*nd;

	nd = info->net_node;
	if ( !info->tty || !nd)
	{
		return -EIO;
	}
	if (!(termio = info->tty->termios))
	{
		return -EIO;
	}

	mode = termio->c_cflag & CSIZE;
	if (mode == CS5)
		mode = ASPP_IOCTL_BITS5;
	else if (mode == CS6)
		mode = ASPP_IOCTL_BITS6;
	else if (mode == CS7)
		mode = ASPP_IOCTL_BITS7;
	else if (mode == CS8)
		mode = ASPP_IOCTL_BITS8;

	if (termio->c_cflag & CSTOPB)
		mode |= ASPP_IOCTL_STOP2;
	else
		mode |= ASPP_IOCTL_STOP1;

	if (termio->c_cflag & PARENB)
	{
		if (termio->c_cflag & PARODD)
			mode |= ASPP_IOCTL_ODD;
		else
			mode |= ASPP_IOCTL_EVEN;
	}
	else
		mode |= ASPP_IOCTL_NONE;


	switch ( termio->c_cflag & (CBAUD|CBAUDEX))
	{
	case B921600:
		baud = 921600L;
		baudIndex = ASPP_IOCTL_B921600;
		break;
	case B460800:
		baud = 460800;
		baudIndex = ASPP_IOCTL_B460800;
		break;
	case B230400:
		baud = 230400L;
		baudIndex = ASPP_IOCTL_B230400;
		break;
	case B115200:
		baud = 115200L;
		baudIndex = ASPP_IOCTL_B115200;
		break;
	case B57600:
		baud = 57600L;
		baudIndex = ASPP_IOCTL_B57600;
		break;
	case B38400:
		baud = 38400L;
		baudIndex = ASPP_IOCTL_B38400;
		if ( (info->flags & ASYNC_SPD_MASK) == ASYNC_SPD_HI )
		{
			baud = 57600L;
			baudIndex = ASPP_IOCTL_B57600;
		}
		if ( (info->flags & ASYNC_SPD_MASK) == ASYNC_SPD_VHI )
		{
			baud = 115200L;
			baudIndex = ASPP_IOCTL_B115200;
		}

#ifdef ASYNC_SPD_SHI
if ((info->flags & ASYNC_SPD_MASK) == ASYNC_SPD_SHI)
{
baud = 230400L;
baudIndex = ASPP_IOCTL_B230400;
}
#endif

#ifdef ASYNC_SPD_WARP
if ((info->flags & ASYNC_SPD_MASK) == ASYNC_SPD_WARP)
{
baud = 460800L;
baudIndex = ASPP_IOCTL_B460800;
}
#endif
break;
	case B19200:
		baud = 19200L;
		baudIndex = ASPP_IOCTL_B19200;
		break;
	case B9600:
		baud = 9600L;
		baudIndex = ASPP_IOCTL_B9600;
		break;
	case B4800:
		baud = 4800L;
		baudIndex = ASPP_IOCTL_B4800;
		break;
	case B2400:
		baud = 2400L;
		baudIndex = ASPP_IOCTL_B2400;
		break;
	case B1800:
		baud = 1800L;
		baudIndex = 0xff;
		break;
	case B1200:
		baud = 1200L;
		baudIndex = ASPP_IOCTL_B1200;
		break;
	case B600:
		baud = 600L;
		baudIndex = ASPP_IOCTL_B600;
		break;
	case B300:
		baud = 300L;
		baudIndex = ASPP_IOCTL_B300;
		break;
	case B200:
		baud = 200L;
		baudIndex = 0xff;
		break;
	case B150:
		baud = 150L;
		baudIndex = ASPP_IOCTL_B150;
		break;
	case B134:
		baud = 134L;
		baudIndex = ASPP_IOCTL_B134;
		break;
	case B110:
		baud = 110L;
		baudIndex = ASPP_IOCTL_B110;
		break;
	case B75:
		baud = 75L;
		baudIndex = ASPP_IOCTL_B75;
		break;
	case B50:
		baud = 50L;
		baudIndex = 0xff;
		break;
		break;
	default:
		baud = 0;
		baudIndex = 0xff;
	}
#if 0
	if (npreal_wait_and_set_command(nd,NPREAL_ASPP_COMMAND_SET,ASPP_CMD_PORT_INIT) < 0)
	{
		return (-EIO);
	}
#endif
	nd->cmd_buffer[0] = NPREAL_ASPP_COMMAND_SET;
	nd->cmd_buffer[1] = ASPP_CMD_PORT_INIT;
	nd->cmd_buffer[2] = 8;
	//
	// baud rate
	//
	nd->cmd_buffer[3] = baudIndex;
	//
	// mode
	//
	nd->cmd_buffer[4] = mode;
	//
	// line control
	//
	if (info->modem_control & UART_MCR_DTR)
		nd->cmd_buffer[5] = 1;
	else
		nd->cmd_buffer[5] = 0;
	if (info->modem_control & UART_MCR_RTS)
		nd->cmd_buffer[6] = 1;
	else
		nd->cmd_buffer[6] = 0;
	// H/W flow control
	nd->cmd_buffer[7] = 0;
	nd->cmd_buffer[8] = 0;
	// Software flow control
	if (termio->c_iflag & IXON)
		nd->cmd_buffer[9] = 1;
	else
		nd->cmd_buffer[9] = 0;
	if (termio->c_iflag & IXOFF)
		nd->cmd_buffer[10] = 1;
	else
		nd->cmd_buffer[10] = 0;

	nd->cmd_ready = 1;
	if ( waitqueue_active(&nd->select_ex_wait))
		wake_up_interruptible( &nd->select_ex_wait );
	//
	// We don't check result at this moment,because it will take time and then
	// next open may fail
	//
	/*
if (npreal_wait_command_completed(nd,
NPREAL_ASPP_COMMAND_SET,
ASPP_CMD_PORT_INIT,NPREAL_CMD_TIMEOUT,
rsp_buffer,
&rsp_length)) {
return(-EIO);
}
if (rsp_length != 6) {
return(-EIO);
}
if (rsp_buffer[2] != 3) {
return(-EIO);
}
	 */
	return(0);
}
#endif

/*
 * ------------------------------------------------------------
 * friends of npreal_ioctl()
 * ------------------------------------------------------------
 */
static int npreal_get_serial_info(struct npreal_struct * info,
		struct serial_struct * retinfo)
{
	struct serial_struct	tmp;

	if ( !retinfo )
		return(-EFAULT);
	memset(&tmp, 0, sizeof(tmp));
	tmp.type = info->type;
	tmp.line = info->port;
	tmp.flags = info->flags;
	tmp.close_delay = info->close_delay;
	tmp.closing_wait = info->closing_wait;
	tmp.custom_divisor = info->custom_divisor;
	tmp.baud_base = info->baud_base;
	tmp.hub6 = 0;
	if (copy_to_user(retinfo, &tmp, sizeof(*retinfo)))
		return(-EFAULT);
	return(0);
}

static int npreal_set_serial_info(struct npreal_struct * info,
		struct serial_struct * new_info)
{
	struct serial_struct	new_serial;
	unsigned int		flags;
	int			retval = 0;
	char	rsp_buffer[8];
	int	rsp_length;


	if ( !new_info)
		return(-EFAULT);
	if (copy_from_user(&new_serial, new_info, sizeof(new_serial)))
		return(-EFAULT);

	flags = info->flags & ASYNC_SPD_MASK;

	if ( !capable(CAP_SYS_ADMIN))
	{
		if ((new_serial.close_delay != info->close_delay) ||
				((new_serial.flags & ~ASYNC_USR_MASK) !=
						(info->flags & ~ASYNC_USR_MASK)) )
			return(-EPERM);
		info->flags = ((info->flags & ~ASYNC_USR_MASK) |
				(new_serial.flags & ASYNC_USR_MASK));
	}
	else
	{
		/*
		 * OK, past this point, all the error checking has been done.
		 * At this point, we start making changes.....
		 */
		info->flags = ((info->flags & ~ASYNC_FLAGS) |
				(new_serial.flags & ASYNC_FLAGS));
		info->close_delay = new_serial.close_delay * HZ/100;
		// Scott: 2005-07-08
		// If user wants to set closing_wait to ASYNC_CLOSING_WAIT_NONE, don't modify the value,
		// since it will be used as a flag indicating closing wait none.
		if (new_serial.closing_wait == ASYNC_CLOSING_WAIT_NONE)
			info->closing_wait = ASYNC_CLOSING_WAIT_NONE;
		else
			info->closing_wait = new_serial.closing_wait * HZ/100;
	}

	info->type = new_serial.type;
	if (info->type == PORT_16550A)
	{
		if (info->net_node)
		{
			if (info->net_node->server_type == CN2500)
				info->xmit_fifo_size = 64;
			else
				info->xmit_fifo_size = 16;
		}
		else
			info->xmit_fifo_size = 16;
	}
	else
		info->xmit_fifo_size = 1;
	if ( info->flags & ASYNC_INITIALIZED )
	{
		if ( flags != (info->flags & ASYNC_SPD_MASK) )
		{
			retval=npreal_port_init(info,0);
		}
		if (info->net_node)
		{
			npreal_wait_and_set_command(info->net_node,NPREAL_ASPP_COMMAND_SET,ASPP_CMD_TX_FIFO);
			info->net_node->cmd_buffer[2] = 1;
			info->net_node->cmd_buffer[3] = info->xmit_fifo_size;
			info->net_node->cmd_ready = 1;
			if ( waitqueue_active(&info->net_node->select_ex_wait))
				wake_up_interruptible(&info->net_node->select_ex_wait);
			npreal_wait_command_completed(info->net_node,
					NPREAL_ASPP_COMMAND_SET,
					ASPP_CMD_TX_FIFO,
					NPREAL_CMD_TIMEOUT,
					rsp_buffer,
					&rsp_length);
		}
	}
	info->custom_divisor = new_serial.custom_divisor;
	if (info->custom_divisor == 0)
		info->baud_base = 921600L;
	else
		info->baud_base = new_serial.baud_base;
	return(retval);
}

/*
 * npreal_get_lsr_info - get line status register info
 *
 * Purpose: Let user call ioctl() to get info when the UART physically
 *	    is emptied.  On bus types like RS485, the transmitter must
 *	    release the bus after transmitting. This must be done when
 *	    the transmit shift register is empty, not be done when the
 *	    transmit holding register is empty.  This functionality
 *	    allows an RS485 driver to be written in user space.
 */
static int npreal_get_lsr_info(struct npreal_struct * info, unsigned int *value)
{
	unsigned int	result = 0;

	if (npreal_wait_oqueue(info,0) == 0)
		result  = TIOCSER_TEMT;
	put_to_user(result, value);
	return(0);
}


static void npreal_start_break(struct nd_struct *nd)
{
	char	rsp_buffer[8];
	int	rsp_length = sizeof (rsp_buffer);

	npreal_wait_and_set_command(nd,NPREAL_ASPP_COMMAND_SET,ASPP_CMD_START_BREAK);
	nd->cmd_buffer[2] = 0;
	nd->cmd_ready = 1;
	if ( waitqueue_active(&nd->select_ex_wait))
	{
		wake_up_interruptible( &nd->select_ex_wait );
	}

	if (npreal_wait_command_completed(nd,
			NPREAL_ASPP_COMMAND_SET,
			ASPP_CMD_START_BREAK,NPREAL_CMD_TIMEOUT,
			rsp_buffer,
			&rsp_length))
		return;
	if (rsp_length != 4)
		return;
	if ((rsp_buffer[2] != 'O') ||
			(rsp_buffer[3] != 'K') )
		return;
}

static void npreal_stop_break(struct nd_struct *nd)
{
	char	rsp_buffer[8];
	int	rsp_length = sizeof (rsp_buffer);

	npreal_wait_and_set_command(nd,NPREAL_ASPP_COMMAND_SET,ASPP_CMD_STOP_BREAK);
	nd->cmd_buffer[2] = 0;
	nd->cmd_ready = 1;
	if ( waitqueue_active(&nd->select_ex_wait))
		wake_up_interruptible( &nd->select_ex_wait );
	rsp_length = sizeof(rsp_buffer);
	if (npreal_wait_command_completed(nd,
			NPREAL_ASPP_COMMAND_SET,
			ASPP_CMD_STOP_BREAK,NPREAL_CMD_TIMEOUT,
			rsp_buffer,
			&rsp_length))
		return;
	if (rsp_length != 4)
		return;
	if ((rsp_buffer[2] != 'O') ||
			(rsp_buffer[3] != 'K') )
		return;
}
static int npreal_break(struct tty_struct *ttyinfo, int break_state)
{
	struct npreal_struct *info;
	struct nd_struct  	*nd;

	DBGPRINT(MX_DEBUG_LOUD, "(Entry)\n");

	if ( !ttyinfo  )
		return (-EFAULT);

	info = (struct npreal_struct *)ttyinfo->driver_data;

	// Scott: 2005-09-12
	if (!info)
		return (-EFAULT);


	if (!(nd = info->net_node))
		return (-EFAULT);


	if (break_state == -1)
	{
		npreal_start_break(nd);
	}
	else
	{
		npreal_stop_break(nd);
	}
	return (0);

}

/*
 * This routine sends a break character out the serial port.
 */
static void npreal_send_break(struct npreal_struct * info, int duration)
{
	struct	nd_struct	*nd;

	if (!(nd = info->net_node))
		return;

	npreal_start_break(nd);

	current->state = TASK_INTERRUPTIBLE;
	schedule_timeout(duration);

	npreal_stop_break(nd);

}


static int npreal_tiocmget(struct tty_struct *tty)
{
	struct npreal_struct *info = (struct npreal_struct *) tty->driver_data;

	DBGPRINT(MX_DEBUG_LOUD, "(Entry)\n");

	if (!info)
	{
		DBGPRINT(MX_DEBUG_ERROR, "info is null\n");
		return (-EINVAL);
	}

	if (PORTNO(tty) == NPREAL_PORTS)
		return (-ENOIOCTLCMD);
	if (tty->flags & (1 << TTY_IO_ERROR))
		return (-EIO);

	return ((info->modem_control & UART_MCR_RTS) ? TIOCM_RTS : 0) |
			((info->modem_control & UART_MCR_DTR) ? TIOCM_DTR : 0) |
			((info->modem_status  & UART_MSR_DCD) ? TIOCM_CAR : 0) |
			((info->modem_status  & UART_MSR_RI)  ? TIOCM_RNG : 0) |
			((info->modem_status  & UART_MSR_DSR) ? TIOCM_DSR : 0) |
			((info->modem_status  & UART_MSR_CTS) ? TIOCM_CTS : 0);
}

static int npreal_tiocmset(struct tty_struct *tty,
		unsigned int set, unsigned int clear)
{
	struct npreal_struct *info = (struct npreal_struct *) tty->driver_data;
	struct	nd_struct	*nd;

	DBGPRINT(MX_DEBUG_LOUD, "(Entry)\n");

	if (!info)
	{
		DBGPRINT(MX_DEBUG_ERROR, "info is null\n");
		return (-EINVAL);
	}

	if (!(nd = info->net_node))
		return(-EINVAL);

	if (PORTNO(tty) == NPREAL_PORTS)
		return (-ENOIOCTLCMD);
	if (tty->flags & (1 << TTY_IO_ERROR))
		return (-EIO);

	if (set & TIOCM_RTS)
		info->modem_control |= UART_MCR_RTS;
	if (set & TIOCM_DTR)
		info->modem_control |= UART_MCR_DTR;

	if (clear & TIOCM_RTS)
		info->modem_control &= ~UART_MCR_RTS;
	if (clear & TIOCM_DTR)
		info->modem_control &= ~UART_MCR_DTR;

	return npreal_linectrl(nd,info->modem_control);
}


#if (LINUX_VERSION_CODE < VERSION_CODE(3,12,0))           
static void tty_buffer_free(struct tty_struct *tty, struct tty_buffer *b)
{
	/* Dumb strategy for now - should keep some stats */
	/* 	printk("Flip dispose %p\n", b); */
	if (b->size >= 512)
		kfree(b);
#if (LINUX_VERSION_CODE < VERSION_CODE(3,12,0))          
	else
	{
#if (LINUX_VERSION_CODE < VERSION_CODE(3,8,0))           
		b->next = tty->buf.free;
		tty->buf.free = b;
#else
		b->next = tty->port->buf.free;
		tty->port->buf.free = b;
#endif        
	}
#else
	else if (b->size > 0)
		llist_add_batch(&b->free, &b->free, &tty->port->buf.free);
#endif
}
#endif

/*
 * This routine is called out of the software interrupt to flush data
 * from the flip buffer to the line discipline.
 */
#if (LINUX_VERSION_CODE < VERSION_CODE(3,12,0))           
static void npreal_flush_to_ldisc(struct work_struct *work)
{
	struct npreal_struct *	info =
			container_of(work, struct npreal_struct, process_flip_tqueue);
	struct tty_struct *	tty;
	int		count;

	struct tty_ldisc *disc;
	struct tty_buffer *tbuf, *head;
	unsigned long 	flags;
	unsigned char	*fp;
	char		*cp;

	if (!info){
		DBGPRINT(MX_DEBUG_ERROR, "MxDebug: info is null @ %d %s\n", __LINE__, __FUNCTION__);
		goto done;
	}

	tty = info->tty;
	if( !tty ){
		DBGPRINT(MX_DEBUG_ERROR, "MxDebug: tty is null @ %d %s\n", __LINE__, __FUNCTION__);
		up(&info->rx_semaphore);
		goto done;
	}

	disc = tty_ldisc_ref(tty);
	if (disc == NULL){ /*  !TTY_LDISC */
		DBGPRINT(MX_DEBUG_ERROR, "MxDebug: disc is null @ %d %s\n", __LINE__, __FUNCTION__);
		return;
	}

	if ( tty && (info->flags & ASYNC_INITIALIZED))
	{
#if (LINUX_VERSION_CODE < VERSION_CODE(3,8,0))  
		spin_lock_irqsave(&tty->buf.lock, flags);
		head = tty->buf.head;
#else
		spin_lock_irqsave(&tty->port->buf.lock, flags);
		head = tty->port->buf.head;
#endif
		if (head != NULL)
		{
#if (LINUX_VERSION_CODE < VERSION_CODE(3,8,0))          
			tty->buf.head = NULL;
#else
			tty->port->buf.head = NULL;
#endif
			for (;;)
			{
				count = head->commit - head->read;
				if (!count)
				{
					if (head->next == NULL)
						break;
					tbuf = head;
					head = head->next;
#if (LINUX_VERSION_CODE < VERSION_CODE(3,12,0))          
					tty_buffer_free(tty, tbuf);
#else 
					tty_buffer_free(tty->port, tbuf);
#endif
					continue;
				}
				if (!tty->receive_room)
				{
					schedule_work(&tty->SAK_work);
					break;
				}
				if (count > tty->receive_room)
					count = tty->receive_room;
				cp = head->char_buf_ptr + head->read;
				fp = head->flag_buf_ptr + head->read;
				head->read += count;
#if (LINUX_VERSION_CODE < VERSION_CODE(3,8,0))                    
				spin_unlock_irqrestore(&tty->buf.lock, flags);
#else
				spin_unlock_irqrestore(&tty->port->buf.lock, flags);
#endif
				disc->ops->receive_buf(tty, cp, fp, count);
#if (LINUX_VERSION_CODE < VERSION_CODE(3,8,0))    
				spin_lock_irqsave(&tty->buf.lock, flags);
#else
				spin_lock_irqsave(&tty->port->buf.lock, flags);
#endif
			}
#if (LINUX_VERSION_CODE < VERSION_CODE(3,8,0))                
			tty->buf.head = head;
#else
			tty->port->buf.head = head;
#endif
		}
#if (LINUX_VERSION_CODE < VERSION_CODE(3,8,0))           
		spin_unlock_irqrestore(&tty->buf.lock, flags);
#else
		spin_unlock_irqrestore(&tty->port->buf.lock, flags);
#endif
		tty_ldisc_deref(disc);
	}

	done:
	;
}
#else
static void npreal_flush_to_ldisc(struct work_struct *work)
{
	struct npreal_struct *	info =
			container_of(work, struct npreal_struct, process_flip_tqueue);

	struct tty_struct *tty;
	struct tty_port *port;

	tty = info->tty;

	if ((info == NULL) || (tty == NULL))
		return;

	port = tty->port;

	tty_flip_buffer_push(port);
}
#endif
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


#if (LINUX_VERSION_CODE < VERSION_CODE(3,10,0))
static struct proc_dir_entry *
npreal_create_proc_entry(
		const char *name,
		const mode_t mode,
		struct proc_dir_entry *parent)
{
	return( create_proc_entry( name, mode, parent ) );
}

static void
npreal_remove_proc_entry(struct proc_dir_entry *pde)
{
	if (!pde) return;
	remove_proc_entry(pde->name, pde->parent);
}

#else

static void npreal_remove_proc_entry(struct proc_dir_entry *pde, int idx)
{
	char tmp[10];

	if (!pde) return;
	sprintf(tmp, "%d", idx);
	if (idx == 404)
		remove_proc_entry("npreal2", NULL);
	else
		remove_proc_entry(tmp, npvar_proc_root);
}
#endif /* 3,10,0 */

static int
npreal_net_open (
		struct inode *inode,
		struct file *file )
{
	struct nd_struct *nd;
	int     rtn = 0;

#if (LINUX_VERSION_CODE < VERSION_CODE(3,10,0))
	struct proc_dir_entry *de;
#endif

	MX_MOD_INC;

	if ( !capable(CAP_SYS_ADMIN) )
	{
		rtn = -EPERM;
		goto done;
	}


	/*
	 *  Make sure that the "private_data" field hasn't already been used.
	 */
	if ( file->private_data )
	{
		rtn = -EINVAL;
		goto done;
	}



	/*
	 *  Get the node pointer, and fail if it doesn't exist.
	 */
	//de = (struct proc_dir_entry *)inode->u.generic_ip;
	/* Casper, 9-11-04
	 Don't get pointer link above. It fail on 2.6 kernel.
	 PDE macro is ok on 2.6.
	 */

#if (LINUX_VERSION_CODE < VERSION_CODE(3,10,0))
	de = PDE(inode);
	if (!de) {
		rtn = -ENXIO;
		goto done;
	}

	nd = (struct nd_struct *) de->data;
#else

	//nd = (struct nd_struct *) &npvar_net_nodes[0];
	nd = (struct nd_struct *) PDE_DATA(inode);
#endif /* 3,10,0 */
	if ( !nd )
	{
		rtn = -ENXIO;
		goto done;
	}

	file->private_data = (void *)nd;

	/*
	 *  This is an exclusive access device.  If it is already
	 *  open, return an error.
	 */

	/*
	 *  Grab the NET lock.
	 */
	down(&nd->semaphore);


	if ( nd->flag & NPREAL_NET_NODE_OPENED)
	{
		rtn = -EBUSY;
		goto unlock;
	}

	nd->flag |= NPREAL_NET_NODE_OPENED;
	nd->tx_ready = 0;
	nd->rx_ready = 1;
	nd->cmd_ready = 0;

#if (LINUX_VERSION_CODE >= VERSION_CODE(3,7,0))    
	tty_register_device(DRV_VAR, nd->tty_node->port, NULL);
#endif

	unlock:

	/*
	 *  Release the NET lock.
	 */
	up( &nd->semaphore );
	//	(struct nd_struct *)(file->private_data) = nd;
	file->private_data = (void*)nd;

	done:

	if ( rtn )
	{
		MX_MOD_DEC;
	}
	return rtn;
}

static int
npreal_net_close (
		struct inode *inode,
		struct file *file )
{
	struct nd_struct *nd;
	/*
	 *  Get the node pointer, and quit if it doesn't exist.
	 */
	nd = (struct nd_struct *)(file->private_data);
	if ( !nd )
	{
		goto done;
	}
	nd->flag  &= ~NPREAL_NET_NODE_OPENED;

#if (LINUX_VERSION_CODE >= VERSION_CODE(3,7,0))    
	tty_unregister_device(DRV_VAR, nd->tty_node->port);
#endif

	done:
	file->private_data = NULL;
	MX_MOD_DEC;
	return(0);
}

static unsigned int
npreal_net_select (
		struct file *file,
		struct poll_table_struct *table)
{
	unsigned int retval = 0;
	struct nd_struct *nd  = file->private_data;

	if (!nd)
	{
		DBGPRINT(MX_DEBUG_ERROR, "nd is null\n");
		return retval;
	}

	poll_wait( file, &nd->select_in_wait, table );
	poll_wait( file, &nd->select_out_wait, table );
	poll_wait( file, &nd->select_ex_wait, table );

	if ( nd->tx_ready )
	{
		retval |= POLLIN | POLLRDNORM;
	}

	if ( nd->rx_ready )
		retval |= POLLOUT | POLLWRNORM;

	if ( nd->cmd_ready )
	{
		retval |= POLLPRI;
	}


	return retval;
}

static long
npreal_net_ioctl (
		struct file	*file,
		unsigned int	cmd,
		unsigned long 	arg )
{
	struct nd_struct *nd  = file->private_data;
	int    rtn  = 0;
	int    size,len;

	if ( !nd )
	{
		rtn = -ENXIO;
		goto done;
	}

	size = _IOC_SIZE( cmd );
	switch (_IOC_NR(cmd))
	{
	case NPREAL_NET_CMD_RETRIEVE :
		if (!nd->cmd_ready)
		{
			rtn = -ENXIO;
			goto done;
		}
		if (nd->flag & NPREAL_NET_DO_SESSION_RECOVERY)
			len = nd->do_session_recovery_len;
		else
			len = (int)nd->cmd_buffer[2] + 3;
#if (LINUX_VERSION_CODE >= VERSION_CODE(5,0,0))    
		rtn = access_ok((void *)arg, len)?0:-EFAULT;
#else
		rtn = access_ok(VERIFY_WRITE, (void *)arg, len)?0:-EFAULT;
#endif
		if ( rtn )
		{
			goto done;
		}
		if (copy_to_user( (void *)arg, (void *)nd->cmd_buffer,len ))
		{
			rtn = -EFAULT;
			goto done;
		}
		nd->cmd_buffer[0] = 0;
		rtn = len;
		nd->cmd_ready = 0;
		break;

	case NPREAL_NET_CMD_RESPONSE :
	{
		unsigned char rsp_buffer[84];

		if (size < 2)
			goto done;
		if (size > 84)
			size = 84;

#if (LINUX_VERSION_CODE >= VERSION_CODE(5,0,0))    
		rtn = access_ok( (void *)arg, size )?0:-EFAULT;
#else
		rtn = access_ok( VERIFY_READ,  (void *)arg, size )?0:-EFAULT;
#endif
		if ( rtn )
		{
			goto done;
		}
		if (copy_from_user( (void *)rsp_buffer, (void *)arg,size))
		{
			rtn = -EFAULT;
			goto done;
		}

		if (rsp_buffer[0] == NPREAL_LOCAL_COMMAND_SET)
		{
			down(&nd->cmd_semaphore);
			memcpy(nd->rsp_buffer,rsp_buffer,size);
			nd->rsp_length = size;
			up(&nd->cmd_semaphore);
			if ( waitqueue_active(&nd->cmd_rsp_wait)){
				nd->cmd_rsp_flag = 1;
				wake_up_interruptible(&nd->cmd_rsp_wait);
			}
			break;
		}

		down(&nd->cmd_semaphore);
		if (nd->flag & NPREAL_NET_DO_SESSION_RECOVERY)
		{
			if (rsp_buffer[1] == ASPP_CMD_LINECTRL)
			{
				nd->flag &= ~ NPREAL_NET_DO_SESSION_RECOVERY;
				up(&nd->cmd_semaphore);
				break;
			}
			else if (rsp_buffer[1] == ASPP_CMD_PORT_INIT)
			{
				int state = 0;
				struct npreal_struct *info;

				up(&nd->cmd_semaphore);
				if (!(info=nd->tty_node))
					break;
				if (size != 6)
					break;
				if (rsp_buffer[2] != 3)
					break;
				if (rsp_buffer[3])
					state |= UART_MSR_DSR;
				if (rsp_buffer[4])
					state |= UART_MSR_CTS;
				if (rsp_buffer[5])
					state |= UART_MSR_DCD;
				npreal_check_modem_status(info,state);
			}
			else
			{
				up(&nd->cmd_semaphore);
				break;
			}
		}
		else
			up(&nd->cmd_semaphore);
		if (rsp_buffer[1] == ASPP_NOTIFY)
		{
			npreal_process_notify(nd,rsp_buffer,size);
		}
		else if (rsp_buffer[1] == ASPP_CMD_WAIT_OQUEUE)
		{
			if (size == 5)
			{
				memcpy(nd->rsp_buffer,rsp_buffer,size);
				nd->oqueue = rsp_buffer[4]*16 + rsp_buffer[3];
				nd->rsp_length = size;
				nd->wait_oqueue_responsed = 1;
				if ( waitqueue_active(&nd->cmd_rsp_wait)){
					nd->cmd_rsp_flag = 1;
					wake_up_interruptible(&nd->cmd_rsp_wait);
				}
			}
		}
		else
		{
			down(&nd->cmd_semaphore);
			memcpy(nd->rsp_buffer,rsp_buffer,size);
			nd->rsp_length = size;
			up(&nd->cmd_semaphore);
			if ( waitqueue_active(&nd->cmd_rsp_wait)){
				nd->cmd_rsp_flag = 1;
				wake_up_interruptible(&nd->cmd_rsp_wait);
			}
		}

		break;
	}
	case NPREAL_NET_CONNECTED :
	{
		struct npreal_struct *info;

		if (!(info=nd->tty_node))
			break;
		if (nd->flag & NPREAL_NET_NODE_DISCONNECTED)
		{
			nd->flag &= ~NPREAL_NET_NODE_DISCONNECTED;
			nd->flag |= NPREAL_NET_NODE_CONNECTED;
			npreal_do_session_recovery(info);
		}
		break;
	}
	case NPREAL_NET_DISCONNECTED :
		nd->flag &= ~NPREAL_NET_NODE_CONNECTED;
		nd->flag |= NPREAL_NET_NODE_DISCONNECTED;
		nd->flag &= ~NPREAL_NET_TTY_INUSED;  //clean inused flag
		if (waitqueue_active(&nd->cmd_rsp_wait)){
			nd->wait_oqueue_responsed = 1;
			nd->cmd_rsp_flag = 1;
			wake_up_interruptible(&nd->cmd_rsp_wait);
		}
		break;

	case NPREAL_NET_GET_TTY_STATUS:
	{
		int	status;

		if (size != sizeof (status))
			goto done;

#if (LINUX_VERSION_CODE >= VERSION_CODE(5,0,0))    
		rtn = access_ok( (void *)arg, size )?0:-EFAULT;
#else
		rtn = access_ok( VERIFY_READ,  (void *)arg, size )?0:-EFAULT;
#endif
		if ( rtn )
		{
			goto done;
		}
		status = (nd->flag & NPREAL_NET_TTY_INUSED) ? 1 : 0;
		if (copy_to_user( (void *)arg, (void *)&status,size ))
		{
			rtn = -EFAULT;
			goto done;
		}
		break;

	}
	case NPREAL_NET_SETTING :
	{
		struct server_setting_struct settings;
		struct npreal_struct *info;

		if (!(info=nd->tty_node))
			break;

		if (size != sizeof (struct server_setting_struct))
			goto done;

#if (LINUX_VERSION_CODE >= VERSION_CODE(5,0,0))    
		rtn = access_ok( (void *)arg, size )?0:-EFAULT;
#else
		rtn = access_ok( VERIFY_READ,  (void *)arg, size )?0:-EFAULT;
#endif
		if ( rtn )
		{
			goto done;
		}
		if (copy_from_user( (void *)&settings, (void *)arg,size))
		{
			rtn = -EFAULT;
			goto done;
		}
		if ((settings.server_type == DE311) ||
				(settings.server_type == DE301) ||
				(settings.server_type == DE302) ||
				(settings.server_type == DE304) ||
				(settings.server_type == DE331) ||
				(settings.server_type == DE332) ||
				(settings.server_type == DE334) ||
				(settings.server_type == DE303) ||
				(settings.server_type == DE308) ||
				(settings.server_type == DE309) ||
				(settings.server_type == CN2100) ||
				(settings.server_type == CN2500))
			nd->server_type =  settings.server_type;

		if (settings.disable_fifo)
			info->type = PORT_16450;
		else
			info->type = PORT_16550A;

		if (info->type == PORT_16550A)
		{
			if (nd->server_type == CN2500)
				info->xmit_fifo_size = 64;
			else
				info->xmit_fifo_size = 16;
		}
		else
			info->xmit_fifo_size = 1;

		break;
	}
	default :
		break;
	}
	done:

	return rtn;
}
/*
 * read data form kernel-layer to proc file system. (files in this folder /proc/npreal2)
 */
static ssize_t
npreal_net_read (
		struct file *file,
		char *buf,
		size_t count,
		loff_t *ppos )
{
	struct nd_struct *nd  = file->private_data;
	ssize_t  rtn = 0;
	int	 cnt;
	struct npreal_struct *info;
	unsigned long   flags;
	struct tty_struct *	tty;

	/*
	 *  Get the node pointer, and quit if it doesn't exist.
	 */

	if ( !nd )
	{
		rtn = -ENXIO;
		goto done;
	}

	if (!(info = (struct npreal_struct *)nd->tty_node))
	{
		rtn = -ENXIO;
		goto done;
	}

	tty = info->tty;
	if ( !tty )
	{
		rtn = -ENXIO;
		goto done;
	}

	if ( info->x_char )
	{
		rtn = 1;
		if (copy_to_user( buf, &info->x_char,rtn ))
		{
			rtn = -EFAULT;
			goto done;
		}
		info->x_char = 0;
		DOWN(info->tx_lock, flags);
		info->icount.tx++;
		UP(info->tx_lock, flags);
		goto done;
	}

	DOWN(info->tx_lock, flags);
	if (!info->xmit_buf || info->xmit_cnt <= 0)
	{
		rtn = 0;
		UP(info->tx_lock, flags);
		goto done;
	}
	UP(info->tx_lock, flags);

	while ( count )
	{
		cnt = MIN(count, MIN( info->xmit_cnt,
				SERIAL_XMIT_SIZE - info->xmit_tail));
		if ( cnt <= 0 )
			break;
		if (copy_to_user( buf+rtn,info->xmit_buf + info->xmit_tail,cnt ))
		{
			rtn = -EFAULT;
			goto done;
		}
		rtn += cnt;
		count -= cnt;

		DOWN(info->tx_lock, flags);
		info->xmit_cnt -= cnt;
		info->xmit_tail += cnt;
		info->xmit_tail = info->xmit_tail & (SERIAL_XMIT_SIZE - 1);
		info->icount.tx += cnt;
		UP(info->tx_lock, flags);
	}
	if (info->xmit_cnt <= 0)
	{
		nd->tx_ready = 0;
	}
	else
	{   /* if not read all, wake up again to read.*/
		nd->tx_ready = 1;
	if ( waitqueue_active(&nd->select_in_wait))
		wake_up_interruptible( &nd->select_in_wait );
	}

	// Scott: 2005-09-14
	// Comment out the following code to prevent softirq from happening.
#if 0
	if ( info->xmit_cnt < WAKEUP_CHARS )
	{
		set_bit(NPREAL_EVENT_TXLOW,&info->event);
		MXQ_TASK(&info->tqueue);
	}
#else
	if ( (tty->flags & (1 << TTY_DO_WRITE_WAKEUP)) &&
			tty->ldisc->ops->write_wakeup )
		(tty->ldisc->ops->write_wakeup)(tty);
	wake_up_interruptible(&tty->write_wait);
#endif
	done:

	return rtn;
}
/*
 * read data form proc file system (files in this folder /proc/npreal2) to kernel,
 * and push data to tty driver let AP-layer to read data.
 */
static ssize_t
npreal_net_write (
		struct file *file,
		const char *buf,
		size_t count,
		loff_t *ppos)
{
	struct nd_struct *nd  = file->private_data;
	ssize_t  rtn = 0;
	int cnt;
	struct npreal_struct *info;
	struct tty_struct *	tty;
	unsigned char * k_buf = NULL;

	/*
	 *  Get the node pointer, and quit if it doesn't exist.
	 */
	if ( !buf )
	{
		rtn = count; /* throw it away*/
		goto done;
	}

	k_buf = kmalloc(sizeof(unsigned char) * count, GFP_ATOMIC);

	if( k_buf==NULL ){
		rtn = count; /* throw it away*/
		goto done;
	}

	if(copy_from_user(k_buf, buf, count)){
		rtn = count;
		goto done; /* throw it away*/
	}

	if ( !nd )
	{
		rtn = count; /* throw it away*/
		goto done;
	}

	if (!(info = (struct npreal_struct *)nd->tty_node))
	{
		rtn = count; /* throw it away*/
		goto done;
	}

	if (info->flags & ASYNC_CLOSING)
	{
		rtn = count; /* throw it away*/
		goto done;
	}

#if 0
	if(!info->tty->low_latency)
	{
		rtn = count; /* throw it away*/
		goto done;
	}
#endif

	down(&info->rx_semaphore);

	if (!(tty = info->tty))
	{
		rtn = count; /* throw it away*/
		up(&info->rx_semaphore);
		goto done;
	}
#if (LINUX_VERSION_CODE < VERSION_CODE(3,9,0))
	if(!info->tty->low_latency)
#else
		if(!info->ttyPort.low_latency)
#endif /* 3,9,0 */
		{
			rtn = count; /* throw it away*/
			up(&info->rx_semaphore);
			goto done;
		}

	if (test_bit(TTY_IO_ERROR, &tty->flags))
	{
		rtn = count; /* throw it away*/
		up(&info->rx_semaphore);
		goto done;
	}

	if ( !nd->rx_ready )
	{
		up(&info->rx_semaphore);
		DBGPRINT(MX_DEBUG_TRACE, "Port %d RX is not ready\n", info->port);
		goto done;
	}

	/*  The receive buffer will overrun,as the TTY_THRESHOLD_THROTTLE is 128*/
#if (LINUX_VERSION_CODE < VERSION_CODE(3,9,0))           	
	if ((cnt = tty_buffer_request_room(tty, count)) <= 0)
#else
	if ((cnt = tty_buffer_request_room(&info->ttyPort, count)) <= 0)
#endif
	{

		/*
		 * Doing throttle here,because that it will spent times
		 * for upper layer driver to throttle and the application
		 * may call write so many times but just can not write.
		 * If we are doing input canonicalization, and there are no
		 * pending newlines, let characters through without limit, so
		 * that erase characters will be handled.  Other excess
		 * characters will be beeped.
		 */

#if (LINUX_VERSION_CODE < VERSION_CODE(3,8,0))   
		if (!tty->icanon || tty->canon_data)
		{
			if (!test_and_set_bit(TTY_THROTTLED,&tty->flags))
			{
				npreal_throttle(tty);
			}
		}
#endif
		up(&info->rx_semaphore);
		goto done;
	}

#if (LINUX_VERSION_CODE < VERSION_CODE(3,8,0))
	if (!tty->icanon || tty->canon_data)
	{
#if (LINUX_VERSION_CODE < VERSION_CODE(3,8,0))    
		if ((cnt = MIN(cnt,(N_TTY_BUF_SIZE-1) - tty->read_cnt )) <= 0)
#else
		if ((cnt = MIN(cnt,(N_TTY_BUF_SIZE-1) - ldata->read_cnt )) <= 0)
#endif
		{
			/*
			 * Doing throttle here,because that it will spent times
			 * for upper layer driver to throttle and the application
			 * may call write so many times but just can not write.
			 * If we are doing input canonicalization, and there are no
			 * pending newlines, let characters through without limit, so
			 * that erase characters will be handled.  Other excess
			 * characters will be beeped.
			 */
			if (!test_and_set_bit(TTY_THROTTLED,&tty->flags))
			{
				npreal_throttle(tty);
			}
			up(&info->rx_semaphore);

			goto done;
		}
	}
#endif
	/*
	 * if kernel version > 2.6.15, push buffer to let AP-layer read data.
	 * if kernel version <= 2.6.15, copy data form 'fs', and set tty driver count to let AP-later read data.
	 */
#if (LINUX_VERSION_CODE < VERSION_CODE(3,9,0))
	if ((count = tty_insert_flip_string(tty, (unsigned char *)k_buf, cnt)))
#else
	if ((count = tty_insert_flip_string(&info->ttyPort, (unsigned char *)k_buf, cnt)))
#endif /* 3,9,0 */
	{
#if (LINUX_VERSION_CODE < VERSION_CODE(3,9,0))
		tty_flip_buffer_push(tty);
#else
		tty_flip_buffer_push(&info->ttyPort);
#endif /* 3,9,0 */
		rtn = count; /* throw it away*/
		up(&info->rx_semaphore);
		goto done;
	}

	up(&info->rx_semaphore);
	MXQ_TASK(&info->process_flip_tqueue);

done:
	if( k_buf ){
		kfree( k_buf );
	}

	return rtn;
}
/*
 * set ASPP command
 */
static int
npreal_wait_and_set_command(
		struct nd_struct *nd,
		char command_set,
		char command)
{

	unsigned long	et;

	if ((command_set != NPREAL_LOCAL_COMMAND_SET)&&((nd->flag & NPREAL_NET_DO_SESSION_RECOVERY)||(nd->flag&NPREAL_NET_NODE_DISCONNECTED)))
	{
		return (-1);
	}

	nd->cmd_rsp_flag = 0;

	et = jiffies + NPREAL_CMD_TIMEOUT;
	while (1)
	{
		down (&nd->cmd_semaphore);
		if (nd->cmd_buffer[0] == 0)
		{
			nd->cmd_buffer[0] = command_set;
			nd->cmd_buffer[1] = command;
			up (&nd->cmd_semaphore);
			return (0);
		}
		else if ((jiffies >= et)||signal_pending(current))
		{ // timeout
			nd->cmd_buffer[0] = command_set;
			nd->cmd_buffer[1] = command;
			up (&nd->cmd_semaphore);
			return (0);
		}
		else
		{
			up (&nd->cmd_semaphore);
			current->state = TASK_INTERRUPTIBLE;
			schedule_timeout(1);
		}
	}
}
/*
 * wait for command set over.
 */
static int
npreal_wait_command_completed(
		struct nd_struct *nd,
		char command_set,
		char command,
		long timeout,
		char *rsp_buf,
		int  *rsp_len)
{
	long	st = 0;
	long	tmp_t = 0;

	if ((command_set != NPREAL_LOCAL_COMMAND_SET)&&((nd->flag & NPREAL_NET_DO_SESSION_RECOVERY)||(nd->flag&NPREAL_NET_NODE_DISCONNECTED)))
	{
		return (-1);
	}

	if (*rsp_len <= 0)
		return (-1);

	while (1)
	{
		down(&nd->cmd_semaphore);

		if ((nd->rsp_length)&&(nd->rsp_buffer[0] == command_set)&&(nd->rsp_buffer[1] == command))
		{
			if (nd->rsp_length > *rsp_len)
				return (-1);
			*rsp_len = nd->rsp_length;
			memcpy(rsp_buf,nd->rsp_buffer,*rsp_len);
			nd->rsp_length = 0;
			up(&nd->cmd_semaphore);
			return (0);
		}
		else if ( timeout > 0)
		{
			up(&nd->cmd_semaphore);
			if ( signal_pending(current) )
			{
				return(-1);
			}

			st = jiffies;

#if (LINUX_VERSION_CODE >= VERSION_CODE(3,15,0))
			if( wait_event_interruptible_timeout(nd->cmd_rsp_wait, nd->cmd_rsp_flag==1, timeout)!=0 ){
				down(&nd->cmd_semaphore);
				nd->cmd_rsp_flag = 0;
				up(&nd->cmd_semaphore);
			}
#else
			interruptible_sleep_on_timeout(&nd->cmd_rsp_wait,timeout);
#endif

			tmp_t = _get_delta_giffies(st);

// If the past delta excees timeout, finish waiting.
			if( tmp_t >= timeout ){
				timeout = 0;
			} else {
				timeout -= tmp_t;
			}

		}
		else
		{ // timeout
			up(&nd->cmd_semaphore);
			return (-1);
		}
	}
}
// Notify event
static void
npreal_process_notify(
		struct nd_struct *nd,
		char *rsp_buffer,
		int rsp_length)
{
	int	state;
	struct npreal_struct	*info = nd->tty_node;

	if (!info)
		return;
	if (rsp_length != 5)
		return;
	// MSR field
	if (rsp_buffer[2] & ASPP_NOTIFY_MSR_CHG)
	{
		state = 0;
		if (rsp_buffer[3] & 0x10)
			state |= UART_MSR_CTS;
		if (rsp_buffer[3] & 0x20)
			state |= UART_MSR_DSR;
		if (rsp_buffer[3] & 0x80)
			state |= UART_MSR_DCD;
		npreal_check_modem_status(info,state);

	}
	if (rsp_buffer[2] & ASPP_NOTIFY_BREAK)
	{
		struct tty_struct	*tty;

		down (&info->rx_semaphore);
		if (!(tty= info->tty))
		{
			up (&info->rx_semaphore);
			return;
		}
#if (LINUX_VERSION_CODE < VERSION_CODE(3,9,0))
		if(!tty->low_latency)
#else
			if(!info->ttyPort.low_latency)
#endif /* 3,9,0 */
			{
				up (&info->rx_semaphore);
				return;
			}
#if (LINUX_VERSION_CODE < VERSION_CODE(3,9,0))
		tty_insert_flip_char(tty, 0, TTY_BREAK);
#else
		tty_insert_flip_char(&info->ttyPort, 0, TTY_BREAK);
#endif
		up (&info->rx_semaphore);
		info->icount.rx ++;

		info->icount.brk++;
		MXQ_TASK(&info->process_flip_tqueue);

		if ( info->flags & ASYNC_SAK )
		{
			do_SAK(info->tty);
		}
	}
	if (rsp_buffer[2] & ASPP_NOTIFY_PARITY)
		info->icount.parity++;
	if (rsp_buffer[2] & ASPP_NOTIFY_FRAMING)
		info->icount.frame++;
	if ((rsp_buffer[2] & ASPP_NOTIFY_SW_OVERRUN) ||
			(rsp_buffer[2] & ASPP_NOTIFY_HW_OVERRUN))
		info->icount.overrun++;

}

static void
npreal_do_session_recovery(struct npreal_struct *info)
{
	struct tty_struct *	tty;
	struct nd_struct *	nd;
	struct ktermios *	termio;
	int32_t    		    baud,mode;
	int		baudIndex,index;

	tty = info->tty;
	nd = info->net_node;

	if ( !tty || !nd)
		return;

	if (!(nd->flag & NPREAL_NET_NODE_OPENED))
		return;
	if (!(nd->flag & NPREAL_NET_NODE_CONNECTED))
		return;
	if (info->flags & ASYNC_INITIALIZED)
	{
#if (LINUX_VERSION_CODE < VERSION_CODE(3,7,0)) 	    
		if (!(termio = info->tty->termios))
			return;
#else
		termio = &(info->tty->termios);
#endif
	}
	else
	{
		if (!(termio = &info->normal_termios))
			return;
	}

	down (&nd->cmd_semaphore);
	mode = termio->c_cflag & CSIZE;
	if (mode == CS5)
		mode = ASPP_IOCTL_BITS5;
	else if (mode == CS6)
		mode = ASPP_IOCTL_BITS6;
	else if (mode == CS7)
		mode = ASPP_IOCTL_BITS7;
	else if (mode == CS8)
		mode = ASPP_IOCTL_BITS8;

	if (termio->c_cflag & CSTOPB)
		mode |= ASPP_IOCTL_STOP2;
	else
		mode |= ASPP_IOCTL_STOP1;

	if (termio->c_cflag & PARENB)
	{
		if (termio->c_cflag & PARODD)
			mode |= ASPP_IOCTL_ODD;
		else
			mode |= ASPP_IOCTL_EVEN;
	}
	else
		mode |= ASPP_IOCTL_NONE;

	switch ( termio->c_cflag & (CBAUD|CBAUDEX))
	{
	case B921600:
		baud = 921600L;
		baudIndex = ASPP_IOCTL_B921600;
		break;
	case B460800:
		baud = 460800;
		baudIndex = ASPP_IOCTL_B460800;
		break;
	case B230400:
		baud = 230400L;
		baudIndex = ASPP_IOCTL_B230400;
		break;
	case B115200:
		baud = 115200L;
		baudIndex = ASPP_IOCTL_B115200;
		break;
	case B57600:
		baud = 57600L;
		baudIndex = ASPP_IOCTL_B57600;
		break;
	case B38400:
		baud = 38400L;
		baudIndex = ASPP_IOCTL_B38400;
		if ( (info->flags & ASYNC_SPD_MASK) == ASYNC_SPD_HI )
		{
			baud = 57600L;
			baudIndex = ASPP_IOCTL_B57600;
		}
		if ( (info->flags & ASYNC_SPD_MASK) == ASYNC_SPD_VHI )
		{
			baud = 115200L;
			baudIndex = ASPP_IOCTL_B115200;
		}

#ifdef ASYNC_SPD_SHI
		if ((info->flags & ASYNC_SPD_MASK) == ASYNC_SPD_SHI)
		{
			baud = 230400L;
			baudIndex = ASPP_IOCTL_B230400;
		}
#endif

#ifdef ASYNC_SPD_WARP
		if ((info->flags & ASYNC_SPD_MASK) == ASYNC_SPD_WARP)
		{
			baud = 460800L;
			baudIndex = ASPP_IOCTL_B460800;
		}
#endif
break;
	case B19200:
		baud = 19200L;
		baudIndex = ASPP_IOCTL_B19200;
		break;
	case B9600:
		baud = 9600L;
		baudIndex = ASPP_IOCTL_B9600;
		break;
	case B4800:
		baud = 4800L;
		baudIndex = ASPP_IOCTL_B4800;
		break;
	case B2400:
		baud = 2400L;
		baudIndex = ASPP_IOCTL_B2400;
		break;
	case B1800:
		baud = 1800L;
		baudIndex = 0xff;
		break;
	case B1200:
		baud = 1200L;
		baudIndex = ASPP_IOCTL_B1200;
		break;
	case B600:
		baud = 600L;
		baudIndex = ASPP_IOCTL_B600;
		break;
	case B300:
		baud = 300L;
		baudIndex = ASPP_IOCTL_B300;
		break;
	case B200:
		baud = 200L;
		baudIndex = 0xff;
		break;
	case B150:
		baud = 150L;
		baudIndex = ASPP_IOCTL_B150;
		break;
	case B134:
		baud = 134L;
		baudIndex = ASPP_IOCTL_B134;
		break;
	case B110:
		baud = 110L;
		baudIndex = ASPP_IOCTL_B110;
		break;
	case B75:
		baud = 75L;
		baudIndex = ASPP_IOCTL_B75;
		break;
	case B50:
		baud = 50L;
		baudIndex = 0xff;
		break;
		break;
	default:
		baud = 0;
		baudIndex = 0xff;
	}
#ifdef ASYNC_SPD_CUST
	if ((info->flags & ASYNC_SPD_MASK) == ASYNC_SPD_CUST)
	{
		baudIndex = 0xff;
	}
#endif

	nd->cmd_buffer[2] = 8;
	//
	// baud rate
	//
	nd->cmd_buffer[3] = baudIndex;
	//
	// mode
	//
	nd->cmd_buffer[4] = mode;
	//
	// line control
	//
	if (info->modem_control & UART_MCR_DTR)
		nd->cmd_buffer[5] = 1;
	else
		nd->cmd_buffer[5] = 0;
	if (info->modem_control & UART_MCR_RTS)
		nd->cmd_buffer[6] = 1;
	else
		nd->cmd_buffer[6] = 0;
	//
	// flow control
	//
	if (info->flags & ASYNC_INITIALIZED)
	{
		if (termio->c_cflag & CRTSCTS)
		{
			nd->cmd_buffer[7] = 1;
			nd->cmd_buffer[8] = 1;
		}
		else
		{
			nd->cmd_buffer[7] = 0;
			nd->cmd_buffer[8] = 0;
		}
	}
	else
	{
		nd->cmd_buffer[7] = 0;
		nd->cmd_buffer[8] = 0;

	}

	if (termio->c_iflag & IXON)
		nd->cmd_buffer[9] = 1;
	else
		nd->cmd_buffer[9] = 0;
	if (termio->c_iflag & IXOFF)
		nd->cmd_buffer[10] = 1;
	else
		nd->cmd_buffer[10] = 0;

	nd->cmd_buffer[0] = NPREAL_ASPP_COMMAND_SET;
	nd->cmd_buffer[1] = ASPP_CMD_PORT_INIT;
	index = 11;

	if ((baudIndex == 0xff)&&(baud != 0))
	{
		nd->cmd_buffer[index+0] = ASPP_CMD_SETBAUD;
		nd->cmd_buffer[index+1] = 4;
		if((info->flags & ASYNC_SPD_MASK) == ASYNC_SPD_CUST)
		{
			if (info->custom_divisor)
				baud=info->baud_base/info->custom_divisor;
		}
		memcpy(&nd->cmd_buffer[index+2],&baud,4);
		index += 6;
	}

	if (termio->c_iflag & (IXON | IXOFF))
	{
		nd->cmd_buffer[index+0] = ASPP_CMD_XONXOFF;
		nd->cmd_buffer[index+1] = 2;
		nd->cmd_buffer[index+2] = termio->c_cc[VSTART];
		nd->cmd_buffer[index+3] = termio->c_cc[VSTOP];
		index += 4;
	}
	nd->cmd_buffer[index+0] = ASPP_CMD_TX_FIFO;
	nd->cmd_buffer[index+1] = 1;
	nd->cmd_buffer[index+2] = info->xmit_fifo_size;
	index += 3;

	nd->cmd_buffer[index+0] = ASPP_CMD_LINECTRL;
	nd->cmd_buffer[index+1] = 2;
	if (info->modem_control & UART_MCR_DTR)
		nd->cmd_buffer[index+2] = 1;
	else
		nd->cmd_buffer[index+2] = 0;
	if (info->modem_control & UART_MCR_RTS)
		nd->cmd_buffer[index+3] = 1;
	else
		nd->cmd_buffer[index+3] = 0;
	index += 4;

	nd->do_session_recovery_len = index;
	nd->flag |= NPREAL_NET_DO_SESSION_RECOVERY;
	nd->cmd_ready = 1;
	up (&nd->cmd_semaphore);

	if ( waitqueue_active(&nd->select_ex_wait))
	{
		wake_up_interruptible( &nd->select_ex_wait );
	}
	return;

}

static long
npreal_wait_oqueue(
		struct npreal_struct * info,
		long timeout)
{
	struct	nd_struct	*nd;
	long	st = 0;
	long	tmp_t = 0;
	uint32_t    tout;

	if (!(nd = info->net_node))
		return (-EIO);
	if (npreal_wait_and_set_command(nd,NPREAL_ASPP_COMMAND_SET,ASPP_CMD_WAIT_OQUEUE) < 0)
		return (-EIO);
	if (timeout < HZ/10)  // at least wait for 100 ms
		timeout = HZ/10;

	st = jiffies;

	if (timeout != MAX_SCHEDULE_TIMEOUT)
		tout = (uint32_t)timeout;
	else
		tout = 0x7FFFFFFF;

	nd->cmd_buffer[2] = 4;
	memcpy(&nd->cmd_buffer[3],(void *)&tout,4);
	nd->wait_oqueue_responsed = 0;
	nd->cmd_ready = 1;
	if ( waitqueue_active(&nd->select_ex_wait))
		wake_up_interruptible( &nd->select_ex_wait );
	while (nd->cmd_ready == 1)
	{
#if (LINUX_VERSION_CODE >= VERSION_CODE(3,15,0))
		if( wait_event_interruptible_timeout(nd->cmd_rsp_wait, nd->cmd_rsp_flag==1, 1) != 0 ){
			nd->cmd_rsp_flag = 0;
		}
#else
		interruptible_sleep_on_timeout(&nd->cmd_rsp_wait,1);
#endif

		tmp_t = _get_delta_giffies(st);

		if( tmp_t > timeout )
			return (-EIO);

	}

	//Why do we increase the timer?
	//if (timeout != MAX_SCHEDULE_TIMEOUT)
	//    timeout += 10;

	nd->cmd_buffer[0] = 0;
	do
	{
		if (nd->wait_oqueue_responsed == 0)
		{
#if (LINUX_VERSION_CODE >= VERSION_CODE(3,15,0))
			if(wait_event_interruptible_timeout(nd->cmd_rsp_wait, nd->cmd_rsp_flag==1, timeout)){
				nd->cmd_rsp_flag = 0;
			}
#else
			timeout =
					interruptible_sleep_on_timeout(&nd->cmd_rsp_wait,timeout);
#endif

			tmp_t = _get_delta_giffies(st);

			// If the past delta excees timeout, finish waiting.
			if( tmp_t >= timeout ){
				timeout = 0;
			} else {
				timeout -= tmp_t;
			}

			if (nd->wait_oqueue_responsed)
			{
				return (nd->oqueue);
			}
		}
		else
		{
			return (nd->oqueue);
		}
	}
	while (timeout > 0);
	return (-EIO);
}

static int
npreal_linectrl(
		struct nd_struct *nd,
		int modem_control)
{
	char rsp_buffer[8];
	int  rsp_length = sizeof(rsp_buffer);

	if (!nd)
		return (-EIO);
	if (npreal_wait_and_set_command(nd,NPREAL_ASPP_COMMAND_SET,ASPP_CMD_LINECTRL) < 0)
		return (-EIO);
	nd->cmd_buffer[2] = 2;
	if (modem_control & UART_MCR_DTR)
		nd->cmd_buffer[3] = 1;
	else
		nd->cmd_buffer[3] = 0;
	if (modem_control & UART_MCR_RTS)
		nd->cmd_buffer[4] = 1;
	else
		nd->cmd_buffer[4] = 0;
	nd->cmd_ready = 1;
	if ( waitqueue_active(&nd->select_ex_wait))
	{
		wake_up_interruptible( &nd->select_ex_wait );
	}
	if (npreal_wait_command_completed(nd,
			NPREAL_ASPP_COMMAND_SET,
			ASPP_CMD_LINECTRL,NPREAL_CMD_TIMEOUT,
			rsp_buffer,
			&rsp_length))
		return(-EIO);
	if (rsp_length != 4)
		return(-EIO);
	if ((rsp_buffer[2] != 'O') ||
			(rsp_buffer[3] != 'K') )
		return(-EIO);
	return (0);
}

/*
Scott: 2005-08-11
ASPP command. This command pretend the serial port receives
XON (or XOFF) character.
 */
static int
npreal_setxon_xoff(struct npreal_struct * info, int cmd)
{
	char rsp_buffer[8];
	int  rsp_length = sizeof(rsp_buffer);
	struct	nd_struct	*nd;

	if (!(nd = info->net_node))
		return (-EIO);

	if (npreal_wait_and_set_command(nd,NPREAL_ASPP_COMMAND_SET,cmd) < 0)
		return (-EIO);
	nd->cmd_buffer[2] = 0;
	nd->cmd_ready = 1;
	if ( waitqueue_active(&nd->select_ex_wait))
	{
		wake_up_interruptible( &nd->select_ex_wait );
	}
	if (npreal_wait_command_completed(nd,
			NPREAL_ASPP_COMMAND_SET,
			cmd,NPREAL_CMD_TIMEOUT,
			rsp_buffer,
			&rsp_length))
		return(-EIO);
	if (rsp_length != 4)
		return(-EIO);
	if ((rsp_buffer[2] != 'O') ||
			(rsp_buffer[3] != 'K') )
		return(-EIO);
	return (0);
}

static long
_get_delta_giffies(long base)
{
	long tmp_t = 0;

	tmp_t = jiffies;

	// Check whether jiffies wraps to zero and get the delta jiffies
	if( tmp_t >= base ){
		tmp_t = tmp_t-base;
	} else {
		tmp_t = tmp_t+(MAX_SCHEDULE_TIMEOUT-base);
	}

	return tmp_t;
}


module_init(npreal2_module_init);
module_exit(npreal2_module_exit);
