#ifndef __WO_CFG_H__
#define __WO_CFG_H__

#include <stdbool.h>

typedef union cust_value_t
{
	int  integer;
	char *str;
	bool boolean;
} cust_value_t;


typedef struct cust_setting_t
{
	char* system_property_key;
	cust_value_t default_value;
} cust_setting_t;

typedef enum cust_setting_type_t
{
	SETTING_START,
	CUST_PCSCF_IP4_VALUE,
	CUST_PCSCF_IP6_VALUE,
	FORCE_TSI_64,
	FORCE_TSI_FULL,
	USE_CFG_VIP,
	CUST_IMEI_CP,
	RETRAN_TO,
	RETRAN_TRIES,
	RETRAN_BASE,
	IS_CERTIFICATE_USED,
	IKE_IDI,
	IKE_HASHANDURL,
	IKE_KEEP_ALIVE_TIMER,
	WIFI_DRIVER_KEEP_ALIVE,
	WIFI_DRIVER_KEEP_ALIVE_TIMER,
	ADDR_CHANGE_N_REAUTH,
	STATUS_CODE,
	SETTING_END
} cust_setting_type_t;

static cust_setting_t cust_settings[SETTING_END] =
{
	[CUST_PCSCF_IP4_VALUE]    = {"net.wo.cust_pcscf_4",  {.integer = 20}},
	[CUST_PCSCF_IP6_VALUE]    = {"net.wo.cust_pcscf_6",  {.integer = 21}},
	[FORCE_TSI_64]            = {"net.wo.force_tsi_64",  {.boolean = true}},
	[FORCE_TSI_FULL]          = {"net.wo.force_tsi_full",  {.boolean = true}},
	[USE_CFG_VIP]             = {"net.wo.use_cfg_vip",   {.boolean = false}},
	[CUST_IMEI_CP]            = {"net.wo.cust_imei_cp",  {.integer = 16397}},
	[RETRAN_TO]               = {"net.wo.retrans_to",    {.str = NULL}},
	[RETRAN_TRIES]            = {"net.wo.retrans_tries", {.str = NULL}},
	[RETRAN_BASE]             = {"net.wo.retrans_base",  {.str = NULL}},
	[IS_CERTIFICATE_USED]     = {"net.wo.cert_used",  {.boolean = true}},
	[IKE_IDI]                 = {"net.wo.IDi",           {.integer = 0}},
	[IKE_HASHANDURL]          = {"net.wo.urlcert",       {.boolean = false}},
	[IKE_KEEP_ALIVE_TIMER]    = {"net.wo.keep_timer",	  {.integer = -1}},
	[WIFI_DRIVER_KEEP_ALIVE]  = {"net.wo.wdrv_keep_alive", 	  {.boolean = false}},
	[WIFI_DRIVER_KEEP_ALIVE_TIMER] = {"net.wo.wdrv_keep_timer",	  {.integer = -1}},
	[ADDR_CHANGE_N_REAUTH]    = {"net.wo.reauth_addr",   {.boolean = false}},
	[STATUS_CODE]             = {"net.wo.statuscode",	 {.integer = 0}},
};

int get_cust_setting(cust_setting_type_t type, char *value);
bool get_cust_setting_bool(cust_setting_type_t type);
int get_cust_setting_int(cust_setting_type_t type);
int set_cust_setting(cust_setting_type_t type, char *value);
int set_cust_setting_bool(cust_setting_type_t type, bool value);
int set_cust_setting_int(cust_setting_type_t type, int value);

#endif
