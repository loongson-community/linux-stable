
#include "l3.h"

/*
 * Send one byte of data to the chip.  Data is latched into the chip on
 * the rising edge of the clock.
 */
static void sendbyte(struct l3_pins *adap, unsigned int byte)
{
	int i;

	for (i = 0; i < 8; i++) {
		adap->setclk(0);
		sb_OS_WAIT_USEC_POLL(adap->data_hold);
		adap->setdat(byte & 1);
		sb_OS_WAIT_USEC_POLL(adap->data_setup);
		adap->setclk(1);
		sb_OS_WAIT_USEC_POLL(adap->clock_high);
		byte >>= 1;
	}
}

/*
 * Send a set of bytes to the chip.  We need to pulse the MODE line
 * between each byte, but never at the start nor at the end of the
 * transfer.
 */
static void sendbytes(struct l3_pins *adap, const u8 *buf,
		      int len)
{
	int i;

	for (i = 0; i < len; i++) {
		if (i) {
			sb_OS_WAIT_USEC_POLL(adap->mode_hold);
			adap->setmode(0);
			sb_OS_WAIT_USEC_POLL(adap->mode);
		}
		adap->setmode(1);
		sb_OS_WAIT_USEC_POLL(adap->mode_setup);
		sendbyte(adap, buf[i]);
	}
}

int l3_write(struct l3_pins *adap, u8 addr, u8 *data, int len)
{
	adap->setclk(1);
	adap->setdat(1);
	adap->setmode(1);
	sb_OS_WAIT_USEC_POLL(adap->mode);

	adap->setmode(0);
	sb_OS_WAIT_USEC_POLL(adap->mode_setup);
	sendbyte(adap, addr);
	sb_OS_WAIT_USEC_POLL(adap->mode_hold);

	sendbytes(adap, data, len);

	adap->setclk(1);
	adap->setdat(1);
	adap->setmode(0);

	return len;
}

