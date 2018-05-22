/**
 * struct loongson_vbios - loongson vbios structure
 *
 * @driver_priv: Pointer to driver-private information.
 */
#define MAX_RESOLUTIONS 10
#define MAX_REG_TABLE   256

struct loongson_vbios {
	char title[16];
	uint32_t version_major;
	uint32_t version_minor;
	char information[20];
	uint32_t crtc_num;
	uint32_t crtc_offset;
	uint32_t connector_num;
	uint32_t connector_offset;
	uint32_t encoder_num;
	uint32_t encoder_offset;
} __packed;

enum loongson_crtc_version {
	default_version = 0,
};

struct loongson_crtc_modeparameter {
	/* horizontal timing. */
	uint32_t horizontaltotal;
	uint32_t horizontaldisplay;
	uint32_t horizontalsyncstart;
	uint32_t horizontalsyncwidth;
	uint32_t horizontalsyncpolarity;

	/* vertical timing. */
	uint32_t verticaltotal;
	uint32_t verticaldisplay;
	uint32_t verticalsyncstart;
	uint32_t verticalsyncheight;
	uint32_t verticalsyncpolarity;

	/* refresh timing. */
	int32_t pixelclock;
	uint32_t horizontalfrequency;
	uint32_t verticalfrequency;

	/* clock phase. this clock phase only applies to panel. */
	uint32_t clockphasepolarity;
};

struct loongson_encoder_conf_reg {
	unsigned char dev_addr;
	unsigned char reg;
	unsigned char value;
} __packed;

struct loongson_encoder_resolution_config {
	unsigned char reg_num;
	struct loongson_encoder_conf_reg config_regs[MAX_REG_TABLE];
} __packed;

struct loongson_resolution_param {
	bool used;
	uint32_t hdisplay;
	uint32_t vdisplay;
};

struct loongson_crtc_config_param {
	struct loongson_resolution_param resolution;
	struct loongson_crtc_modeparameter crtc_resol_param;
};

struct loongson_encoder_config_param {
	struct loongson_resolution_param resolution;
	struct loongson_encoder_resolution_config encoder_resol_param;
};

struct loongson_vbios_crtc {
	uint32_t next_crtc_offset;
	uint32_t crtc_id;
	enum loongson_crtc_version crtc_version;
	uint32_t crtc_max_freq;
	uint32_t crtc_max_width;
	uint32_t crtc_max_height;
	uint32_t connector_id;
	uint32_t phy_num;
	uint32_t encoder_id;
	uint32_t reserve;
	bool use_local_param;
	struct loongson_crtc_config_param mode_config_tables[MAX_RESOLUTIONS];
} __packed;

enum loongson_edid_method {
	edid_method_null = 0,
	edid_method_i2c = 1,
	edid_method_vbios = 2,
	edid_method_encoder = 3,
	edid_method_max = 0xffffffff,
};

enum loongson_vbios_i2c_type {
	i2c_type_null = 0,
	i2c_type_gpio = 1,
	i2c_type_cpu  = 2,
	i2c_type_encoder  = 3,
	i2c_type_max = 0xffffffff,
};

enum hot_swap_method {
	hot_swap_disable = 0,
	hot_swap_polling = 1,
	hot_swap_irq = 2,
	hot_swap_max = 0xffffffff,
};

enum loongson_encoder_config {
	encoder_transparent = 0,
	encoder_os_config,
	encoder_bios_config,
	encoder_type_max = 0xffffffff,
};

enum encoder_type {
	encoder_none,
	encoder_dac,
	encoder_tmds,
	encoder_lvds,
	encoder_tvdac,
	encoder_virtual,
	encoder_dsi,
	encoder_dpmst,
	encoder_dpi
};

enum connector_type {
	connector_unknown,
	connector_vga,
	connector_dvii,
	connector_dvid,
	connector_dvia,
	connector_composite,
	connector_svideo,
	connector_lvds,
	connector_component,
	connector_9pindin,
	connector_displayport,
	connector_hdmia,
	connector_hdmib,
	connector_tv,
	connector_edp,
	connector_virtual,
	connector_dsi,
	connector_dpi
};

struct loongson_backlight_pwm {
    uint8_t pwm_id, polarity;
    uint32_t period_ns;
};

struct loongson_vbios_encoder {
	uint32_t next_encoder_offset;
	uint32_t crtc_id;
	uint32_t connector_id;
	uint32_t reserve;
	enum loongson_encoder_config config_type;
	enum loongson_vbios_i2c_type i2c_type;
	uint32_t i2c_id;
	enum encoder_type type;
	struct loongson_encoder_config_param mode_config_tables[MAX_RESOLUTIONS];
} __packed;

struct loongson_vbios_connector {
	uint32_t next_connector_offset;
	uint32_t crtc_id;
	enum loongson_edid_method edid_method;
	enum loongson_vbios_i2c_type i2c_type;
	uint32_t i2c_id;
	uint32_t encoder_id;
	enum connector_type type;
	enum hot_swap_method hot_swap_method;
	uint32_t hot_swap_irq;
	uint32_t edid_version;
	uint8_t internal_edid[256];
	struct loongson_backlight_pwm bl_pwm;
} __packed;
