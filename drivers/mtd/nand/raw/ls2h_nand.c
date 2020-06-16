#include <linux/module.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/dma-mapping.h>
#include <linux/delay.h>
#include <linux/clk.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/rawnand.h>
#include <linux/mtd/partitions.h>
#include <linux/io.h>
#include <linux/slab.h>
#include <linux/irq.h>
#include <asm/dma.h>
#include <loongson-pch.h>

#define LS2H_NAND_CMD_REG		(LS2H_NAND_REG_BASE + 0x0000)
#define LS2H_NAND_ADDR_C_REG		(LS2H_NAND_REG_BASE + 0x0004)
#define LS2H_NAND_ADDR_R_REG		(LS2H_NAND_REG_BASE + 0x0008)
#define LS2H_NAND_TIMING_REG		(LS2H_NAND_REG_BASE + 0x000c)
#define LS2H_NAND_IDL_REG		(LS2H_NAND_REG_BASE + 0x0010)
#define LS2H_NAND_STA_IDH_REG		(LS2H_NAND_REG_BASE + 0x0014)
#define LS2H_NAND_PARAM_REG		(LS2H_NAND_REG_BASE + 0x0018)
#define LS2H_NAND_OP_NUM_REG		(LS2H_NAND_REG_BASE + 0x001c)
#define LS2H_NAND_CSRDY_MAP_REG		(LS2H_NAND_REG_BASE + 0x0020)
#define LS2H_NAND_DMA_ACC_REG		(LS2H_NAND_REG_BASE + 0x0040)

#define DMA_ACCESS_ADDR		LS2H_NAND_DMA_ACC_REG
#define ORDER_REG_ADDR		(CKSEG1ADDR(LS2H_DMA_ORDER_REG_BASE))
#define MAX_BUFF_SIZE		4096
#define NAND_PAGE_SHIFT		12
#define NO_SPARE_ADDRH(x)	((x) >> (32 - (NAND_PAGE_SHIFT - 1 )))
#define NO_SPARE_ADDRL(x)	((x) << (NAND_PAGE_SHIFT - 1))
#define SPARE_ADDRH(x)		((x) >> (32 - (NAND_PAGE_SHIFT)))
#define SPARE_ADDRL(x)		((x) << (NAND_PAGE_SHIFT))
#define ALIGN_DMA(x)		(((x)+ 3)/4)

#define USE_POLL
#ifdef USE_POLL
#undef complete
#define complete(...)
#undef init_completion
#define init_completion(...)
#undef wait_for_completion_timeout
#define wait_for_completion_timeout(...)
#undef request_irq
#define request_irq(...) (0)
#undef free_irq
#define free_irq(...)
#endif

#define CHIP_DELAY_TIMEOUT (2*HZ/10)

#define STATUS_TIME_LOOP_R	30
#define STATUS_TIME_LOOP_WS	100
#define STATUS_TIME_LOOP_WM	60
#define STATUS_TIME_LOOP_E	100

#define NAND_CMD	0x1
#define NAND_ADDRL	0x2
#define NAND_ADDRH	0x4
#define NAND_TIMING	0x8
#define NAND_IDL	0x10
#define NAND_STATUS_IDL	0x20
#define NAND_PARAM	0x40
#define NAND_OP_NUM	0X80
#define NAND_CS_RDY_MAP	0x100

#define DMA_ORDERAD	0x1
#define DMA_SADDR	0x2
#define DMA_DADDR	0x4
#define DMA_LENGTH	0x8
#define DMA_STEP_LENGTH	0x10
#define DMA_STEP_TIMES	0x20
#define DMA_CMD		0x40

#define	_NAND_IDL \
	(*((volatile unsigned int*)(CKSEG1ADDR(LS2H_NAND_IDL_REG))))
#define	_NAND_IDH \
	(*((volatile unsigned int*)(CKSEG1ADDR(LS2H_NAND_STA_IDH_REG))))
#define	_NAND_BASE	CKSEG1ADDR(LS2H_NAND_REG_BASE)
#define	_NAND_SET_REG(x,y) \
		do{*((volatile unsigned int*)(_NAND_BASE+x)) = (y);}while(0)
#define	_NAND_READ_REG(x,y) \
		do{(y) = *((volatile unsigned int*)(_NAND_BASE+x));}while(0)

#define show_data_debug	0
#define show_debug(x,y)	show_debug_msk(x,y)
#define show_debug_msk(x,y) do{ if(show_data_debug) \
	{printk(KERN_ERR "%s:\n",__func__);show_data(x,y);}}while(0)

enum {
	ERR_NONE = 0,
	ERR_DMABUSERR = -1,
	ERR_SENDCMD = -2,
	ERR_DBERR = -3,
	ERR_BBERR = -4,
};

enum {
	STATE_READY = 0,
	STATE_BUSY,
};

struct ls2h_nand_cmdset {
	uint32_t cmd_valid:1;
	uint32_t read:1;
	uint32_t write:1;
	uint32_t erase_one:1;
	uint32_t erase_con:1;
	uint32_t read_id:1;
	uint32_t reset:1;
	uint32_t read_sr:1;
	uint32_t op_main:1;
	uint32_t op_spare:1;
	uint32_t done:1;
	uint32_t resv1:5;	//11-15 reserved
	uint32_t nand_rdy:4;	//16-19
	uint32_t nand_ce:4;	//20-23
	uint32_t resv2:8;	//24-32 reserved
};

struct ls2h_nand_dma_desc {
	uint32_t orderad;
	uint32_t saddr;
	uint32_t daddr;
	uint32_t length;
	uint32_t step_length;
	uint32_t step_times;
	uint32_t cmd;
};

struct ls2h_nand_dma_cmd {
	uint32_t dma_int_mask:1;
	uint32_t dma_int:1;
	uint32_t dma_sl_tran_over:1;
	uint32_t dma_tran_over:1;
	uint32_t dma_r_state:4;
	uint32_t dma_w_state:4;
	uint32_t dma_r_w:1;
	uint32_t dma_cmd:2;
	uint32_t revl:17;
};

struct ls2h_nand_desc {
	uint32_t cmd;
	uint32_t addrl;
	uint32_t addrh;
	uint32_t timing;
	uint32_t idl;		//readonly
	uint32_t status_idh;	//readonly
	uint32_t param;
	uint32_t op_num;
	uint32_t cs_rdy_map;
};

struct ls2h_nand_info {
	struct nand_chip nand_chip;

	struct platform_device *pdev;
	/* MTD data control */
	unsigned int buf_start;
	unsigned int buf_count;
	/* NAND registers */
	void __iomem *mmio_base;
	unsigned int irq;
	struct ls2h_nand_desc nand_regs;
	unsigned int nand_addrl;
	unsigned int nand_addrh;
	unsigned int nand_timing;
	unsigned int nand_op_num;
	unsigned int nand_cs_rdy_map;
	unsigned int nand_cmd;

	/* DMA information */
	struct ls2h_nand_dma_desc dma_regs;
	u64 order_reg_addr;
	unsigned int dma_orderad;
	unsigned int dma_saddr;
	unsigned int dma_daddr;
	unsigned int dma_length;
	unsigned int dma_step_length;
	unsigned int dma_step_times;
	unsigned int dma_cmd;
	u64 drcmr_dat;	/* dma descriptor address */
	dma_addr_t drcmr_dat_phys;
	size_t drcmr_dat_size;
	unsigned char *data_buff;	/* dma data buffer */
	dma_addr_t data_buff_phys;
	size_t data_buff_size;
	unsigned long cac_size;
	unsigned long num;
	unsigned long size;
	struct timer_list test_timer;
	u64 dma_ask;
	dma_addr_t dma_ask_phy;

	/* relate to the command */
	unsigned int state;
	size_t data_size;	/* data size in FIFO */
	unsigned int cmd;
	unsigned int page_addr;
	struct completion cmd_complete;
	unsigned int seqin_column;
	unsigned int seqin_page_addr;
	unsigned int timing_flag;
	unsigned int timing_val;
};

/* this isn't used unless ecc.
static struct nand_ecclayout hw_largepage_ecclayout = {
	.eccbytes = 24,
	.eccpos = {
		   40, 41, 42, 43, 44, 45, 46, 47,
		   48, 49, 50, 51, 52, 53, 54, 55,
		   56, 57, 58, 59, 60, 61, 62, 63},
	.oobfree = {{2, 38}}
};
*/

static void show_data(void *base, int num)
{
	int i = 0;
	unsigned char *arry = (unsigned char *)base;
	for (i = 0; i < num; i++) {
		if (!(i % 32)) {
			printk(KERN_ERR "\n");
		}
		if (!(i % 16)) {
			printk("  ");
		}
		printk("%02x ", arry[i]);
	}
	printk(KERN_ERR "\n");
}

static inline struct ls2h_nand_info *mtd_to_ls2h_nand(struct mtd_info *mtd)
{
	return container_of(mtd_to_nand(mtd), struct ls2h_nand_info, nand_chip);
}

static int ls2h_nand_init_buff(struct ls2h_nand_info *info)
{
	struct platform_device *pdev = info->pdev;
	info->data_buff = dma_alloc_coherent(&pdev->dev, MAX_BUFF_SIZE,
					     &info->data_buff_phys, GFP_KERNEL);
	if (info->data_buff == NULL) {
		dev_err(&pdev->dev, "failed to allocate dma buffer\n");
		return -ENOMEM;
	}
	info->data_buff_size = MAX_BUFF_SIZE;
	return 0;
}

static int ls2h_nand_ecc_calculate(struct mtd_info *mtd,
				   const uint8_t * dat, uint8_t * ecc_code)
{
	return 0;
}

static int ls2h_nand_ecc_correct(struct mtd_info *mtd,
				 uint8_t * dat, uint8_t * read_ecc,
				 uint8_t * calc_ecc)
{
	/*
	 * Any error include ERR_SEND_CMD, ERR_DBERR, ERR_BUSERR, we
	 * consider it as a ecc error which will tell the caller the
	 * read fail We have distinguish all the errors, but the
	 * nand_read_ecc only check this function return value
	 */
	return 0;
}

static void ls2h_nand_ecc_hwctl(struct mtd_info *mtd, int mode)
{
	return;
}

static int ls2h_nand_waitfunc(struct mtd_info *mtd, struct nand_chip *this)
{
	udelay(50);
	return 0;
}

static void ls2h_nand_select_chip(struct mtd_info *mtd, int chip)
{
	return;
}

static int ls2h_nand_dev_ready(struct mtd_info *mtd)
{
	return 1;
}

static void ls2h_nand_read_buf(struct mtd_info *mtd, uint8_t * buf, int len)
{
	struct ls2h_nand_info *info = mtd_to_ls2h_nand(mtd);
	int real_len = min_t(size_t, len, info->buf_count - info->buf_start);

	memcpy(buf, info->data_buff + info->buf_start, real_len);
	show_debug(info->data_buff, 0x40);

	info->buf_start += real_len;
}

static u16 ls2h_nand_read_word(struct mtd_info *mtd)
{
	u16 retval = 0xFFFF;
	struct ls2h_nand_info *info = mtd_to_ls2h_nand(mtd);

	if (!(info->buf_start & 0x1) && info->buf_start < info->buf_count) {
		retval = *(u16 *) (info->data_buff + info->buf_start);
	}
	info->buf_start += 2;

	return retval;
}

static uint8_t ls2h_nand_read_byte(struct mtd_info *mtd)
{
	char retval = 0xFF;
	struct ls2h_nand_info *info = mtd_to_ls2h_nand(mtd);

	if (info->buf_start < info->buf_count)
		retval = info->data_buff[(info->buf_start)++];
	return retval;
}

static void ls2h_nand_write_buf(struct mtd_info *mtd, const uint8_t * buf,
				int len)
{
	struct ls2h_nand_info *info = mtd_to_ls2h_nand(mtd);
	int real_len = min_t(size_t, len, info->buf_count - info->buf_start);

	memcpy(info->data_buff + info->buf_start, buf, real_len);
	show_debug(info->data_buff, 0x20);
	info->buf_start += real_len;
}

static void ls2h_nand_cmdfunc(struct mtd_info *mtd, unsigned command,
			      int column, int page_addr);

static void ls2h_nand_init_mtd(struct mtd_info *mtd,
			       struct ls2h_nand_info *info)
{
	struct nand_chip *this = &info->nand_chip;

	this->options = 8;
	this->waitfunc = ls2h_nand_waitfunc;
	this->select_chip = ls2h_nand_select_chip;
	this->dev_ready = ls2h_nand_dev_ready;
	this->cmdfunc = ls2h_nand_cmdfunc;
	this->read_word = ls2h_nand_read_word;
	this->read_byte = ls2h_nand_read_byte;
	this->read_buf = ls2h_nand_read_buf;
	this->write_buf = ls2h_nand_write_buf;

	this->ecc.mode = NAND_ECC_NONE;
	this->ecc.hwctl = ls2h_nand_ecc_hwctl;
	this->ecc.calculate = ls2h_nand_ecc_calculate;
	this->ecc.correct = ls2h_nand_ecc_correct;
	this->ecc.size = 2048;
	this->ecc.bytes = 24;

	mtd->owner = THIS_MODULE;
}

static unsigned ls2h_nand_status(struct ls2h_nand_info *info)
{
	return (*((volatile unsigned int *)CKSEG1ADDR(LS2H_NAND_CMD_REG)) &
		(0x1 << 10));
}

#define write_z_cmd  do{	\
	*((volatile unsigned int *)CKSEG1ADDR(LS2H_NAND_CMD_REG)) = 0;	\
	*((volatile unsigned int *)CKSEG1ADDR(LS2H_NAND_CMD_REG)) = 0;	\
	*((volatile unsigned int *)CKSEG1ADDR(LS2H_NAND_CMD_REG)) = 400;\
}while(0)

/* this isn't used until the bug is fixed up
static irqreturn_t ls2h_nand_irq(int irq, void *devid)
{
	int status_time;
	struct ls2h_nand_info *info = devid;
	switch (info->cmd) {
	case NAND_CMD_READOOB:
	case NAND_CMD_READ0:
		udelay(20);
		info->state = STATE_READY;
		break;
	case NAND_CMD_PAGEPROG:
		status_time = STATUS_TIME_LOOP_WS;
		while (!(ls2h_nand_status(info))) {
			if (!(status_time--)) {
				write_z_cmd;
				break;
			}
			udelay(50);
		}
		info->state = STATE_READY;
		break;
	default:
		break;
	}
	complete(&info->cmd_complete);
	return IRQ_HANDLED;
}
*/

/*
 *	flags & 0x1	orderad
 *	flags & 0x2	saddr
 *	flags & 0x4	daddr
 *	flags & 0x8	length
 *	flags & 0x10	step_length
 *	flags & 0x20	step_times
 *	flags & 0x40	cmd
 ***/
static unsigned char flagsss = 0;

static void dma_setup(unsigned int flags, struct ls2h_nand_info *info)
{
	volatile struct ls2h_nand_dma_desc *dma_base =
		(volatile struct ls2h_nand_dma_desc *)(info->drcmr_dat);
	int status_time;
	dma_base->orderad = (flags & DMA_ORDERAD) == DMA_ORDERAD ?
		info->dma_regs.orderad : info->dma_orderad;
	dma_base->saddr = (flags & DMA_SADDR) == DMA_SADDR ?
		info->dma_regs.saddr : info->dma_saddr;
	dma_base->daddr = (flags & DMA_DADDR) == DMA_DADDR ?
		info->dma_regs.daddr : info->dma_daddr;
	dma_base->length = (flags & DMA_LENGTH) == DMA_LENGTH ?
		info->dma_regs.length : info->dma_length;
	dma_base->step_length = (flags & DMA_STEP_LENGTH) == DMA_STEP_LENGTH ?
		info->dma_regs.step_length : info->dma_step_length;
	dma_base->step_times = (flags & DMA_STEP_TIMES) == DMA_STEP_TIMES ?
		info->dma_regs.step_times : info->dma_step_times;
	dma_base->cmd = (flags & DMA_CMD) == DMA_CMD ?
		info->dma_regs.cmd : info->dma_cmd;
	{
		unsigned long flags;
		local_irq_save(flags);
		*(volatile unsigned int *)info->order_reg_addr =
		    ((unsigned int)info->drcmr_dat_phys) | 0x1 << 3;
		while (*(volatile unsigned int *)info->order_reg_addr & 0x8) ;
#ifdef USE_POLL
		/* wait nand irq, but there is no irq coming */
		/*clear irq */
		writel((readl(LS2H_INT_CLR0_REG) | 0x400), LS2H_INT_CLR0_REG);
		status_time = STATUS_TIME_LOOP_WS;
		while (!(ls2h_nand_status(info))) {
			if (!(status_time--)) {
				/*time out,so clear cmd,fixme */
				write_z_cmd;
				break;
			}
			udelay(60);
		}
		info->state = STATE_READY;
#endif
		local_irq_restore(flags);
	}
}

/**
 *	flags & 0x1	cmd
 *	flags & 0x2	addrl
 *	flags & 0x4	addrh
 *	flags & 0x8	timing
 *	flags & 0x10	idl
 *	flags & 0x20	status_idh
 *	flags & 0x40	param
 *	flags & 0x80	op_num
 *	flags & 0x100	cs_rdy_map
 ****/
static void nand_setup(unsigned int flags, struct ls2h_nand_info *info)
{
	struct ls2h_nand_desc *nand_base =
	    (struct ls2h_nand_desc *)(info->mmio_base);
	nand_base->cmd = 0;
	nand_base->addrl =
	    (flags & NAND_ADDRL) ==
	    NAND_ADDRL ? info->nand_regs.addrl : info->nand_addrl;
	nand_base->addrh =
	    (flags & NAND_ADDRH) ==
	    NAND_ADDRH ? info->nand_regs.addrh : info->nand_addrh;
	nand_base->timing =
	    (flags & NAND_TIMING) ==
	    NAND_TIMING ? info->nand_regs.timing : info->nand_timing;
	nand_base->op_num =
	    (flags & NAND_OP_NUM) ==
	    NAND_OP_NUM ? info->nand_regs.op_num : info->nand_op_num;
	nand_base->cs_rdy_map =
	    (flags & NAND_CS_RDY_MAP) ==
	    NAND_CS_RDY_MAP ? info->nand_regs.cs_rdy_map : info->
	    nand_cs_rdy_map;
	if (flags & NAND_CMD) {
		if (info->nand_regs.cmd & 0x4)
			flagsss = 1;
		else
			flagsss = 0;

		nand_base->cmd = (info->nand_regs.cmd) & (~0xff);
		nand_base->cmd = info->nand_regs.cmd;
	} else
		nand_base->cmd = info->nand_cmd;
}

static void ls2h_nand_cmdfunc(struct mtd_info *mtd, unsigned command,
			      int column, int page_addr)
{
	unsigned cmd_prev;
	int status_time, page_prev;
	struct ls2h_nand_info *info = mtd_to_ls2h_nand(mtd);

	init_completion(&info->cmd_complete);
	cmd_prev = info->cmd;
	page_prev = info->page_addr;

	info->cmd = command;
	info->page_addr = page_addr;
	switch (command) {
	case NAND_CMD_READOOB:
		if (info->state == STATE_BUSY) {
			printk("nandflash chip if busy...\n");
			return;
		}

		info->state = STATE_BUSY;
		info->buf_count = mtd->oobsize;
		info->buf_start = 0;
		info->cac_size = info->buf_count;
		if (info->buf_count <= 0)
			break;
		/*nand regs set */
		info->nand_regs.addrh = page_addr;
		info->nand_regs.addrl = mtd->writesize;
		info->nand_regs.op_num = info->buf_count;
		/*nand cmd set */
		info->nand_regs.cmd = 0;
		((struct ls2h_nand_cmdset *)&(info->nand_regs.cmd))->read = 1;
		((struct ls2h_nand_cmdset *)&(info->nand_regs.cmd))->op_spare =
		    1;
		((struct ls2h_nand_cmdset *)&(info->nand_regs.cmd))->cmd_valid =
		    1;
		/*dma regs config */
		info->dma_regs.length = ALIGN_DMA(info->buf_count);
		((struct ls2h_nand_dma_cmd *)&(info->dma_regs.cmd))->
		    dma_int_mask = 1;
		/*dma GO set */
		nand_setup(NAND_ADDRL | NAND_ADDRH | NAND_OP_NUM | NAND_CMD,
			   info);
		dma_setup(DMA_LENGTH | DMA_CMD, info);
		break;
	case NAND_CMD_READ0:
		if (info->state == STATE_BUSY) {
			printk("nandflash chip if busy...\n");
			return;
		}
		info->state = STATE_BUSY;
		info->buf_count = mtd->oobsize + mtd->writesize;
		info->buf_start = 0;
		info->cac_size = info->buf_count;
		if (info->buf_count <= 0)
			break;
		info->nand_regs.addrh = page_addr;
		info->nand_regs.addrl = 0x0;
		info->nand_regs.op_num = info->buf_count;
		/*nand cmd set */
		info->nand_regs.cmd = 0;
		info->dma_regs.cmd = 0;
		((struct ls2h_nand_cmdset *)&(info->nand_regs.cmd))->read = 1;
		((struct ls2h_nand_cmdset *)&(info->nand_regs.cmd))->op_spare =
		    1;
		((struct ls2h_nand_cmdset *)&(info->nand_regs.cmd))->op_main =
		    1;
		((struct ls2h_nand_cmdset *)&(info->nand_regs.cmd))->cmd_valid =
		    1;
		/*dma regs config */
		info->dma_regs.length = ALIGN_DMA(info->buf_count);
		((struct ls2h_nand_dma_cmd *)&(info->dma_regs.cmd))->
		    dma_int_mask = 1;
		nand_setup(NAND_ADDRL | NAND_ADDRH | NAND_OP_NUM | NAND_CMD,
			   info);
		dma_setup(DMA_LENGTH | DMA_CMD, info);
		break;
	case NAND_CMD_SEQIN:
		if (info->state == STATE_BUSY) {
			printk("nandflash chip if busy...\n");
			return;
		}
		info->state = STATE_BUSY;
		info->buf_count = mtd->oobsize + mtd->writesize - column;
		info->buf_start = 0;
		info->seqin_column = column;
		info->seqin_page_addr = page_addr;
		complete(&info->cmd_complete);
		break;
	case NAND_CMD_PAGEPROG:
		if (info->state == STATE_BUSY) {
			printk("nandflash chip if busy...\n");
			return;
		}
		info->state = STATE_BUSY;
		if (cmd_prev != NAND_CMD_SEQIN) {
			printk("Prev cmd don't complete...\n");
			break;
		}
		if (info->buf_count <= 0)
			break;

		/*nand regs set */
		info->nand_regs.addrh = info->seqin_page_addr;
		info->nand_regs.addrl = info->seqin_column;
		info->nand_regs.op_num = info->buf_start;
		/*nand cmd set */
		info->nand_regs.cmd = 0;
		info->dma_regs.cmd = 0;
		((struct ls2h_nand_cmdset *)&(info->nand_regs.cmd))->write = 1;
		if (info->seqin_column < mtd->writesize)
			((struct ls2h_nand_cmdset *)&(info->nand_regs.cmd))->
			    op_main = 1;
		((struct ls2h_nand_cmdset *)&(info->nand_regs.cmd))->op_spare =
		    1;
		((struct ls2h_nand_cmdset *)&(info->nand_regs.cmd))->cmd_valid =
		    1;
		/*dma regs config */
		info->dma_regs.length = ALIGN_DMA(info->buf_start);
		((struct ls2h_nand_dma_cmd *)&(info->dma_regs.cmd))->
		    dma_int_mask = 1;
		((struct ls2h_nand_dma_cmd *)&(info->dma_regs.cmd))->dma_r_w =
		    1;
		nand_setup(NAND_ADDRL | NAND_ADDRH | NAND_OP_NUM | NAND_CMD,
			   info);
		dma_setup(DMA_LENGTH | DMA_CMD, info);
		break;
	case NAND_CMD_RESET:
		/*nand cmd set */
		info->nand_regs.cmd = 0;
		((struct ls2h_nand_cmdset *)&(info->nand_regs.cmd))->reset = 1;
		((struct ls2h_nand_cmdset *)&(info->nand_regs.cmd))->cmd_valid =
		    1;
		nand_setup(NAND_CMD, info);
		status_time = STATUS_TIME_LOOP_R;
		while (!ls2h_nand_status(info)) {
			if (!(status_time--)) {
				write_z_cmd;
				break;
			}
			udelay(50);
		}
		complete(&info->cmd_complete);
		break;
	case NAND_CMD_ERASE1:
		if (info->state == STATE_BUSY) {
			printk("nandflash chip if busy...\n");
			return;
		}
		info->state = STATE_BUSY;
		/*nand regs set */
		info->nand_regs.addrh = page_addr;
		info->nand_regs.addrl = 0x0;
		/*nand cmd set */
		info->nand_regs.cmd = 0;
		((struct ls2h_nand_cmdset *)&(info->nand_regs.cmd))->erase_one =
		    1;
		((struct ls2h_nand_cmdset *)&(info->nand_regs.cmd))->cmd_valid =
		    1;
		nand_setup(NAND_ADDRL | NAND_ADDRH | NAND_OP_NUM | NAND_CMD,
			   info);
		status_time = STATUS_TIME_LOOP_E;
		while (!ls2h_nand_status(info)) {
			if (!(status_time--)) {
				write_z_cmd;
				break;
			}
			udelay(50);
		}
		info->state = STATE_READY;
		complete(&info->cmd_complete);
		break;
	case NAND_CMD_STATUS:
		if (info->state == STATE_BUSY) {
			printk("nandflash chip if busy...\n");
			return;
		}
		info->state = STATE_BUSY;
		info->buf_count = 0x1;
		info->buf_start = 0x0;
		*(unsigned char *)info->data_buff =
		    ls2h_nand_status(info) | 0x80;
		complete(&info->cmd_complete);
		break;
	case NAND_CMD_READID:
		if (info->state == STATE_BUSY) {
			printk("nandflash chip if busy...\n");
			return;
		}
		info->state = STATE_BUSY;
		info->buf_count = 0x4;
		info->buf_start = 0;

		{
			unsigned int id_val_l = 0, id_val_h = 0;
			unsigned int timing = 0;
			unsigned char *data =
			    (unsigned char *)(info->data_buff);
			_NAND_READ_REG(0xc, timing);
			_NAND_SET_REG(0xc, 0x30f0);
			_NAND_SET_REG(0x0, 0x21);

			while (((id_val_l |= _NAND_IDL) & 0xff) == 0) {
				id_val_h = _NAND_IDH;
			}
			while (((id_val_h = _NAND_IDH) & 0xff) == 0) ;
#ifdef NAND_DEBUG
			printk("id_val_l=0x%08x\nid_val_h=0x%08x\n", id_val_l,
			       id_val_h);
#endif
			_NAND_SET_REG(0xc, timing);
			data[0] = (id_val_h & 0xff);
			data[1] = (id_val_l & 0xff000000) >> 24;
			data[2] = (id_val_l & 0x00ff0000) >> 16;
			data[3] = (id_val_l & 0x0000ff00) >> 8;
#ifdef NAND_DEBUG
			printk(KERN_ERR
			       "IDS=============================0x%x\n",
			       *((int *)(info->data_buff)));
#endif
		}
		complete(&info->cmd_complete);
		break;
	case NAND_CMD_ERASE2:
	case NAND_CMD_READ1:
		complete(&info->cmd_complete);
		break;
	default:
		printk(KERN_ERR "non-supported command.\n");
		complete(&info->cmd_complete);
		break;
	}
	wait_for_completion_timeout(&info->cmd_complete, timeout);
	if (info->cmd == NAND_CMD_READ0 || info->cmd == NAND_CMD_READOOB) {
		dma_cache_inv((unsigned long)(info->data_buff), info->cac_size);
	}
	info->state = STATE_READY;
}

static int ls2h_nand_detect(struct mtd_info *mtd)
{
	return (mtd->erasesize != 1 << 17 || mtd->writesize != 1 << 11
		|| mtd->oobsize != 1 << 6);
}

static void test_handler(struct timer_list *t)
{
	u32 val;

	struct ls2h_nand_info *s = from_timer(s, t, test_timer);;
	mod_timer(&s->test_timer, jiffies + 1);
	val = s->dma_ask_phy | 0x4;
	*(volatile unsigned int *)ORDER_REG_ADDR = val;
	udelay(1000);
}

static void ls2h_nand_init_info(struct ls2h_nand_info *info)
{
	info->timing_flag = 1;	/*0:read; 1:write; */
	info->num = 0;
	info->size = 0;
	info->cac_size = 0;
	info->state = STATE_READY;
	info->cmd = -1;
	info->page_addr = -1;
	info->nand_addrl = 0x0;
	info->nand_addrh = 0x0;
	info->nand_timing = 0x412;	// 0x4<<8 | 0x12;
	info->nand_op_num = 0x0;
	info->nand_cs_rdy_map = 0x00000000;
	info->nand_cmd = 0;

	info->dma_orderad = 0x0;
	info->dma_saddr = info->data_buff_phys;
	info->dma_daddr = DMA_ACCESS_ADDR;
	info->dma_length = 0x0;
	info->dma_step_length = 0x0;
	info->dma_step_times = 0x1;
	info->dma_cmd = 0x0;

	info->test_timer.expires = jiffies + 10;
	timer_setup(&info->test_timer, test_handler, 0);

	info->order_reg_addr = ORDER_REG_ADDR;
}

static int ls2h_nand_probe(struct platform_device *pdev)
{
	int ret = 0, irq;
	struct resource *r;
	struct mtd_info *mtd;
	struct nand_chip *this;
	struct ls2h_nand_info *info;
	struct platform_nand_chip *pdata;

	pdata = dev_get_platdata(&pdev->dev);;

	info = kzalloc(sizeof(struct ls2h_nand_info), GFP_KERNEL);
	if (!info) {
		dev_err(&pdev->dev, "failed to allocate memory\n");
		return -ENOMEM;
	}

	info->pdev = pdev;

	this = &info->nand_chip;
	mtd  = nand_to_mtd(this);
	info->drcmr_dat = (u64) dma_alloc_coherent(&pdev->dev, MAX_BUFF_SIZE,
						   &info->drcmr_dat_phys,
						   GFP_KERNEL);
	info->dma_ask =
	    (u64) dma_alloc_coherent(&pdev->dev, MAX_BUFF_SIZE,
				     &info->dma_ask_phy, GFP_KERNEL);

	if (!info->drcmr_dat) {
		dev_err(&pdev->dev, "fialed to allocate memory\n");
		ret = -ENOMEM;
		goto fail_free_mtd;
	}
	r = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (r == NULL) {
		dev_err(&pdev->dev, "no IO memory resource defined\n");
		ret = -ENODEV;
		goto fail_free_buf;
	}

	r = request_mem_region(r->start, r->end - r->start + 1, pdev->name);
	if (r == NULL) {
		dev_err(&pdev->dev, "failed to request memory resource\n");
		ret = -EBUSY;
		goto fail_free_buf;
	}

	info->mmio_base = ioremap(r->start, r->end - r->start + 1);
	if (info->mmio_base == NULL) {
		dev_err(&pdev->dev, "ioremap() failed\n");
		ret = -ENODEV;
		goto fail_free_res;
	}
	ret = ls2h_nand_init_buff(info);
	if (ret)
		goto fail_free_io;

	irq = platform_get_irq(pdev, 0);
	if (irq < 0) {
		dev_err(&pdev->dev, "no IRQ resource defined\n");
		ret = -ENXIO;
		goto fail_free_io;
	}
	info->irq = irq;

	ls2h_nand_init_mtd(mtd, info);
	ls2h_nand_init_info(info);
	platform_set_drvdata(pdev, mtd);

	if (nand_scan(mtd, 1)) {
		dev_err(&pdev->dev, "failed to scan nand\n");
		ret = -ENXIO;
		goto fail_free_irq;
	}
	if (ls2h_nand_detect(mtd)) {
		dev_err(&pdev->dev, "driver don't support the Flash!\n");
		ret = -ENXIO;
		goto fail_free_irq;
	}
	mtd->name = "mtd0";
	return mtd_device_parse_register(mtd, NULL, NULL,
				pdata ? pdata->partitions : NULL,
				pdata ? pdata->nr_partitions : 0);

fail_free_irq:
	free_irq(irq, info);
fail_free_io:
	iounmap(info->mmio_base);
fail_free_res:
	release_mem_region(r->start, r->end - r->start + 1);
fail_free_buf:
	dma_free_coherent(&pdev->dev, info->data_buff_size,
			  info->data_buff, info->data_buff_phys);
fail_free_mtd:
	kfree(mtd);
	return ret;
}

static int ls2h_nand_remove(struct platform_device *pdev)
{
	struct mtd_info *mtd = platform_get_drvdata(pdev);
	struct ls2h_nand_info *info = mtd->priv;

	platform_set_drvdata(pdev, NULL);

	mtd_device_unregister(mtd);
	free_irq(info->irq, info);
	kfree((void *)info->drcmr_dat);
	kfree(mtd);

	return 0;
}

static int ls2h_nand_suspend(struct platform_device *pdev, pm_message_t state)
{
	struct mtd_info *mtd = (struct mtd_info *)platform_get_drvdata(pdev);
	struct ls2h_nand_info *info = mtd->priv;

	if (info->state != STATE_READY) {
		dev_err(&pdev->dev, "driver busy, state = %d\n", info->state);
		return -EAGAIN;
	}

	return 0;
}

static int ls2h_nand_resume(struct platform_device *pdev)
{
	return 0;
}

#ifdef CONFIG_OF
static const struct of_device_id ls2h_nand_dt_match[] = {
    { .compatible = "loongson,ls2h-nand", },
    {},
};
MODULE_DEVICE_TABLE(of, ls2h_nand_dt_match);
#endif

static struct platform_driver ls2h_nand_driver = {
	.probe = ls2h_nand_probe,
	.remove = ls2h_nand_remove,
	.suspend = ls2h_nand_suspend,
	.resume = ls2h_nand_resume,
	.driver = {
		.name = "ls2h-nand",
		.owner = THIS_MODULE,
#ifdef CONFIG_OF
		.of_match_table = of_match_ptr(ls2h_nand_dt_match),
#endif
	},
};

static int __init ls2h_nand_init(void)
{
	int ret = 0;

	ret = platform_driver_register(&ls2h_nand_driver);
	if (ret) {
		printk(KERN_ERR "failed to register loongson_1g_nand_driver\n");
	}
	return ret;
}

static void __exit ls2h_nand_exit(void)
{
	platform_driver_unregister(&ls2h_nand_driver);
}

module_init(ls2h_nand_init);
module_exit(ls2h_nand_exit);
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Loongson-2H NAND controller driver");
