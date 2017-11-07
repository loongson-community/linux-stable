/*
 * Copyright (C) 2016 SiliconMotion Inc.
 *
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License v2. See the file COPYING in the main directory of this archive for
 * more details.
 */



#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/pci.h>
#include <linux/slab.h>

#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/jiffies.h>
#include <linux/timer.h>


#include "smi_snd.h"
#include "ddk768/uda1345.h"
#include "ddk768/ddk768_reg.h"
#include "ddk768/ddk768_iis.h"
#include "ddk768/ddk768_intr.h"
#include "ddk768/ddk768_power.h"

#include <drm/drmP.h>
#include <drm/drm_crtc_helper.h>

#include "smi_drv.h"
#include "hw768.h"

char sramTxSection = 0; /* updated section by firmware, Start with section 0 of SRAM */
struct sm768chip *chip_irq_id=NULL;/*chip_irq_id is use for request and free irq*/

static int SM768_AudioInit(unsigned long wordLength, unsigned long sampleRate)
{

	// Set up I2S and GPIO registers to transmit/receive data.
    iisOpen(wordLength, sampleRate);
    //Set I2S to DMA 256 DWord from SRAM starting at location 0 of SRAM
    iisTxDmaSetup(0,SRAM_SECTION_SIZE);

	// Init audio codec
    if(uda1345_init())
    {
        uda1345_deinit();
        return -1;
    }

    return 0;
}

/*
 * This function call iis driver interface iisStart() to start play audio. 
 */
static int SM768_AudioStart(void)
{
    
    iisStart();
    uda1345_setpower(ADCOFF_DACON);
	uda1345_setmute(NO_MUTE);
	HDMI_Audio_Unmute();
    return 0;
}

/*
 * Stop audio. 
 */
static int SM768_AudioStop(void)
{
	uda1345_setmute(MUTE);
    uda1345_setpower(ADCOFF_DACOFF);
	HDMI_Audio_Mute();
    iisStop();
	
    return 0;
}



static int SM768_AudioDeinit(void)
{
	sb_IRQMask(SB_IRQ_VAL_I2S);
	uda1345_deinit();
	iisClose();

    return 0;
}





static u8  VolAuDrvToCodec(u16 audrv)
{
	u8 codecdb,map;
    if(audrv == 0x8000)
        return 0x3b;

    codecdb = (audrv >> 8);

    if(codecdb < 0x80)
        map = (0x1f - (codecdb >> 2));
    else
        map = (0x5f - (codecdb >> 2));
    return map;
}


static int snd_falconi2s_info_hw_volume(struct snd_kcontrol *kcontrol,
				     struct snd_ctl_elem_info *uinfo)
{
	dbg_msg("snd_falconi2s_info_hw_volume\n");
	uinfo->type = SNDRV_CTL_ELEM_TYPE_INTEGER;
	uinfo->count = 1;
	uinfo->value.integer.min = 0;
	uinfo->value.integer.max = 0x1f;
	return 0;
}

static int snd_falconi2s_get_hw_play_volume(struct snd_kcontrol *kcontrol,
				    struct snd_ctl_elem_value *ucontrol)
{
	dbg_msg("snd_falconi2s_get_hw_volume\n");

	struct sm768chip *chip = kcontrol->private_data;
	
	ucontrol->value.integer.value[0] = chip->playback_vol;
	
	return 0;
}
static int snd_falconi2s_put_hw_play_volume(struct snd_kcontrol *kcontrol,
				    struct snd_ctl_elem_value *ucontrol)
{
	dbg_msg("snd_falconi2s_put_hw_volume:%d\n",ucontrol->value.integer.value[0]);

	struct sm768chip *chip = kcontrol->private_data;
	int changed = 0;
	unsigned short Reg_Vol;
	u8 vol;


	if (chip->playback_vol!= ucontrol->value.integer.value[0]) {
		vol = chip->playback_vol = ucontrol->value.integer.value[0];
		uda1345_setvolume(VolAuDrvToCodec(vol));
		changed = 1;
	}

	return changed;

}


static int snd_falconi2s_get_hw_capture_volume(struct snd_kcontrol *kcontrol,
				    struct snd_ctl_elem_value *ucontrol)
{
	dbg_msg("snd_falconi2s_get_hw_volume\n");

	struct sm768chip *chip = kcontrol->private_data;
	
	ucontrol->value.integer.value[0] = chip->capture_vol;
	
	return 0;
}
static int snd_falconi2s_put_hw_capture_volume(struct snd_kcontrol *kcontrol,
				    struct snd_ctl_elem_value *ucontrol)
{
	dbg_msg("snd_falconi2s_put_hw_volume:%d\n",ucontrol->value.integer.value[0]);


	struct sm768chip *chip = kcontrol->private_data;
	int changed = 0;
	unsigned short Reg_Vol;
	u8 vol;


	if (chip->capture_vol!= ucontrol->value.integer.value[0]) {
		vol = chip->capture_vol = ucontrol->value.integer.value[0];
		uda1345_setvolume(VolAuDrvToCodec(vol));
		changed = 1;
		}

	return changed;
	
}


static struct snd_kcontrol_new falconi2s_vol[] = {
 {
		.iface = SNDRV_CTL_ELEM_IFACE_MIXER,
		.name = "Playback Volume",
		.info = snd_falconi2s_info_hw_volume,
		.get = snd_falconi2s_get_hw_play_volume,
		.put = snd_falconi2s_put_hw_play_volume,
		.private_value = 0,

},
{
		.iface = SNDRV_CTL_ELEM_IFACE_MIXER,
		.name = "Capture Volume",
		.info = snd_falconi2s_info_hw_volume,
		.get = snd_falconi2s_get_hw_capture_volume,
		.put = snd_falconi2s_put_hw_capture_volume,
		.private_value = 0,
},
};
  
/* hardware definition */
static struct snd_pcm_hardware snd_falconi2s_playback_hw = {
	.info = (SNDRV_PCM_INFO_MMAP |
                   SNDRV_PCM_INFO_INTERLEAVED |
                   SNDRV_PCM_INFO_BLOCK_TRANSFER |
                   SNDRV_PCM_INFO_MMAP_VALID),
	.formats =		  SNDRV_PCM_FMTBIT_S16_LE,
	.rates =			  SNDRV_PCM_RATE_8000_48000,//this value means both of 44100 and 48000 can work 
	.rate_min =		  48000,
	.rate_max =		  48000,
	.channels_min =	  2,
	.channels_max =	  2,
	.buffer_bytes_max = P_PERIOD_BYTE*P_PERIOD_MAX,//actually total length should less than 4096*1024.
	.period_bytes_min = P_PERIOD_BYTE ,
	.period_bytes_max = P_PERIOD_BYTE,
	.periods_min =	  P_PERIOD_MIN,
	.periods_max =	  P_PERIOD_MAX,
};

  /* hardware definition */
static struct snd_pcm_hardware snd_falconi2s_capture_hw = {
	.info = (SNDRV_PCM_INFO_MMAP |
                   SNDRV_PCM_INFO_INTERLEAVED |
                   SNDRV_PCM_INFO_BLOCK_TRANSFER |
                   SNDRV_PCM_INFO_MMAP_VALID),
	.formats =          SNDRV_PCM_FMTBIT_S16_LE,
	.rates =            SNDRV_PCM_RATE_8000_48000,
	.rate_min =         8000,
	.rate_max =         48000,
	.channels_min =     2,
	.channels_max =     2,
	.buffer_bytes_max = 32768,
	.period_bytes_min = 4096,
	.period_bytes_max = 32768,
	.periods_min =      1,
	.periods_max =      1024,
};

  /* open callback */
static int snd_falconi2s_playback_open(struct snd_pcm_substream *substream)
{
	dbg_msg("snd_falconi2s_playback_open\n");
		
	struct sm768chip *chip = snd_pcm_substream_chip(substream);
	struct snd_pcm_runtime *runtime = substream->runtime;

	spin_lock(&chip->lock);
	runtime->hw = snd_falconi2s_playback_hw;
	/* set the pointer value of substream field in the chip record at open callback to hold the current running substream pointer */
	chip->substream = substream;
	spin_unlock(&chip->lock);
	
	return 0;
}

  /* close callback */
static int snd_falconi2s_playback_close(struct snd_pcm_substream *substream)
{
	struct sm768chip *chip = snd_pcm_substream_chip(substream);
	/* the hardware-specific codes will be here */
	dbg_msg("snd_falconi2s_playback_close\n");
	/* reset the pointer value of substream field in the chip record at close callback */
	chip->substream = NULL;			 
	return 0;

}

  /* open callback */
static int snd_falconi2s_capture_open(struct snd_pcm_substream *substream)
{
	struct sm768chip *chip = snd_pcm_substream_chip(substream);
	struct snd_pcm_runtime *runtime = substream->runtime;

	runtime->hw = snd_falconi2s_capture_hw;
	spin_lock(&chip->lock);
	/* set the pointer value of substream field in the chip record at open callback to hold the current running substream pointer */
	chip->substream = substream;
	spin_unlock(&chip->lock);
	
	return 0;
}

  /* close callback */
static int snd_falconi2s_capture_close(struct snd_pcm_substream *substream)
{
	struct sm768chip *chip = snd_pcm_substream_chip(substream);
	/* the hardware-specific codes will be here */
	/* reset the pointer value of substream field in the chip record at close callback */
	chip->substream = NULL;
	return 0;

}

  /* hw_params callback */
static int snd_falconi2s_pcm_hw_params(struct snd_pcm_substream *substream,
                               struct snd_pcm_hw_params *hw_params)
{
	dbg_msg("snd_falconi2s_pcm_hw_params,malloc:%d\n",params_buffer_bytes(hw_params));
	   
	return snd_pcm_lib_malloc_pages(substream,
                                     params_buffer_bytes(hw_params));
}

  /* hw_free callback */
static int snd_falconi2s_pcm_hw_free(struct snd_pcm_substream *substream)
{
	dbg_msg("snd_falconi2s_pcm_hw_free\n");
	return snd_pcm_lib_free_pages(substream);
}

  /* prepare callback */
static int snd_falconi2s_pcm_prepare(struct snd_pcm_substream *substream)
{
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct sm768chip *chip = snd_pcm_substream_chip(substream);
	int i=0;
	dbg_msg("snd_falconi2s_pcm_prepare\n");

	chip->pos = 0;

	dbg_msg("runtime->rate:%d\n",runtime->rate);
	dbg_msg("runtime->buffer_size:%d\n",runtime->buffer_size);
	dbg_msg("runtime->periods:%d\n",runtime->periods);
	dbg_msg("runtime->period_size:%d\n",runtime->period_size);
	dbg_msg("runtime->frame_bits:%d\n",runtime->frame_bits);
	dbg_msg("runtime->dma_bytes:%d\n",runtime->dma_bytes);

	return 0;
}

/* trigger callback */
static int snd_falconi2s_pcm_playback_trigger(struct snd_pcm_substream *substream,
                                    int cmd)
{
	struct sm768chip *chip = snd_pcm_substream_chip(substream);
	dbg_msg("snd_falconi2s_pcm_trigger\n");
	dbg_msg("substream:0x%x\n",substream);

	spin_lock(&chip->lock);
	
	switch (cmd) {
	case SNDRV_PCM_TRIGGER_START:		
		dbg_msg("SNDRV_PCM_TRIGGER_START\n");
	
		chip->pos = 0;	
		memset_io(chip->pvReg + SRAM_OUTPUT_BASE, 0, SRAM_OUTPUT_SIZE);
		
		/*enable IIS*/    
		sramTxSection = 0;
		SM768_AudioStart();
		
		sramTxSection = 1;

		break;
	case SNDRV_PCM_TRIGGER_STOP:
		dbg_msg("SNDRV_PCM_TRIGGER_STOP\n");
		
		/*disable IIS*/    
		SM768_AudioStop();

		break;
	default:
		return -EINVAL;
	}

	spin_unlock(&chip->lock);
	
	return 0;
}


 static int snd_falconi2s_pcm_capture_trigger(struct snd_pcm_substream *substream,
									  int cmd)
 {
  	return 0;
 }

 static int snd_falconi2s_pcm_trigger(struct snd_pcm_substream *substream, int cmd)
 {
	 
	  if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK)
		  return snd_falconi2s_pcm_playback_trigger(substream, cmd);
	  else
		  return snd_falconi2s_pcm_capture_trigger(substream, cmd);
 }



  /* pointer callback */
  static snd_pcm_uframes_t
snd_falconi2s_pcm_pointer(struct snd_pcm_substream *substream)
{
	struct sm768chip *chip = snd_pcm_substream_chip(substream);
	snd_pcm_uframes_t value = 0;

	if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK)
		value = bytes_to_frames(substream->runtime, chip->pos);
		
	return value;
}

  /* operators */
static struct snd_pcm_ops snd_falconi2s_playback_ops = {
          .open =        snd_falconi2s_playback_open,
          .close =       snd_falconi2s_playback_close,
          .ioctl =       snd_pcm_lib_ioctl,
          .hw_params =   snd_falconi2s_pcm_hw_params,
          .hw_free =     snd_falconi2s_pcm_hw_free,
          .prepare =     snd_falconi2s_pcm_prepare,
          .trigger =     snd_falconi2s_pcm_trigger,
          .pointer =     snd_falconi2s_pcm_pointer,
  };

  /* operators */
static struct snd_pcm_ops snd_falconi2s_capture_ops = {
          .open =        snd_falconi2s_capture_open,
          .close =       snd_falconi2s_capture_close,
          .ioctl =       snd_pcm_lib_ioctl,
          .hw_params =   snd_falconi2s_pcm_hw_params,
          .hw_free =     snd_falconi2s_pcm_hw_free,
          .prepare =     snd_falconi2s_pcm_prepare,
          .trigger =     snd_falconi2s_pcm_trigger,
          .pointer =     snd_falconi2s_pcm_pointer,
  };




static int snd_falconi2s_free(struct sm768chip *chip)
{
	/* will be implemented later... */
	dbg_msg("snd_falconi2s_free!\n");

	return 0;
}

/* component-destructor
* (see "Management of Cards and Components")
*/
static int snd_falconi2s_dev_free(struct snd_device *device)
{
	return snd_falconi2s_free(device->device_data);
}

/*
 * interrupt handler
 */
static irqreturn_t snd_smi_interrupt(int irq, void *dev_id)
{
	
	struct sm768chip *chip = dev_id;
	struct snd_pcm_runtime *runtime;
	struct snd_pcm_substream *substream;

	if(hw768_check_iis_interrupt())
	{
		iisClearRawInt();//clear int

		substream = chip->substream;
		if(substream == NULL){
			printk("substream == NULL\n");
			return IRQ_NONE;
		}
		runtime = substream->runtime;
		if (runtime->dma_area == NULL) {
			printk("runtime->dma_area == NULL\n");
			return IRQ_NONE;
		}
		
		/*Now the hardware will automatically swtich to next section when it finished
		So directly increase the sramTxSection in ISR to point to the next next section and start to prepare address and data */
		sramTxSection++;
		sramTxSection %= OUTPUT_SRAM_SECTIONS_NUM;
		
		/* put data of next section */  
		/* only support SRAM_SECTION_NUM = 2 */
		memcpy_fromio(chip->pvReg + SRAM_OUTPUT_BASE + SRAM_SECTION_SIZE * sramTxSection, runtime->dma_area + chip->pos, P_PERIOD_BYTE);
		
		chip->pos += P_PERIOD_BYTE;
		chip->pos %= ((runtime->periods) * (P_PERIOD_BYTE));
		
		snd_pcm_period_elapsed(substream);
		
		return IRQ_HANDLED;
	}
	else
		return IRQ_NONE;
}

  /* chip-specific constructor
   * (see "Management of Cards and Components")
   */
static int snd_falconi2s_create(struct snd_card *card,
                                         struct drm_device *dev,
                                         struct sm768chip **rchip)
{
	int err;
	struct pci_dev *pci = dev->pdev;
	struct smi_device *smi_device = dev->dev_private;
	struct sm768chip *chip;
	static struct snd_device_ops ops = {
		.dev_free = snd_falconi2s_dev_free,
	};

	*rchip = NULL;

	/* allocate a chip-specific data with zero filled */
	chip = kzalloc(sizeof(*chip), GFP_KERNEL);
	if (chip == NULL)
		return -ENOMEM;

	chip->card = card;

	err = snd_device_new(card, SNDRV_DEV_LOWLEVEL, chip, &ops);
	if (err < 0) {
		snd_falconi2s_free(chip);
		return err;
	}
	
	//map register
	chip->vidreg_start = smi_device->rmmio_base;
	chip->vidreg_size = smi_device->rmmio_size;
	dbg_msg("Audio MMIO phyAddr = 0x%x\n",chip->vidreg_start);

	chip->pvReg = smi_device->rmmio;
	dbg_msg("Audio MMIO virtual addr = %p\n",chip->pvReg);

	//map video memory.
	chip->vidmem_start = smi_device->mc.vram_base;
	chip->vidmem_size = 0x200000;   // change the video memory temperarily
	dbg_msg("video memory phyAddr = 0x%x, size = (Dec)%d bytes\n",
	chip->vidmem_start,chip->vidmem_size);
	chip->pvMem = ioremap_wc(chip->vidmem_start,chip->vidmem_size);

	if(!chip->pvMem){
		err_msg("Map video memory failed\n");
		snd_falconi2s_free(chip);
		err = -EFAULT;
		return err;
	}else{
		dbg_msg("Audio video memory virtual addr = %p\n",chip->pvMem);
	}
	//above 

	chip->irq = pci->irq;

	dbg_msg("Audio pci irq :%d\n",chip->irq);


	if(SM768_AudioInit(SAMPLE_BITS, SAMPLE_RATE)) {
		err_msg("Audio init failed!\n");	
		snd_falconi2s_free(chip);
		return -1;
	}

	chip_irq_id=chip;/*Record chip_irq_id which will use in free_irq*/
	dbg_msg("chip_irq_id=%d\n", chip_irq_id);

	iisClearRawInt();//clear int

	//Setup ISR. The ISR will move more data from DDR to SRAM.
	
	if (request_irq(pci->irq, snd_smi_interrupt, IRQF_SHARED,
		KBUILD_MODNAME, chip_irq_id)) {
		err_msg("unable to grab IRQ %d\n", pci->irq);
		snd_falconi2s_free(chip);
		return -EBUSY;
	}
	sb_IRQUnmask(SB_IRQ_VAL_I2S); 

#if LINUX_VERSION_CODE <= KERNEL_VERSION(3,18,0)
	snd_card_set_dev(card, &pci->dev);
#endif

	*rchip = chip;
	return 0;
}


int smi_audio_init(struct drm_device *dev)
{

	int idx, ret, err;
	struct pci_dev *pci = dev->pdev;
	struct snd_pcm *pcm;
	struct snd_card *card;
	struct sm768chip *chip;
	
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,16,0)
	err = snd_card_new(&pci->dev, SNDRV_DEFAULT_IDX1, SNDRV_DEFAULT_STR1, THIS_MODULE, 0, &card);
#else
	err = snd_card_create(-1, 0, THIS_MODULE, 0, &card);
#endif
	
	if (err < 0)
		return err;

	err = snd_falconi2s_create(card, dev, &chip);
	if (err < 0) {
		snd_card_free(card);
      		return err;
	}

	strcpy(card->driver, "SiliconMotion Audio");
	strcpy(card->shortname, "SMI Audio");
	strcpy(card->longname, "SiliconMotion Audio");

	snd_pcm_new(card,"smiaudio_pcm",0,1,0,&pcm);
	pcm->private_data = chip;

      
	/* set operators */
	snd_pcm_set_ops(pcm, SNDRV_PCM_STREAM_PLAYBACK,
                          &snd_falconi2s_playback_ops);
	  
	snd_pcm_lib_preallocate_pages_for_all(pcm, SNDRV_DMA_TYPE_DEV,
                                                snd_dma_pci_data(pci),
						P_PERIOD_BYTE*P_PERIOD_MIN, P_PERIOD_BYTE*P_PERIOD_MAX);

	strcpy(card->mixername, "SiliconMotion Audio Mixer Control");
	
	for (idx = 0; idx < ARRAY_SIZE(falconi2s_vol); idx++) {
		if ((err = snd_ctl_add(card,snd_ctl_new1(&falconi2s_vol[idx], chip))) < 0)
		{
			return err;
		}
	}

	
	err = snd_card_register(card);
	if (err < 0) {
		snd_card_free(card);
		return err;
	}

	pci_set_drvdata(pci, card);

	return 0;

}

void smi_audio_remove(struct drm_device *dev)
{
	struct pci_dev *pci = dev->pdev;
	struct snd_card *card;

	inf_msg("smi_pci_remove\n");
	card = pci_get_drvdata(pci);
	
	dbg_msg("perpare to free irq, pci irq :%d, chip_irq_id=%d\n", pci->irq, chip_irq_id);
	if(pci->irq){				
		free_irq(pci->irq, chip_irq_id);
		dbg_msg("free irq\n");
	}

	SM768_AudioDeinit();
	
	snd_card_free(card);
	pci_set_drvdata(pci, NULL);
	iounmap(chip_irq_id->pvMem);
}

