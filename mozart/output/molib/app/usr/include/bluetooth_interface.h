/**
 * @file bluetooth_interface.h
 * @brief For the operation of the Bluetooth API
 * @author <zhe.wu@ingenic.com>
 * @version 1.0.0
 * @date 2015-06-15
 *
 * Copyright (C) 2014 Ingenic Semiconductor Co., Ltd.
 *
 * The program is not free, Ingenic without permission,
 * no one shall be arbitrarily (including but not limited
 * to: copy, to the illegal way of communication, display,
 * mirror, upload, download) use, or by unconventional
 * methods (such as: malicious intervention Ingenic data)
 * Ingenic's normal service, no one shall be arbitrarily by
 * software the program automatically get Ingenic data
 * Otherwise, Ingenic will be investigated for legal responsibility
 * according to law.
 */

#ifndef __BLUETOOTH_INTERFACE_H_
#define __BLUETOOTH_INTERFACE_H_

#undef TRUE
#define BSA_DISC_VID_PID_MAX		1
#define HCI_EXT_INQ_RESPONSE_LEN        240
#define BD_ADDR_LEN			6
#define BD_NAME_LEN			248
#define DEV_CLASS_LEN			3
#define BSA_EIR_DATA_LENGTH		HCI_EXT_INQ_RESPONSE_LEN
#define BSA_AVK_ORI_SAMPLE		0
#define BSA_HS_ORI_SAMPLE		0
#define BLE_INCLUDED			TRUE

typedef unsigned char   UINT8;
typedef unsigned short  UINT16;
typedef unsigned int    UINT32;
typedef unsigned char   BOOLEAN;
typedef UINT8		tBTA_HH_PROTO_MODE;

typedef struct {
	UINT8 data_length;
	UINT16 company_id;
	UINT8 *p_manu;
} bsa_manu_data;

#define LINK_KEY_LEN    16
typedef UINT8 LINK_KEY[LINK_KEY_LEN];       /* Link Key */

#define BT_OCTET8_LEN    8
typedef UINT8 BT_OCTET8[BT_OCTET8_LEN];   /* octet array: size 16 */

/*****************************************************************************
 **  BLE Constants and Type Definitions
 *****************************************************************************/
#ifndef BSA_BLE_DEBUG
#define BSA_BLE_DEBUG FALSE
#endif

/* ADV data flag bit definition used for BTM_BLE_AD_TYPE_FLAG */
#define BTM_BLE_LIMIT_DISC_FLAG         (0x01 << 0)             /* bit 0 */
#define BTM_BLE_GEN_DISC_FLAG           (0x01 << 1)             /* bit 1 */
#define BTM_BLE_BREDR_NOT_SPT           (0x01 << 2)             /* bit 2*/
/* 4.1 spec adv flag for simultaneous BR/EDR+LE connection support */
#define BTM_BLE_DMT_CONTROLLER_SPT      (0x01 << 3)             /* bit 3 */
#define BTM_BLE_DMT_HOST_SPT            (0x01 << 4)             /* bit 4 */
#define BTM_BLE_NON_LIMIT_DISC_FLAG     (0x00 )         /* lowest bit unset */
#define BTM_BLE_ADV_FLAG_MASK           (BTM_BLE_LIMIT_DISC_FLAG | BTM_BLE_BREDR_NOT_SPT | BTM_BLE_GEN_DISC_FLAG)
#define BTM_BLE_LIMIT_DISC_MASK         (BTM_BLE_LIMIT_DISC_FLAG )

#define BTM_BLE_AD_BIT_DEV_NAME        (0x00000001 << 0)
#define BTM_BLE_AD_BIT_FLAGS           (0x00000001 << 1)
#define BTM_BLE_AD_BIT_MANU            (0x00000001 << 2)
#define BTM_BLE_AD_BIT_TX_PWR          (0x00000001 << 3)
#define BTM_BLE_AD_BIT_INT_RANGE       (0x00000001 << 5)
#define BTM_BLE_AD_BIT_SERVICE         (0x00000001 << 6)
#define BTM_BLE_AD_BIT_SERVICE_SOL     (0x00000001 << 7)
#define BTM_BLE_AD_BIT_SERVICE_DATA    (0x00000001 << 8)
#define BTM_BLE_AD_BIT_SIGN_DATA       (0x00000001 << 9)
#define BTM_BLE_AD_BIT_SERVICE_128SOL  (0x00000001 << 10)
#define BTM_BLE_AD_BIT_APPEARANCE      (0x00000001 << 11)
#define BTM_BLE_AD_BIT_PUBLIC_ADDR     (0x00000001 << 12)
#define BTM_BLE_AD_BIT_RANDOM_ADDR     (0x00000001 << 13)
#define BTM_BLE_AD_BIT_SERVICE_32      (0x00000001 << 4)
#define BTM_BLE_AD_BIT_SERVICE_32SOL   (0x00000001 << 14)
#define BTM_BLE_AD_BIT_PROPRIETARY     (0x00000001 << 15)
#define BTM_BLE_AD_BIT_SERVICE_128     (0x00000001 << 16)      /*128-bit Service UUIDs*/

/* type of protocol mode */
#define BTA_HH_PROTO_RPT_MODE                   (0x00)
#define BTA_HH_PROTO_BOOT_MODE                  (0x01)
#define BTA_HH_PROTO_UNKNOWN                    (0xff)

#define BSA_DM_BLE_AD_DATA_LEN          31   /*BLE Advertisement data size limit, stack takes 31bytes of data */
#define BSA_DM_BLE_AD_UUID_MAX          6   /*Max number of Service UUID the device can advertise*/
#define APP_DISC_NB_DEVICES		20
#define BSA_BLE_CL_WRITE_MAX		100
#define GATT_MAX_ATTR_LEN		600
#define APP_BLE_MAIN_DEFAULT_APPL_UUID	9000
#define BLE_ADV_APPEARANCE_DATA         0x832	/* Generic Heart rate Sensor */
#define APP_BLE_ADV_VALUE_LEN           8  /*This is temporary value, Total Adv data including all fields should be <31bytes*/
#define BTA_GATTS_INVALID_APP   	0xff
#define BTA_GATTS_INVALID_IF    	0
#define BSA_BLE_MAX_ATTR_LEN  		  GATT_MAX_ATTR_LEN
#define BSA_DM_BLE_ADV_FLAG_MASK          BTM_BLE_ADV_FLAG_MASK
#define BSA_DM_BLE_AD_BIT_DEV_NAME        BTM_BLE_AD_BIT_DEV_NAME
#define BSA_DM_BLE_AD_BIT_FLAGS           BTM_BLE_AD_BIT_FLAGS
#define BSA_DM_BLE_AD_BIT_MANU            BTM_BLE_AD_BIT_MANU
#define BSA_DM_BLE_AD_BIT_TX_PWR          BTM_BLE_AD_BIT_TX_PWR
#define BSA_DM_BLE_AD_BIT_INT_RANGE       BTM_BLE_AD_BIT_INT_RANGE
#define BSA_DM_BLE_AD_BIT_SERVICE         BTM_BLE_AD_BIT_SERVICE
#define BSA_DM_BLE_AD_BIT_SERVICE_SOL     BTM_BLE_AD_BIT_SERVICE_SOL
#define BSA_DM_BLE_AD_BIT_SERVICE_DATA    BTM_BLE_AD_BIT_SERVICE_DATA
#define BSA_DM_BLE_AD_BIT_SIGN_DATA       BTM_BLE_AD_BIT_SIGN_DATA
#define BSA_DM_BLE_AD_BIT_SERVICE_128     BTM_BLE_AD_BIT_SERVICE_128
#define BSA_DM_BLE_AD_BIT_SERVICE_128SOL  BTM_BLE_AD_BIT_SERVICE_128SOL
#define BSA_DM_BLE_AD_BIT_APPEARANCE      BTM_BLE_AD_BIT_APPEARANCE
#define BSA_DM_BLE_AD_BIT_PUBLIC_ADDR     BTM_BLE_AD_BIT_PUBLIC_ADDR
#define BSA_DM_BLE_AD_BIT_RANDOM_ADDR     BTM_BLE_AD_BIT_RANDOM_ADDR
#define BSA_DM_BLE_AD_BIT_PROPRIETARY     BTM_BLE_AD_BIT_PROPRIETARY


#define BTM_BLE_SEC_NONE                0
#define BTM_BLE_SEC_ENCRYPT             1 /* encrypt the link using current key */
#define BTM_BLE_SEC_ENCRYPT_NO_MITM     2
#define BTM_BLE_SEC_ENCRYPT_MITM        3

#define BT_TRANSPORT_BR_EDR    		1
#define BT_TRANSPORT_LE        		2

/* Maximum UUID size - 16 bytes, and structure to hold any type of UUID. */
#define MAX_UUID_SIZE			16

/*   GATT_READ_MULTIPLE request data
*/
#define GATT_MAX_READ_MULTI_HANDLES      10           /* Max attributes to read in one request */

#define BTA_DM_BLE_SEC_NONE		BTM_BLE_SEC_NONE
#define BTA_DM_BLE_SEC_ENCRYPT		BTM_BLE_SEC_ENCRYPT
#define BTA_DM_BLE_SEC_NO_MITM		BTM_BLE_SEC_ENCRYPT_NO_MITM
#define BTA_DM_BLE_SEC_MITM		BTM_BLE_SEC_ENCRYPT_MITM
#define BTA_GATTC_MULTI_MAX		GATT_MAX_READ_MULTI_HANDLES

/* type of protocol mode */
#define BSA_HH_PROTO_RPT_MODE		BTA_HH_PROTO_RPT_MODE
#define BSA_HH_PROTO_BOOT_MODE		BTA_HH_PROTO_BOOT_MODE
#define BSA_HH_PROTO_UNKNOWN		BTA_HH_PROTO_UNKNOWN

typedef tBTA_HH_PROTO_MODE tBSA_HH_PROTO_MODE;
typedef UINT8   tBTM_BLE_SEC_ACT;
typedef UINT8   tGATT_IF;
typedef UINT8   tBTA_GATTC_ATTR_TYPE;
typedef UINT16  tBTA_GATT_REASON;
typedef UINT16  tBTA_GATT_PERM;
typedef UINT8   tBTA_GATT_CHAR_PROP;
typedef UINT8   tBTA_GATT_TRANSPORT;
typedef UINT8   tBTA_GATTC_WRITE_TYPE;
typedef UINT8   tBT_TRANSPORT;
typedef UINT8   tGATT_TRANSPORT;
typedef UINT32  tBSA_DM_CONFIG_MASK;
typedef UINT32  tBSA_DM_BLE_AD_MASK;


enum {
	BTA_GATTC_ATTR_TYPE_INCL_SRVC,
	BTA_GATTC_ATTR_TYPE_CHAR,
	BTA_GATTC_ATTR_TYPE_CHAR_DESCR,
	BTA_GATTC_ATTR_TYPE_SRVC
};

typedef struct {
#define LEN_UUID_16     2
#define LEN_UUID_32     4
#define LEN_UUID_128    16
	UINT16          len;
	union
	{
		UINT16  uuid16;
		UINT32  uuid32;
		UINT8   uuid128[MAX_UUID_SIZE];
	} uu;
} tBT_UUID;

typedef struct {
	tBT_UUID  uuid;           /* uuid of the attribute */
	UINT8     inst_id;        /* instance ID */
} tBTA_GATT_ID;

typedef struct {
	tBTA_GATT_ID id;
	BOOLEAN      is_primary;
} tBTA_GATT_SRVC_ID;

typedef struct {
	tBTA_GATT_SRVC_ID srvc_id;
	tBTA_GATT_ID      char_id;
} tBTA_GATTC_CHAR_ID;

typedef struct {
	tBTA_GATTC_CHAR_ID char_id;
	tBTA_GATT_ID       descr_id;
} tBTA_GATTC_CHAR_DESCR_ID;

typedef struct {
	UINT8                       num_pres_fmt;   /* number of presentation format aggregated*/
	tBTA_GATTC_CHAR_DESCR_ID    pre_format[BTA_GATTC_MULTI_MAX];
} tBTA_GATT_CHAR_AGGRE_VALUE;

typedef struct {
	UINT16  len;
	UINT8   *p_value;
} tBTA_GATT_UNFMT;

typedef union {
	tBTA_GATT_CHAR_AGGRE_VALUE  aggre_value;
	tBTA_GATT_UNFMT             unformat;

} tBTA_GATT_READ_VAL;

typedef struct {
	UINT16  low;
	UINT16  hi;
} tBSA_DM_BLE_INT_RANGE;

typedef struct {
	UINT8       adv_type;
	UINT8       len;
	UINT8       *p_val;     /* number of len byte */
} tBTA_BLE_PROP_ELEM;

/* vendor proprietary adv type */
typedef struct {
	UINT8                   num_elem;
	tBTA_BLE_PROP_ELEM      *p_elem;
} tBSA_DM_BLE_PROPRIETARY;

typedef struct {
	BOOLEAN list_cmpl;
	UINT8 uuid128[MAX_UUID_SIZE];
} tBSA_DM_BLE_128SERVICE;



/* BLE Advertisement configuration parameters */
typedef struct {
	UINT8                     len; /* Number of bytes of data to be advertised */
	UINT8                     flag; /* AD flag value to be set */
	UINT8                     p_val[BSA_DM_BLE_AD_DATA_LEN];/* Data to be advertised */
	tBSA_DM_BLE_AD_MASK       adv_data_mask; /* Data Mask: Eg: BTM_BLE_AD_BIT_FLAGS, BTM_BLE_AD_BIT_MANU */
	UINT16                    appearance_data;     /* Device appearance data */
	UINT8                     num_service; /* number of services */
	UINT16                    uuid_val[BSA_DM_BLE_AD_UUID_MAX];
	/* for DataType Service Data - 0x16 */
	UINT8                     service_data_len; /* length = AD type + service data uuid + data) */
	tBT_UUID                  service_data_uuid; /* service data uuid */
	UINT8                     service_data_val[BSA_DM_BLE_AD_DATA_LEN];
	BOOLEAN                   is_scan_rsp;  /* is the data scan response or adv data */
	UINT8                     tx_power;
	tBSA_DM_BLE_INT_RANGE     int_range;
	tBSA_DM_BLE_128SERVICE    services_128b;
	tBSA_DM_BLE_128SERVICE    sol_service_128b;
	tBSA_DM_BLE_PROPRIETARY   elem;
} tBSA_DM_BLE_ADV_CONFIG;

typedef tGATT_IF                   tBSA_BLE_IF;
typedef tBTA_GATTC_CHAR_ID         tBSA_BLE_CL_CHAR_ID;
typedef tBTA_GATTC_CHAR_DESCR_ID   tBSA_BLE_CL_CHAR_DESCR_ID;
typedef tBTA_GATT_ID               tBSA_BLE_ID;
typedef tBTA_GATT_READ_VAL         tBSA_BLE_READ_VAL;
typedef tBTA_GATT_REASON           tBSA_BLE_REASON;
typedef tBTA_GATT_PERM             tBSA_BLE_PERM;
typedef tBTA_GATT_CHAR_PROP        tBSA_BLE_CHAR_PROP;
typedef tBTA_GATT_TRANSPORT        tBSA_BLE_TRANSPORT;
typedef tBTA_GATTC_WRITE_TYPE      tBSA_BLE_WRITE_TYPE;
typedef tBTM_BLE_SEC_ACT           tBTA_DM_BLE_SEC_ACT;

/* Attribute permissions
*/
#define GATT_PERM_READ              		(1 << 0) /* bit 0 */
#define GATT_PERM_READ_ENCRYPTED    		(1 << 1) /* bit 1 */
#define GATT_PERM_READ_ENC_MITM     		(1 << 2) /* bit 2 */
#define GATT_PERM_WRITE             		(1 << 4) /* bit 4 */
#define GATT_PERM_WRITE_ENCRYPTED   		(1 << 5) /* bit 5 */
#define GATT_PERM_WRITE_ENC_MITM    		(1 << 6) /* bit 6 */
#define GATT_PERM_WRITE_SIGNED      		(1 << 7) /* bit 7 */
#define GATT_PERM_WRITE_SIGNED_MITM 		(1 << 8) /* bit 8 */

#define BTA_GATT_PERM_READ              	GATT_PERM_READ              /* bit 0 -  0x0001 */
#define BTA_GATT_PERM_READ_ENCRYPTED    	GATT_PERM_READ_ENCRYPTED    /* bit 1 -  0x0002 */
#define BTA_GATT_PERM_READ_ENC_MITM     	GATT_PERM_READ_ENC_MITM     /* bit 2 -  0x0004 */
#define BTA_GATT_PERM_WRITE             	GATT_PERM_WRITE             /* bit 4 -  0x0010 */
#define BTA_GATT_PERM_WRITE_ENCRYPTED   	GATT_PERM_WRITE_ENCRYPTED   /* bit 5 -  0x0020 */
#define BTA_GATT_PERM_WRITE_ENC_MITM    	GATT_PERM_WRITE_ENC_MITM    /* bit 6 -  0x0040 */
#define BTA_GATT_PERM_WRITE_SIGNED      	GATT_PERM_WRITE_SIGNED      /* bit 7 -  0x0080 */
#define BTA_GATT_PERM_WRITE_SIGNED_MITM 	GATT_PERM_WRITE_SIGNED_MITM /* bit 8 -  0x0100 */

#define BSA_GATT_PERM_READ              	BTA_GATT_PERM_READ              /* bit 0 -  0x0001 */
#define BSA_GATT_PERM_READ_ENCRYPTED    	BTA_GATT_PERM_READ_ENCRYPTED    /* bit 1 -  0x0002 */
#define BSA_GATT_PERM_READ_ENC_MITM     	BTA_GATT_PERM_READ_ENC_MITM     /* bit 2 -  0x0004 */
#define BSA_GATT_PERM_WRITE             	BTA_GATT_PERM_WRITE             /* bit 4 -  0x0010 */
#define BSA_GATT_PERM_WRITE_ENCRYPTED   	BTA_GATT_PERM_WRITE_ENCRYPTED   /* bit 5 -  0x0020 */
#define BSA_GATT_PERM_WRITE_ENC_MITM    	BTA_GATT_PERM_WRITE_ENC_MITM    /* bit 6 -  0x0040 */
#define BSA_GATT_PERM_WRITE_SIGNED      	BTA_GATT_PERM_WRITE_SIGNED      /* bit 7 -  0x0080 */
#define BSA_GATT_PERM_WRITE_SIGNED_MITM 	BTA_GATT_PERM_WRITE_SIGNED_MITM /* bit 8 -  0x0100 */
/* End Attribute permissions */

/* Definition of characteristic properties */
#define GATT_CHAR_PROP_BIT_BROADCAST    	(1 << 0)
#define GATT_CHAR_PROP_BIT_READ         	(1 << 1)
#define GATT_CHAR_PROP_BIT_WRITE_NR     	(1 << 2)
#define GATT_CHAR_PROP_BIT_WRITE        	(1 << 3)
#define GATT_CHAR_PROP_BIT_NOTIFY       	(1 << 4)
#define GATT_CHAR_PROP_BIT_INDICATE     	(1 << 5)
#define GATT_CHAR_PROP_BIT_AUTH         	(1 << 6)
#define GATT_CHAR_PROP_BIT_EXT_PROP     	(1 << 7)

#define BTA_GATT_CHAR_PROP_BIT_BROADCAST	GATT_CHAR_PROP_BIT_BROADCAST    /* 0x01 */
#define BTA_GATT_CHAR_PROP_BIT_READ		GATT_CHAR_PROP_BIT_READ    /* 0x02 */
#define BTA_GATT_CHAR_PROP_BIT_WRITE_NR		GATT_CHAR_PROP_BIT_WRITE_NR    /* 0x04 */
#define BTA_GATT_CHAR_PROP_BIT_WRITE		GATT_CHAR_PROP_BIT_WRITE       /* 0x08 */
#define BTA_GATT_CHAR_PROP_BIT_NOTIFY		GATT_CHAR_PROP_BIT_NOTIFY      /* 0x10 */
#define BTA_GATT_CHAR_PROP_BIT_INDICATE		GATT_CHAR_PROP_BIT_INDICATE    /* 0x20 */
#define BTA_GATT_CHAR_PROP_BIT_AUTH		GATT_CHAR_PROP_BIT_AUTH        /* 0x40 */
#define BTA_GATT_CHAR_PROP_BIT_EXT_PROP		GATT_CHAR_PROP_BIT_EXT_PROP    /* 0x80 */

#define BSA_GATT_CHAR_PROP_BIT_BROADCAST	BTA_GATT_CHAR_PROP_BIT_BROADCAST   /* 0x01 */
#define BSA_GATT_CHAR_PROP_BIT_READ		BTA_GATT_CHAR_PROP_BIT_READ        /* 0x02 */
#define BSA_GATT_CHAR_PROP_BIT_WRITE_NR		BTA_GATT_CHAR_PROP_BIT_WRITE_NR    /* 0x04 */
#define BSA_GATT_CHAR_PROP_BIT_WRITE		BTA_GATT_CHAR_PROP_BIT_WRITE       /* 0x08 */
#define BSA_GATT_CHAR_PROP_BIT_NOTIFY		BTA_GATT_CHAR_PROP_BIT_NOTIFY      /* 0x10 */
#define BSA_GATT_CHAR_PROP_BIT_INDICATE		BTA_GATT_CHAR_PROP_BIT_INDICATE    /* 0x20 */
#define BSA_GATT_CHAR_PROP_BIT_AUTH		BTA_GATT_CHAR_PROP_BIT_AUTH        /* 0x40 */
#define BSA_GATT_CHAR_PROP_BIT_EXT_PROP		BTA_GATT_CHAR_PROP_BIT_EXT_PROP    /* 0x80 */
/* End characteristic properties */

#define BSA_GATTC_ATTR_TYPE_INCL_SRVC		BTA_GATTC_ATTR_TYPE_INCL_SRVC
#define BSA_GATTC_ATTR_TYPE_CHAR		BTA_GATTC_ATTR_TYPE_CHAR
#define BSA_GATTC_ATTR_TYPE_CHAR_DESCR		BTA_GATTC_ATTR_TYPE_CHAR_DESCR
#define BSA_GATTC_ATTR_TYPE_SRVC		BTA_GATTC_ATTR_TYPE_SRVC
typedef tBTA_GATTC_ATTR_TYPE			tBSA_GATTC_ATTR_TYPE;

/* Max client application BSA BLE Client can support */
#ifndef BSA_BLE_CLIENT_MAX
#define BSA_BLE_CLIENT_MAX    			3
#endif

/* Max server application BSA BLE Server can support */
#define BSA_BLE_SERVER_MAX    			4
#define BSA_BLE_ATTRIBUTE_MAX 			50

#ifndef BSA_BLE_SERVER_SECURITY
#define BSA_BLE_SERVER_SECURITY BTA_DM_BLE_SEC_NONE
#endif

#define BSA_BLE_INVALID_IF         		0xff
#define BSA_BLE_INVALID_CONN       		0xffff

/* Define common 16-bit service class UUIDs
*/
#define UUID_SERVCLASS_SERVICE_DISCOVERY_SERVER 0X1000
#define UUID_SERVCLASS_BROWSE_GROUP_DESCRIPTOR  0X1001
#define UUID_SERVCLASS_PUBLIC_BROWSE_GROUP      0X1002
#define UUID_SERVCLASS_SERIAL_PORT              0X1101
#define UUID_SERVCLASS_LAN_ACCESS_USING_PPP     0X1102
#define UUID_SERVCLASS_DIALUP_NETWORKING        0X1103
#define UUID_SERVCLASS_IRMC_SYNC                0X1104
#define UUID_SERVCLASS_OBEX_OBJECT_PUSH         0X1105
#define UUID_SERVCLASS_OBEX_FILE_TRANSFER       0X1106
#define UUID_SERVCLASS_IRMC_SYNC_COMMAND        0X1107
#define UUID_SERVCLASS_HEADSET                  0X1108
#define UUID_SERVCLASS_CORDLESS_TELEPHONY       0X1109
#define UUID_SERVCLASS_AUDIO_SOURCE             0X110A
#define UUID_SERVCLASS_AUDIO_SINK               0X110B
#define UUID_SERVCLASS_AV_REM_CTRL_TARGET       0X110C  /* Audio/Video Control profile */
#define UUID_SERVCLASS_ADV_AUDIO_DISTRIBUTION   0X110D  /* Advanced Audio Distribution profile */
#define UUID_SERVCLASS_AV_REMOTE_CONTROL        0X110E  /* Audio/Video Control profile */
#define UUID_SERVCLASS_AV_REM_CTRL_CONTROL      0X110F  /* Audio/Video Control profile */
#define UUID_SERVCLASS_INTERCOM                 0X1110
#define UUID_SERVCLASS_FAX                      0X1111
#define UUID_SERVCLASS_HEADSET_AUDIO_GATEWAY    0X1112
#define UUID_SERVCLASS_WAP                      0X1113
#define UUID_SERVCLASS_WAP_CLIENT               0X1114
#define UUID_SERVCLASS_PANU                     0X1115  /* PAN profile */
#define UUID_SERVCLASS_NAP                      0X1116  /* PAN profile */
#define UUID_SERVCLASS_GN                       0X1117  /* PAN profile */
#define UUID_SERVCLASS_DIRECT_PRINTING          0X1118  /* BPP profile */
#define UUID_SERVCLASS_REFERENCE_PRINTING       0X1119  /* BPP profile */
#define UUID_SERVCLASS_IMAGING                  0X111A  /* Imaging profile */
#define UUID_SERVCLASS_IMAGING_RESPONDER        0X111B  /* Imaging profile */
#define UUID_SERVCLASS_IMAGING_AUTO_ARCHIVE     0X111C  /* Imaging profile */
#define UUID_SERVCLASS_IMAGING_REF_OBJECTS      0X111D  /* Imaging profile */
#define UUID_SERVCLASS_HF_HANDSFREE             0X111E  /* Handsfree profile */
#define UUID_SERVCLASS_AG_HANDSFREE             0X111F  /* Handsfree profile */
#define UUID_SERVCLASS_DIR_PRT_REF_OBJ_SERVICE  0X1120  /* BPP profile */
#define UUID_SERVCLASS_REFLECTED_UI             0X1121  /* BPP profile */
#define UUID_SERVCLASS_BASIC_PRINTING           0X1122  /* BPP profile */
#define UUID_SERVCLASS_PRINTING_STATUS          0X1123  /* BPP profile */
#define UUID_SERVCLASS_HUMAN_INTERFACE          0X1124  /* HID profile */
#define UUID_SERVCLASS_CABLE_REPLACEMENT        0X1125  /* HCRP profile */
#define UUID_SERVCLASS_HCRP_PRINT               0X1126  /* HCRP profile */
#define UUID_SERVCLASS_HCRP_SCAN                0X1127  /* HCRP profile */
#define UUID_SERVCLASS_COMMON_ISDN_ACCESS       0X1128  /* CAPI Message Transport Protocol*/
#define UUID_SERVCLASS_VIDEO_CONFERENCING_GW    0X1129  /* Video Conferencing profile */
#define UUID_SERVCLASS_UDI_MT                   0X112A  /* Unrestricted Digital Information profile */
#define UUID_SERVCLASS_UDI_TA                   0X112B  /* Unrestricted Digital Information profile */
#define UUID_SERVCLASS_VCP                      0X112C  /* Video Conferencing profile */
#define UUID_SERVCLASS_SAP                      0X112D  /* SIM Access profile */
#define UUID_SERVCLASS_PBAP_PCE                 0X112E  /* Phonebook Access - PCE */
#define UUID_SERVCLASS_PBAP_PSE                 0X112F  /* Phonebook Access - PSE */
#define UUID_SERVCLASS_PHONE_ACCESS             0x1130
#define UUID_SERVCLASS_HEADSET_HS               0x1131  /* Headset - HS, from HSP v1.2 */
#define UUID_SERVCLASS_3DD                      0x1137  /* 3D Sync (Display role) */
#define UUID_SERVCLASS_3DG                      0x1138  /* 3D Sync (Glasses role) */
#define UUID_SERVCLASS_3DS                      0x1139  /* 3D Sync Profile */
#define UUID_SERVCLASS_PNP_INFORMATION          0X1200  /* Device Identification */
#define UUID_SERVCLASS_GENERIC_NETWORKING       0X1201
#define UUID_SERVCLASS_GENERIC_FILETRANSFER     0X1202
#define UUID_SERVCLASS_GENERIC_AUDIO            0X1203
#define UUID_SERVCLASS_GENERIC_TELEPHONY        0X1204
#define UUID_SERVCLASS_UPNP_SERVICE             0X1205  /* UPNP_Service [ESDP] */
#define UUID_SERVCLASS_UPNP_IP_SERVICE          0X1206  /* UPNP_IP_Service [ESDP] */
#define UUID_SERVCLASS_ESDP_UPNP_IP_PAN         0X1300  /* UPNP_IP_PAN [ESDP] */
#define UUID_SERVCLASS_ESDP_UPNP_IP_LAP         0X1301  /* UPNP_IP_LAP [ESDP] */
#define UUID_SERVCLASS_ESDP_UPNP_IP_L2CAP       0X1302  /* UPNP_L2CAP [ESDP] */
#define UUID_SERVCLASS_VIDEO_SOURCE             0X1303  /* Video Distribution Profile (VDP) */
#define UUID_SERVCLASS_VIDEO_SINK               0X1304  /* Video Distribution Profile (VDP) */
#define UUID_SERVCLASS_VIDEO_DISTRIBUTION       0X1305  /* Video Distribution Profile (VDP) */
#define UUID_SERVCLASS_HDP_PROFILE              0X1400  /* Health Device profile (HDP) */
#define UUID_SERVCLASS_HDP_SOURCE               0X1401  /* Health Device profile (HDP) */
#define UUID_SERVCLASS_HDP_SINK                 0X1402  /* Health Device profile (HDP) */
#define UUID_SERVCLASS_MAP_PROFILE              0X1134  /* MAP profile UUID */
#define UUID_SERVCLASS_MESSAGE_ACCESS           0X1132  /* Message Access Service UUID */
#define UUID_SERVCLASS_MESSAGE_NOTIFICATION     0X1133  /* Message Notification Service UUID */

#define UUID_SERVCLASS_GAP_SERVER               0x1800
#define UUID_SERVCLASS_GATT_SERVER              0x1801
#define UUID_SERVCLASS_IMMEDIATE_ALERT          0x1802      /* immediate alert */
#define UUID_SERVCLASS_LINKLOSS                 0x1803      /* Link Loss Alert */
#define UUID_SERVCLASS_TX_POWER                 0x1804      /* TX power */
#define UUID_SERVCLASS_CURRENT_TIME             0x1805      /* Link Loss Alert */
#define UUID_SERVCLASS_DST_CHG                  0x1806      /* DST Time change */
#define UUID_SERVCLASS_REF_TIME_UPD             0x1807      /* reference time update */
#define UUID_SERVCLASS_THERMOMETER              0x1809      /* Thermometer UUID */
#define UUID_SERVCLASS_DEVICE_INFO              0x180A      /* device info service */
#define UUID_SERVCLASS_NWA                      0x180B      /* Network availability */
#define UUID_SERVCLASS_HEART_RATE               0x180D      /* Heart Rate service */
#define UUID_SERVCLASS_PHALERT                  0x180E      /* phone alert service */
#define UUID_SERVCLASS_BATTERY                  0x180F     /* battery service */
#define UUID_SERVCLASS_BPM                      0x1810      /*  blood pressure service */
#define UUID_SERVCLASS_ALERT_NOTIFICATION       0x1811      /* alert notification service */
#define UUID_SERVCLASS_LE_HID                   0x1812     /*  HID over LE */
#define UUID_SERVCLASS_SCAN_PARAM               0x1813      /* Scan Parameter service */
#define UUID_SERVCLASS_GLUCOSE                  0x1808      /* Glucose Meter Service */
#define UUID_SERVCLASS_RSC                      0x1814      /* Runners Speed and Cadence Service */
#define UUID_SERVCLASS_CSC                      0x1816      /* Cycling Speed and Cadence Service */
#define UUID_SERVCLASS_BAV_SOURCE               0x8000      /* Broadcast AV service*/ /* BSA_SPECIFIC */
#define UUID_SERVCLASS_CP                       0x1818      /* Cycling Power Service (pre-Adoption IOP) */
#define UUID_SERVCLASS_LN                       0x1819      /* Location and Navigation Service (pre-Adoption IOP) */

#define UUID_SERVCLASS_TEST_SERVER              0x9000      /* Test Group UUID */

#define BSA_BLE_UUID_SERVCLASS_GAP_SERVER                    UUID_SERVCLASS_GAP_SERVER
#define BSA_BLE_UUID_SERVCLASS_GATT_SERVER                   UUID_SERVCLASS_GATT_SERVER
#define BSA_BLE_UUID_SERVCLASS_IMMEDIATE_ALERT               UUID_SERVCLASS_IMMEDIATE_ALERT
#define BSA_BLE_UUID_SERVCLASS_LINKLOSS                      UUID_SERVCLASS_LINKLOSS
#define BSA_BLE_UUID_SERVCLASS_TX_POWER                      UUID_SERVCLASS_TX_POWER
#define BSA_BLE_UUID_SERVCLASS_CURRENT_TIME                  UUID_SERVCLASS_CURRENT_TIME
#define BSA_BLE_UUID_SERVCLASS_DST_CHG                       UUID_SERVCLASS_DST_CHG
#define BSA_BLE_UUID_SERVCLASS_REF_TIME_UPD                  UUID_SERVCLASS_REF_TIME_UPD
#define BSA_BLE_UUID_SERVCLASS_GLUCOSE                       UUID_SERVCLASS_GLUCOSE
#define BSA_BLE_UUID_SERVCLASS_HEALTH_THERMOMETER            UUID_SERVCLASS_THERMOMETER
#define BSA_BLE_UUID_SERVCLASS_DEVICE_INFORMATION            UUID_SERVCLASS_DEVICE_INFO
#define BSA_BLE_UUID_SERVCLASS_NWA                           UUID_SERVCLASS_NWA
#define BSA_BLE_UUID_SERVCLASS_PHALERT                       UUID_SERVCLASS_PHALERT
#define BSA_BLE_UUID_SERVCLASS_HEART_RATE                    UUID_SERVCLASS_HEART_RATE
#define BSA_BLE_UUID_SERVCLASS_BATTERY_SERVICE               UUID_SERVCLASS_BATTERY
#define BSA_BLE_UUID_SERVCLASS_BLOOD_PRESSURE                UUID_SERVCLASS_BPM
#define BSA_BLE_UUID_SERVCLASS_ALERT_NOTIFICATION_SERVICE    UUID_SERVCLASS_ALERT_NOTIFICATION
#define BSA_BLE_UUID_SERVCLASS_HUMAN_INTERFACE_DEVICE        UUID_SERVCLASS_LE_HID
#define BSA_BLE_UUID_SERVCLASS_SCAN_PARAMETERS               UUID_SERVCLASS_SCAN_PARAM
#define BSA_BLE_UUID_SERVCLASS_RUNNING_SPEED_AND_CADENCE     UUID_SERVCLASS_RSC
#define BSA_BLE_UUID_SERVCLASS_CYCLING_SPEED_AND_CADENCE     UUID_SERVCLASS_CSC
#define BSA_BLE_UUID_SERVCLASS_TEST_SERVER                   UUID_SERVCLASS_TEST_SERVER

/* GATT attribute types
*/
#define GATT_UUID_PRI_SERVICE           	0x2800
#define GATT_UUID_SEC_SERVICE           	0x2801
#define GATT_UUID_INCLUDE_SERVICE       	0x2802
#define GATT_UUID_CHAR_DECLARE          	0x2803      /*  Characteristic Declaration*/

#define GATT_UUID_CHAR_EXT_PROP         	0x2900      /*	Characteristic Extended Properties */
#define GATT_UUID_CHAR_DESCRIPTION      	0x2901      /*  Characteristic User Description*/
#define GATT_UUID_CHAR_CLIENT_CONFIG    	0x2902      /*  Client Characteristic Configuration */
#define GATT_UUID_CHAR_SRVR_CONFIG      	0x2903      /*  Server Characteristic Configuration */
#define GATT_UUID_CHAR_PRESENT_FORMAT   	0x2904      /*  Characteristic Presentation Format*/
#define GATT_UUID_CHAR_AGG_FORMAT       	0x2905      /*  Characteristic Aggregate Format*/
#define GATT_UUID_CHAR_VALID_RANGE       	0x2906      /*  Characteristic Valid Range */
#define GATT_UUID_EXT_RPT_REF_DESCR     	0x2907
#define GATT_UUID_RPT_REF_DESCR         	0x2908

/* GAP Profile Attributes
*/
#define GATT_UUID_GAP_DEVICE_NAME       	0x2A00
#define GATT_UUID_GAP_ICON              	0x2A01
#define GATT_UUID_GAP_PREF_CONN_PARAM   	0x2A04
#define GATT_UUID_GAP_CENTRAL_ADDR_RESOL    	0x2AA6

/* Attribute Profile Attribute UUID */
#define GATT_UUID_GATT_SRV_CHGD         	0x2A05
/* Attribute Protocol Test */

/* Link Loss Service */
#define GATT_UUID_ALERT_LEVEL           	0x2A06      /* Alert Level */
#define GATT_UUID_TX_POWER_LEVEL        	0x2A07      /* TX power level */

/* Time Profile */
/* Current Time Service */
#define GATT_UUID_CURRENT_TIME          	0x2A2B      /* Current Time */
#define GATT_UUID_LOCAL_TIME_INFO       	0x2A0F      /* Local time info */
#define GATT_UUID_REF_TIME_INFO         	0x2A14      /* reference time information */

/* NwA Profile */
#define GATT_UUID_NW_STATUS             	0x2A18      /* network availability status */
#define GATT_UUID_NW_TRIGGER            	0x2A1A      /* Network availability trigger */

/* phone alert */
#define GATT_UUID_ALERT_STATUS          	0x2A40    /* alert status */
#define GATT_UUID_RINGER_CP             	0x2A42    /* ringer control point */
#define GATT_UUID_RINGER_SETTING        	0x2A41    /* ringer setting */

/* Glucose Service */
#define GATT_UUID_GM_MEASUREMENT        	0x2A18
#define GATT_UUID_GM_CONTEXT            	0x2A34
#define GATT_UUID_GM_CONTROL_POINT      	0x2A52
#define GATT_UUID_GM_FEATURE            	0x2A51

/* device infor characteristic */
#define GATT_UUID_SYSTEM_ID             	0x2A23
#define GATT_UUID_MODEL_NUMBER_STR      	0x2A24
#define GATT_UUID_SERIAL_NUMBER_STR     	0x2A25
#define GATT_UUID_FW_VERSION_STR        	0x2A26
#define GATT_UUID_HW_VERSION_STR        	0x2A27
#define GATT_UUID_SW_VERSION_STR        	0x2A28
#define GATT_UUID_MANU_NAME             	0x2A29
#define GATT_UUID_IEEE_DATA             	0x2A2A
#define GATT_UUID_PNP_ID                	0x2A50

/* HID characteristics */
#define GATT_UUID_HID_INFORMATION       	0x2A4A
#define GATT_UUID_HID_REPORT_MAP        	0x2A4B
#define GATT_UUID_HID_CONTROL_POINT     	0x2A4C
#define GATT_UUID_HID_REPORT            	0x2A4D
#define GATT_UUID_HID_PROTO_MODE        	0x2A4E
#define GATT_UUID_HID_BT_KB_INPUT       	0x2A22
#define GATT_UUID_HID_BT_KB_OUTPUT      	0x2A32
#define GATT_UUID_HID_BT_MOUSE_INPUT    	0x2A33

/* Battery Service char */
#define GATT_UUID_BATTERY_LEVEL         	0x2A19

#define GATT_UUID_SC_CONTROL_POINT      	0x2A55
#define GATT_UUID_SENSOR_LOCATION       	0x2A5D

/* RUNNERS SPEED AND CADENCE SERVICE */
#define GATT_UUID_RSC_MEASUREMENT       	0x2A53
#define GATT_UUID_RSC_FEATURE           	0x2A54

/* CYCLING SPEED AND CADENCE SERVICE */
#define GATT_UUID_CSC_MEASUREMENT       	0x2A5B
#define GATT_UUID_CSC_FEATURE           	0x2A5C

/* CYCLING POWER SERVICE  (Temp for IOP) */
#define GATT_UUID_CP_MEASUREMENT        	0x2A63
#define GATT_UUID_CP_VECTOR             	0x2A64
#define GATT_UUID_CP_FEATURE            	0x2A65
#define GATT_UUID_CP_CONTROL_POINT      	0x2A66

/* LOCATION AND NAVIGATION SERVICE  (Temp for IOP) */
#define GATT_UUID_LN_LOC_AND_SPEED      	0x2A67
#define GATT_UUID_LN_NAVIGATION         	0x2A68
#define GATT_UUID_LN_POSITION_QUALITY   	0x2A69
#define GATT_UUID_LN_FEATURE            	0x2A6A
#define GATT_UUID_LN_CONTROL_POINT      	0x2A6B

/* Scan Parameter charatceristics */
#define GATT_UUID_SCAN_INT_WINDOW       	0x2A4F
#define GATT_UUID_SCAN_REFRESH          	0x2A31

/* Transports for the primary service  */
#define GATT_TRANSPORT_LE           		BT_TRANSPORT_LE
#define GATT_TRANSPORT_BR_EDR       		BT_TRANSPORT_BR_EDR
#define GATT_TRANSPORT_LE_BR_EDR    		(BT_TRANSPORT_LE|BT_TRANSPORT_BR_EDR)

#define BSA_BLE_GATT_UUID_PRI_SERVICE                       GATT_UUID_PRI_SERVICE
#define BSA_BLE_GATT_UUID_SEC_SERVICE                       GATT_UUID_SEC_SERVICE
#define BSA_BLE_GATT_UUID_INCLUDE_SERVICE                   GATT_UUID_INCLUDE_SERVICE
#define BSA_BLE_GATT_UUID_CHAR_DECLARE                      GATT_UUID_CHAR_DECLARE            /*  Characteristic Declaration*/
#define BSA_BLE_GATT_UUID_CHAR_EXT_PROP                     GATT_UUID_CHAR_EXT_PROP           /*  Characteristic Extended Properties */
#define BSA_BLE_GATT_UUID_CHAR_DESCRIPTION                  GATT_UUID_CHAR_DESCRIPTION        /*  Characteristic User Description*/
#define BSA_BLE_GATT_UUID_CHAR_CLIENT_CONFIG                GATT_UUID_CHAR_CLIENT_CONFIG      /*  Client Characteristic Configuration */
#define BSA_BLE_GATT_UUID_CHAR_VALID_RANGE                  GATT_UUID_CHAR_VALID_RANGE        /*  Characteristic Valid Range */

#define BSA_BLE_GATT_UUID_CHAR_CLIENT_CONFIG_ENABLE_NOTI    0x01    /* Enable Notification of Client Characteristic Configuration, defined at bluetooth org */
#define BSA_BLE_GATT_UUID_CHAR_CLIENT_CONFIG_ENABLE_INDI    0x02    /* Enable Indication of Client Characteristic Configuration, defined at bluetooth org */

#define BSA_BLE_GATT_UUID_CHAR_SRVR_CONFIG                  GATT_UUID_CHAR_SRVR_CONFIG        /*  Server Characteristic Configuration */
#define BSA_BLE_GATT_UUID_CHAR_PRESENT_FORMAT               GATT_UUID_CHAR_PRESENT_FORMAT     /*  Characteristic Presentation Format*/
#define BSA_BLE_GATT_UUID_CHAR_AGG_FORMAT                   GATT_UUID_CHAR_AGG_FORMAT         /*  Characteristic Aggregate Format*/
#define BSA_BLE_GATT_UUID_CHAR_VALID_RANGE                  GATT_UUID_CHAR_VALID_RANGE        /*  Characteristic Valid Range */

#define BSA_BLE_GATT_UUID_GAP_DEVICE_NAME                   GATT_UUID_GAP_DEVICE_NAME
#define BSA_BLE_GATT_UUID_GAP_ICON                          GATT_UUID_GAP_ICON
#define BSA_BLE_GATT_UUID_GAP_PRIVACY_FLAG                  GATT_UUID_GAP_PRIVACY_FLAG
#define BSA_BLE_GATT_UUID_GAP_RECONN_ADDR                   GATT_UUID_GAP_RECONN_ADDR
#define BSA_BLE_GATT_UUID_GAP_PREF_CONN_PARAM               GATT_UUID_GAP_PREF_CONN_PARAM

#define BSA_BLE_GATT_UUID_SENSOR_LOCATION                   GATT_UUID_SENSOR_LOCATION

/* Battery Service */
#define BSA_BLE_GATT_UUID_BATTERY_LEVEL                     GATT_UUID_BATTERY_LEVEL

/* device infor characteristic */
#define BSA_BLE_GATT_UUID_SYSTEM_ID                         GATT_UUID_SYSTEM_ID
#define BSA_BLE_GATT_UUID_MODEL_NUMBER_STR                  GATT_UUID_MODEL_NUMBER_STR
#define BSA_BLE_GATT_UUID_SERIAL_NUMBER_STR                 GATT_UUID_SERIAL_NUMBER_STR
#define BSA_BLE_GATT_UUID_FW_VERSION_STR                    GATT_UUID_FW_VERSION_STR
#define BSA_BLE_GATT_UUID_HW_VERSION_STR                    GATT_UUID_HW_VERSION_STR
#define BSA_BLE_GATT_UUID_SW_VERSION_STR                    GATT_UUID_SW_VERSION_STR
#define BSA_BLE_GATT_UUID_MANU_NAME                         GATT_UUID_MANU_NAME
#define BSA_BLE_GATT_UUID_IEEE_DATA                         GATT_UUID_IEEE_DATA
#define BSA_BLE_GATT_UUID_PNP_ID                            GATT_UUID_PNP_ID

/* Link Loss Service */
#define BSA_BLE_GATT_UUID_ALERT_LEVEL                       GATT_UUID_ALERT_LEVEL      /* Alert Level */
#define BSA_BLE_GATT_UUID_TX_POWER_LEVEL                    GATT_UUID_TX_POWER_LEVEL      /* TX power level */

/* Heart Rate Service */
#define BSA_BLE_GATT_UUID_HEART_RATE_MEASUREMENT            0x2A37
#define BSA_BLE_GATT_UUID_BODY_SENSOR_LOCATION              0x2A38
#define BSA_BLE_GATT_UUID_BODY_SENSOR_CONTROL_POINT         0x2A39

/* BLOOD PRESSURE SERVICE */
#define BSA_BLE_GATT_UUID_BLOOD_PRESSURE_FEATURE            0x2A49
#define BSA_BLE_GATT_UUID_BLOOD_PRESSURE_MEASUREMENT        0x2A35
#define BSA_BLE_GATT_UUID_INTERMEDIATE_CUFF_PRESSURE        0x2A36
#define BSA_BLE_GATT_LENGTH_OF_BLOOD_PRESSURE_MEASUREMENT   13

/*HEALTH THERMOMETER SERVICE*/
#define BSA_BLE_GATT_UUID_TEMPERATURE_TYPE                  0x2A1D
#define BSA_BLE_GATT_UUID_TEMPERATURE_MEASUREMENT           0X2A1C
#define BSA_BLE_GATT_UUID_INTERMEDIATE_TEMPERATURE          0x2A1E
#define BSA_BLE_GATT_UUID_TEMPERATURE_MEASUREMENT_INTERVAL  0x2A21

/* CYCLING SPEED AND CADENCE SERVICE      */
#define BSA_BLE_GATT_UUID_CSC_MEASUREMENT                   GATT_UUID_CSC_MEASUREMENT
#define BSA_BLE_GATT_UUID_CSC_FEATURE                       GATT_UUID_CSC_FEATURE
#define BSA_BLE_GATT_LENGTH_OF_CSC_MEASUREMENT              11

/* RUNNERS SPEED AND CADENCE SERVICE      */
#define BSA_BLE_GATT_UUID_RSC_MEASUREMENT                   GATT_UUID_RSC_MEASUREMENT
#define BSA_BLE_GATT_UUID_RSC_FEATURE                       GATT_UUID_RSC_FEATURE
#define BSA_BLE_GATT_LENGTH_OF_RSC_MEASUREMENT              10

#define BSA_BLE_GATT_UUID_SC_CONTROL_POINT                  GATT_UUID_SC_CONTROL_POINT

/* HID characteristics */
#define BSA_BLE_GATT_UUID_HID_INFORMATION                   GATT_UUID_HID_INFORMATION
#define BSA_BLE_GATT_UUID_HID_REPORT_MAP                    GATT_UUID_HID_REPORT_MAP
#define BSA_BLE_GATT_UUID_HID_CONTROL_POINT                 GATT_UUID_HID_CONTROL_POINT
#define BSA_BLE_GATT_UUID_HID_REPORT                        GATT_UUID_HID_REPORT
#define BSA_BLE_GATT_UUID_HID_PROTO_MODE                    GATT_UUID_HID_PROTO_MODE
#define BSA_BLE_GATT_UUID_HID_BT_KB_INPUT                   GATT_UUID_HID_BT_KB_INPUT
#define BSA_BLE_GATT_UUID_HID_BT_KB_OUTPUT                  GATT_UUID_HID_BT_KB_OUTPUT
#define BSA_BLE_GATT_UUID_HID_BT_MOUSE_INPUT                GATT_UUID_HID_BT_MOUSE_INPUT

#define BSA_BLE_GATT_TRANSPORT_LE			    GATT_TRANSPORT_LE
#define BSA_BLE_GATT_TRANSPORT_BR_EDR      		    GATT_TRANSPORT_BR_EDR
#define BSA_BLE_GATT_TRANSPORT_LE_BR_EDR		    GATT_TRANSPORT_LE_BR_EDR

/* Define character set */
#define AVRC_CHAR_SET_SIZE                      2

/* Define the Media Attribute IDs
*/
#define AVRC_MEDIA_ATTR_ID_TITLE                 0x00000001
#define AVRC_MEDIA_ATTR_ID_ARTIST                0x00000002
#define AVRC_MEDIA_ATTR_ID_ALBUM                 0x00000003
#define AVRC_MEDIA_ATTR_ID_TRACK_NUM             0x00000004
#define AVRC_MEDIA_ATTR_ID_NUM_TRACKS            0x00000005
#define AVRC_MEDIA_ATTR_ID_GENRE                 0x00000006
#define AVRC_MEDIA_ATTR_ID_PLAYING_TIME          0x00000007        /* in miliseconds */
#define AVRC_MAX_NUM_MEDIA_ATTR_ID               7

typedef UINT8			tBT_DEVICE_TYPE;
typedef UINT8			BD_ADDR[BD_ADDR_LEN];
typedef UINT8			BD_NAME[BD_NAME_LEN + 1];	/* Device name */
typedef UINT8			DEV_CLASS[DEV_CLASS_LEN];	/* Device class */
typedef UINT16			tBSA_STATUS;
typedef UINT32			tBTA_SERVICE_MASK;
typedef tBTA_SERVICE_MASK   	tBSA_SERVICE_MASK;
typedef UINT16			tGATT_PERM;

typedef enum {
	/* means not currently in call set up */
	CALLSETUP_STATE_NO_CALL_SETUP = 0,
	/* means an incoming call process ongoing */
	CALLSETUP_STATE_INCOMMING_CALL,
	/* means an outgoing call set up is ongoing */
	CALLSETUP_STATE_OUTGOING_CALL,
	/* means remote party being alerted in an outgoing call */
	CALLSETUP_STATE_REMOTE_BEING_ALERTED_IN_OUTGOING_CALL,
	/* means a call is waiting */
	CALLSETUP_STATE_WAITING_CALL,

	/* means there are no calls in progress */
	CALL_STATE_NO_CALLS_ONGOING,
	/* means at least one call is in progress */
	CALL_STATE_LEAST_ONE_CALL_ONGOING,

	/* No calls held */
	CALLHELD_STATE_NO_CALL_HELD,
	/* Call is placed on hold or active/held calls swapped */
	CALLHELD_STATE_PLACED_ON_HOLD_OR_SWAPPED,
	/* Call on hold, no active call */
	CALLHELD_STATE_CALL_ON_HOLD_AND_NO_ACTIVE_CALL

} BSA_CALL_STATE;

typedef enum {
	BT_LINK_DISCONNECTING,
	BT_LINK_DISCONNECTED,
	BT_LINK_CONNECTING,
	BT_LINK_CONNECTED,
} bsa_link_status;

typedef enum {
	BSA_APP_RECONNECT,
	BSA_APP_DISCOVERY
} BSA_OPEN_TYPE;

typedef enum {
	BTHF_CHLD_TYPE_RELEASEHELD,
	BTHF_CHLD_TYPE_RELEASEACTIVE_ACCEPTHELD,
	BTHF_CHLD_TYPE_HOLDACTIVE_ACCEPTHELD,
	BTHF_CHLD_TYPE_ADDHELDTOCONF
} tBSA_BTHF_CHLD_TYPE_T;

enum {
	USE_HS_AVK = 0,
	USE_HS_ONLY,
	USE_AVK_ONLY
};

typedef enum {
	BTHF_VOLUME_TYPE_SPK = 0,	/* Update speaker volume */
	BTHF_VOLUME_TYPE_MIC = 1	/* Update microphone volume */
} tBSA_BTHF_VOLUME_TYPE_T;

/* Discovery callback events */
typedef enum
{
	BSA_DISC_NEW_EVT, 		/* a New Device has been discovered */
	BSA_DISC_CMPL_EVT, 		/* End of Discovery */
	BSA_DISC_DEV_INFO_EVT, 		/* Device Info discovery event */
	BSA_DISC_REMOTE_NAME_EVT  	/* Read remote device name event */
} tBSA_DISC_EVT;

/* Vendor and Product Identification
 * of the peer device
 * */
typedef struct
{
	BOOLEAN valid; 			/* TRUE if this entry is valid */
	UINT16 vendor_id_source; 	/* Indicate if the vendor field is BT or USB */
	UINT16 vendor; 			/* Vendor Id of the peer device */
	UINT16 product; 		/* Product Id of the peer device */
	UINT16 version; 		/* Version of the peer device */
} tBSA_DISC_VID_PID;

typedef struct
{
	BD_ADDR bd_addr; 		/* BD address peer device. */
	DEV_CLASS class_of_device; 	/* Class of Device */
	BD_NAME name; 			/* Name of peer device. */
	int rssi; 			/* The rssi value */
	tBSA_SERVICE_MASK services; 	/* Service discovery discovered */
	tBSA_DISC_VID_PID eir_vid_pid[BSA_DISC_VID_PID_MAX];
	UINT8 eir_data[BSA_EIR_DATA_LENGTH];  /* Full EIR data */
	UINT8 inq_result_type;
	UINT8 ble_addr_type;
	tBT_DEVICE_TYPE device_type;
} tBSA_DISC_REMOTE_DEV;

typedef struct
{
	tBSA_STATUS status;		/* Status of the request */
	BD_ADDR bd_addr;		/* BD address peer device. */
	UINT8 index;
	UINT16 spec_id;
	BOOLEAN primary;
	UINT16 vendor;
	UINT16 vendor_id_source;
	UINT16 product;
	UINT16 version;
} tBSA_DISC_DEV_INFO_MSG;

typedef struct
{
	UINT16      status;
	BD_ADDR     bd_addr;
	UINT16      length;
	BD_NAME     remote_bd_name;
} tBSA_DISC_READ_REMOTE_NAME_MSG;

/* Structure associated with BSA_DISC_NEW_EVT */
typedef tBSA_DISC_REMOTE_DEV tBSA_DISC_NEW_MSG;

/* Union of all Discovery callback structures */
typedef union
{
	tBSA_DISC_NEW_MSG disc_new;			/* a New Device has been discovered */
	tBSA_DISC_DEV_INFO_MSG dev_info;		/* Device Info of a device */
	tBSA_DISC_READ_REMOTE_NAME_MSG remote_name; 	/* Name of Remote device */
} tBSA_DISC_MSG;

typedef struct
{
	BOOLEAN in_use; /* TRUE is this element is used, FALSE otherwise */
	tBSA_DISC_REMOTE_DEV device; /* Device Info */
} tBSA_DISC_DEV;

typedef struct
{
	tBSA_DISC_DEV devs[APP_DISC_NB_DEVICES];
} tAPP_DISCOVERY_CB;

/* data for meta data items */
#define BSA_AVK_ATTR_STR_LEN_MAX 102

/* data for get element attribute response */
#define BSA_AVK_ELEMENT_ATTR_MAX 7

typedef UINT8 tAVRC_STS;

/* media string */
typedef struct
{
	UINT8       data[BSA_AVK_ATTR_STR_LEN_MAX];
	UINT16      charset_id;
	UINT16      str_len;
} tBSA_AVK_STRING;

/* attibute entry */
typedef struct
{
	UINT32             attr_id;
	tBSA_AVK_STRING    name;
} tBSA_AVK_ATTR_ENTRY;

typedef struct
{
	tAVRC_STS               status;
	UINT8                   num_attr;
	tBSA_AVK_ATTR_ENTRY     attr_entry[BSA_AVK_ELEMENT_ATTR_MAX];
} tBSA_AVK_GET_ELEMENT_ATTR_MSG;

/* GetPlayStatus */
typedef struct
{
	UINT8       pdu;
	tAVRC_STS   status;
	UINT8       opcode;         /* Op Code (copied from avrc_cmd.opcode by AVRC_BldResponse user. invalid one to generate according to pdu) */
	UINT32      song_len;
	UINT32      song_pos;
	UINT8       play_status;
} tAVRC_GET_PLAY_STATUS_RSP;

typedef tAVRC_GET_PLAY_STATUS_RSP   tBSA_AVK_GET_PLAY_STATUS_MSG;

typedef struct {
	char	*in_buffer;
	char	*out_buffer;
	UINT32	in_len;
	UINT32	out_len;
	UINT32 	sample_rate;
	UINT32 	sample_channel;
	UINT32 	sample_bits;
} avk_callback_msg;

typedef void (tBSA_DISC_CBACK)(tBSA_DISC_EVT event, tBSA_DISC_MSG *p_data);
typedef unsigned int (*aec_func_t)(void);
typedef void (*aec_calculate_t)(void *buf_mic, void *buf_ref, void *buf_result, unsigned int size);

typedef struct {
	char *in_buffer;
	char *out_buffer;
	unsigned int in_len;
	unsigned int out_len;
} bt_aec_resample_msg;

typedef void (*bt_aec_resample_t)(bt_aec_resample_msg *bt_aec_rmsg);

typedef struct {
	char *in_buffer;
	char *out_buffer;
	unsigned int in_len;
	unsigned int out_len;
} hs_resample_msg;

typedef void (*hs_resample_t)(hs_resample_msg *hs_msg);

typedef struct bt_aec_callback_interface {
	aec_func_t aec_enable;
	aec_func_t aec_get_buffer_length;
	aec_calculate_t aec_calculate;
	aec_func_t aec_init;
	aec_func_t aec_destroy;
} bt_aec_callback;

typedef struct {
	int sample_rate;
	int sample_channel;
	int sample_bits;
} hs_sample_init_data;

typedef struct {
	int sample_rate;
	int sample_channel;
	int sample_bits;
} bt_aec_sample_init_data;

typedef struct {
	int resample_rate;
	int resample_channel;
	int resample_bits;
	int resample_enable;
	bt_aec_resample_t aec_resample_data_cback;
} bt_aec_resample_init_data;

typedef struct {
	int resample_rate;
	int resample_channel;
	int resample_bits;
	int resample_enable;
	hs_resample_t mozart_hs_resample_data_cback;
} hs_resample_init_data;

typedef struct {
	int sample_rate;
	int sample_channel;
	int sample_bits;
} avk_sample_init_data;

typedef void (*avk_resample_t)(avk_callback_msg *avk_msg);
typedef struct {
	int resample_rate;
	int resample_channel;
	int resample_bits;
	int resample_enable;
	avk_resample_t mozart_avk_resample_data_cback;
} avk_resample_init_data;

typedef struct bt_manager_init_info
{
	char *bt_name;
	char *bt_ble_name;
	int discoverable;
	int connectable;
	unsigned char out_bd_addr[BD_ADDR_LEN];
} bt_init_info;

typedef struct bsa_ble_create_service_data {
	int	server_num;	/* server number which is distributed */
	int	attr_num;
	UINT16	service_uuid;	/*service uuid */
	UINT16  num_handle;
	BOOLEAN	is_primary;
} ble_create_service_data;

typedef struct bsa_ble_server_indication {
	int server_num;
	int attr_num;
	int length_of_data;
	UINT8 *value;
} ble_server_indication;

typedef struct bsa_ble_start_service_data {
	int server_num;
	int attr_num;
} ble_start_service_data;

typedef struct bsa_ble_add_character_data {
	int server_num;
	int srvc_attr_num;
	int char_attr_num;
	int is_descript;
	int attribute_permission;
	int characteristic_property;
	UINT16	char_uuid;
} ble_add_char_data;

typedef struct {
	UINT16 service_uuid;	/* Enter Service UUID to read(eg. x1800) */
	UINT16 char_uuid;	/* Enter Char UUID to read(eg. x2a00) */
	UINT16 descr_id;	/* Enter Descriptor type UUID to read(eg. x2902) */
	int client_num;
	int is_descript;	/* Select descriptor? (yes=1 or no=0),default 0 */
	int ser_inst_id;	/* Enter Instance ID for Service UUID(eg. 0,1,2..), default 0 */
	int char_inst_id;	/* Enter Instance ID for Char UUID(eg. 0,1,2..) */
	int is_primary;		/* Enter Is_primary value(eg:0,1) */
} ble_client_read_data;

typedef struct {
	UINT16 service_uuid;	/* Service UUID to write */
	UINT16 char_uuid;	/* Char UUID to write */
	UINT16 descr_id;	/* Descriptor type UUID to write(eg. x2902) */
	UINT8 desc_value;	/* Descriptor value to write(eg. x01) */
	UINT8 write_data[BSA_BLE_CL_WRITE_MAX];		/* write data */
	UINT16 write_len;	/* write length: bytes */
	UINT8 write_type;	/* 1-GATT_WRITE_NO_RSP 2-GATT_WRITE */
	int client_num;
	int is_descript;	/* select descriptor? (yes=1 or no=0) */
	int ser_inst_id;	/* Instance ID for Service UUID, default 0 */
	int char_inst_id;	/* Instance ID for Char UUID(eg. 0,1,2..) */
	int is_primary;		/* Is_primary value(eg:0,1) */
} ble_client_write_data;

typedef struct {
	int device_index;	/* 0 Device from XML database (already paired), 1 Device found in last discovery */
	char *ble_name; 	/* The ble device you want to connect */
	int client_num;
	int direct;		/* Direct connection:1, Background connection:0 */
	BD_ADDR bd_addr;
} ble_client_connect_data;

typedef struct {
	int client_num;
	int service_id;
	int character_id;
	int ser_inst_id;
	int char_inst_id;
	int is_primary;
} BLE_CL_NOTIFREG;

/* callback event data for BSA_BLE_CL_NOTIF_EVT event */
typedef struct
{
	UINT16              conn_id;
	BD_ADDR             bda;
	tBTA_GATTC_CHAR_ID  char_id;
	tBTA_GATT_ID            descr_type;
	UINT16              len;
	UINT8               value[BSA_BLE_MAX_ATTR_LEN];
	BOOLEAN             is_notify;
} tBSA_BLE_CL_NOTIF_MSG;

typedef enum {
	SEC_NONE = 0,
	SEC_AUTHENTICATION = 1,
	SEC_ENCRYPTION_AUTHENTICATION = 3,
	SEC_AUTHORIZATION = 4,
	SEC_ENCRYPTION_AUTHENTICATION_AUTHORIZATION = 7
} bt_sec_type;

typedef struct {
	int device_index;
	int disc_index;
	bt_sec_type sec_type;
	tBSA_HH_PROTO_MODE mode;
} ble_hh_connect_data;


typedef UINT8 tBTM_IO_CAP;
typedef tBTM_IO_CAP     tBTA_IO_CAP;
typedef tBTA_IO_CAP tBSA_SEC_IO_CAP;

#define BD_FEATURES_LEN 8
typedef UINT8 BD_FEATURES[BD_FEATURES_LEN]; /* LMP features supported by device */

#define BT_OCTET16_LEN    16
typedef UINT8 BT_OCTET16[BT_OCTET16_LEN];   /* octet array: size 16 */

typedef UINT8 tBLE_ADDR_TYPE;
/* remote device */
typedef struct
{
	BOOLEAN in_use;
	BD_ADDR bd_addr;
	BD_NAME name;
	DEV_CLASS class_of_device;
	tBSA_SERVICE_MASK available_services;
	tBSA_SERVICE_MASK trusted_services;
	BOOLEAN is_default_hs;
	BOOLEAN stored;
	BOOLEAN link_key_present;
	LINK_KEY link_key;
	unsigned char key_type;
	tBSA_SEC_IO_CAP io_cap;
	UINT16 pid;
	UINT16 vid;
	UINT16 version;
	BD_FEATURES features;
	UINT8 lmp_version;
#if (defined(BLE_INCLUDED) && BLE_INCLUDED == TRUE)
	UINT8 ble_addr_type;
	tBT_DEVICE_TYPE device_type;
	UINT8 inq_result_type;
	BOOLEAN ble_link_key_present;
	/* KEY_PENC */
	BT_OCTET16 penc_ltk;
	BT_OCTET8 penc_rand;
	UINT16 penc_ediv;
	UINT8 penc_sec_level;
	UINT8 penc_key_size;
	/* KEY_PID */
	BT_OCTET16 pid_irk;
	tBLE_ADDR_TYPE pid_addr_type;
	BD_ADDR pid_static_addr;
	/* KEY_PCSRK */
	UINT32 pcsrk_counter;
	BT_OCTET16 pcsrk_csrk;
	UINT8 pcsrk_sec_level;
	/* KEY_LCSRK */
	UINT32 lcsrk_counter;
	UINT16 lcsrk_div;
	UINT8 lcsrk_sec_level;
	/* KEY_LENC */
	UINT16 lenc_div;
	UINT8 lenc_key_size;
	UINT8 lenc_sec_level;
#endif
} tAPP_XML_REM_DEVICE;

/*******************************************************************************
 **
 ** Function         mozart_bluetooth_init
 **
 ** Description      Initial bluetooth function.
 **
 ** Parameters	     bt_name:       bluetooth device name
 **		     discoverable:  0 is no discovery, 1 is discoverable.
 ** 		     connectable:   0 is disconnect,   1 is connectable.
 **		     out_bd_addr:   0 is use bt controler bd_addr, other is user-defined.
 **
 ** Returns          0 is ok, -1 otherwise
 **
 *******************************************************************************/
extern int mozart_bluetooth_init(bt_init_info *bt_info);

/*******************************************************************************
 **
 ** Function         mozart_bluetooth_uninit
 **
 ** Description      Uninitial bluetooth function.
 **
 ** Returns          0 is ok, -1 otherwise
 **
 *******************************************************************************/
extern int mozart_bluetooth_uninit(void);

/*******************************************************************************
 **
 ** Function         mozart_bluetooth_auto_reconnect
 **
 ** Description      Auto reconnect the last connected BT device
 **
 ** Parameters       type: USE_HS_AVK is use HFP and A2DP
 **			   USE_HS_ONLY is only use HFP
 **			   USE_AVK_ONLY is only use A2DP.
 **		     select_num: Which device you want to reconnected, 0 is the last connected device
 **
 ** Returns           0 if successful
 **		     -1 is connect failed
 **		     -2 is bt_reconnect_devices.xml not existed, no connected device
 **
 *******************************************************************************/
extern int mozart_bluetooth_auto_reconnect(int type, unsigned int select_num);

/*******************************************************************************
 **
 ** Function         mozart_bluetooth_disconnect
 **
 ** Description      Bluetooth disconnect connection with bt device
 **
 ** Parameters       type: USE_HS_AVK is use HFP and A2DP, USE_HS_ONLY is only use HFP, USE_AVK_ONLY is only use A2DP.
 **
 ** Returns          0 if successful
 **		     -1 is disconnect failed
 **
 *******************************************************************************/
extern int mozart_bluetooth_disconnect(int type);

/*******************************************************************************
 **
 ** Function        mozart_bluetooth_set_visibility
 **
 ** Description     Set the Device Visibility and connectable.
 **
 ** Parameters      discoverable: FALSE if not discoverable, TRUE is discoverable
 **                 connectable:  FALSE if not connectable, TRUE is connectable
 **
 ** Returns         0 if success, -1 otherwise
 **
 *******************************************************************************/
extern int mozart_bluetooth_set_visibility(BOOLEAN discoverable, BOOLEAN connectable);

/*******************************************************************************
 **
 ** Function         mozart_bluetooth_read_remote_device_info
 **
 ** Description      This function is used to read the XML bluetooth remote device file
 **
 ** Parameters
 **
 ** Returns          void
 **
 *******************************************************************************/
extern tAPP_XML_REM_DEVICE *mozart_bluetooth_read_remote_device_info();

/*******************************************************************************
 **
 ** Function         mozart_bluetooth_set_link_status
 **
 ** Description      Bluetooth set current link status
 **
 ** Parameters       void
 **
 ** Returns          0: success, -1: FALSE
 **
 *******************************************************************************/
extern int mozart_bluetooth_set_link_status(bsa_link_status bt_status);

/*******************************************************************************
 **
 ** Function         mozart_bluetooth_get_link_status
 **
 ** Description      Bluetooth get current link status
 **
 ** Parameters       void
 **
 ** Returns          bsa_link_status
 **
 *******************************************************************************/
extern bsa_link_status mozart_bluetooth_get_link_status(void);

/*******************************************************************************
 **
 ** Function         mozart_bluetooth_get_link_down_status
 **
 ** Description      get link down status
 **
 **
 ** Parameter        None
 **
 ** Return Value:    Reason code 19: Mobile terminal to close Bluetooth
 ** 		     Reason code 8: connection timeout
 **		     other Reason code, please see Bluetooth Core spec
 *******************************************************************************/
extern UINT8 mozart_bluetooth_get_link_down_status();

/*******************************************************************************
 **
 ** Function        mozart_bluetooth_get_cod_string
 **
 ** Description     This function is used to get readable string from Class of device
 **
 ** Parameters      class_of_device: The Class of device to decode
 **
 ** Returns         Pointer on string containing device type
 **
 *******************************************************************************/
extern char *mozart_bluetooth_get_cod_string(const DEV_CLASS class_of_device);

/*******************************************************************************
 **
 ** Function         mozart_bluetooth_parse_eir_manuf_specific
 **
 ** Description      This function is used to parse EIR manufacturer data
 **
 ** Returns          0 if successful, Otherwise failed
 **
 *******************************************************************************/
extern int mozart_bluetooth_parse_eir_manuf_specific(UINT8 *p_eir, bsa_manu_data *manu_data);



/******************************* Discovery Interface **************************************************/

/*******************************************************************************
 **
 ** Function         mozart_bluetooth_disc_start_regular
 **
 ** Description      Start Device discovery
 **
 ** Returns          0 is ok, other is failed
 **
 *******************************************************************************/
extern int mozart_bluetooth_disc_start_regular(void);

/*******************************************************************************
 **
 ** Function         mozart_bluetooth_disc_stop_regular
 **
 ** Description      Abort Device discovery
 **
 ** Returns          status: 0 if success / -1 otherwise
 **
 *******************************************************************************/
extern int mozart_bluetooth_disc_stop_regular(void);

/*******************************************************************************
 **
 ** Function        mozart_bluetooth_disc_get_device_count
 **
 ** Description     Get bluetooth search bt devices count
 **
 ** Parameters      void
 **
 ** Returns         > 0 : bt devices number, == 0 : no bt devices,
 **
 *******************************************************************************/
extern unsigned int mozart_bluetooth_disc_get_device_count(void);

/******************************************************************************
 **
 ** Function         mozart_bluetooth_disc_get_device_info
 **
 ** Description      get all discovery bt device info
 **
 ** Parameter        NULL
 **
 ** Returns          tAPP_DISCOVERY_CB addr
 *******************************************************************************/
extern tAPP_DISCOVERY_CB *mozart_bluetooth_disc_get_device_info();

/*******************************************************************************
 **
 ** Function         mozart_bluetooth_disc_find_device_name
 **
 ** Description      Find a device in the list of discovered devices
 **
 ** Parameters       index_device
 **
 ** Returns          index_device if successful, -1 is failed.
 **
 *******************************************************************************/
extern int mozart_bluetooth_disc_find_device_name(char *name);



/******************************  HS Servie Interface  *********************************/

/*******************************************************************************
 **
 ** Function         mozart_bluetooth_hs_start_service
 **
 ** Description      Start bluetooth HS service
 **
 ** Parameters	     void
 **
 ** Returns          0 is ok, -1 otherwise
 **
 *******************************************************************************/
extern int mozart_bluetooth_hs_start_service(void);

/*******************************************************************************
 **
 ** Function         mozart_bluetooth_hs_stop_service
 **
 ** Description      Stop bluetooth HS service
 **
 ** Parameters	     void
 **
 ** Returns          0 is ok, -1 otherwise
 **
 *******************************************************************************/
extern int mozart_bluetooth_hs_stop_service(void);

/*******************************************************************************
 **
 ** Function         mozart_bluetooth_hs_open
 **
 ** Description      Establishes mono headset connections
 **
 ** Parameter        BD address to connect to. If its NULL, the app will prompt user for device.
 **
 ** Returns          0 if success -1 if failure
 *******************************************************************************/
extern int mozart_bluetooth_hs_open(BSA_OPEN_TYPE choice, unsigned int select_num);

/*******************************************************************************
 **
 ** Function         mozart_bluetooth_hs_close
 **
 ** Description      release mono headset connections
 **
 ** Returns          0 if success -1 if failure
 *******************************************************************************/
extern int mozart_bluetooth_hs_close();

/*******************************************************************************
 **
 ** Function         mozart_bluetooth_hs_audio_open
 **
 ** Description      Open the SCO connection alone
 ** 		     switch the phone call between the phone to the units
 **
 ** Parameter        None
 **
 ** Returns          0 if success, -1 if failure
 *******************************************************************************/
extern int mozart_bluetooth_hs_audio_open();

/*******************************************************************************
 **
 ** Function         mozart_bluetooth_hs_audio_close
 **
 ** Description      Close the SCO connection alone
 ** 		     switch the phone call between the units to the phone
 **
 ** Parameter        None
 **
 ** Returns          0 if success, -1 if failure
 *******************************************************************************/
extern int mozart_bluetooth_hs_audio_close();

/*******************************************************************************
 **
 ** Function         mozart_bluetooth_hs_get_call_state
 **
 ** Description      Access to bluetooth calls state
 **
 ** Parameters	     void
 **
 ** Returns          see BSA_CALL_STATE
 **
 *******************************************************************************/
extern BSA_CALL_STATE mozart_bluetooth_hs_get_call_state(void);

/*******************************************************************************
 **
 ** Function         mozart_bluetooth_hs_answer_call
 **
 ** Description      Bluetooth answer the call
 **
 ** Parameters	     void
 **
 ** Returns          0 if OK, -1 otherwise.
 **
 *******************************************************************************/
extern int mozart_bluetooth_hs_answer_call(void);

/*******************************************************************************
 **
 ** Function         mozart_bluetooth_hs_hangup_call
 **
 ** Description      Bluetooth hang up the call
 **
 ** Parameters	     void
 **
 ** Returns          0 if OK, -1 otherwise.
 **
 *******************************************************************************/
extern int mozart_bluetooth_hs_hangup_call(void);

/*******************************************************************************
 **
 ** Function         mozart_bluetooth_hs_hold_call
 **
 ** Description      Hold active call
 **
 ** Parameters       void
 **
 ** Returns          0 if successful execution, error code else
 **
 *******************************************************************************/
extern int mozart_bluetooth_hs_hold_call(tBSA_BTHF_CHLD_TYPE_T type);

/*******************************************************************************
 **
 ** Function         mozart_blutooth_hs_set_volume
 **
 ** Description      Send volume AT Command
 **
 ** Parameters       type: speaker or microphone, volume: speaker or microphone volume value
 **
 ** Returns          0 if successful execution, error code else
 **
 *******************************************************************************/
extern int mozart_blutooth_hs_set_volume(tBSA_BTHF_VOLUME_TYPE_T type, int volume);

/*******************************************************************************
 **
 ** Function         mozart_hs_get_default_sampledata
 **
 ** Description      mozart get hs default sample data
 **
 ** Parameters       struct hs_sample_init_data
 **
 ** Returns          void
 **
 *******************************************************************************/
extern void mozart_hs_get_default_sampledata(hs_sample_init_data *sample_data);

/*******************************************************************************
 **
 ** Function         mozart_bluetooth_hs_set_resampledata_callback
 **
 ** Description      mozart set hs resample_data to bsa, bsa set resample_data to codec controler.
 **
 ** Parameters       struct hs_resample_init_data
 **
 ** Returns          void
 **
 *******************************************************************************/
extern void mozart_bluetooth_hs_set_resampledata_callback(hs_resample_init_data *resample_data);

/*******************************************************************************
 **
 ** Function         mozart_bluetooth_hs_send_battery_level
 **
 ** Description      Send bt battery_value to phone device
 **
 ** Parameters       unsigned int battery_value: 0~9
 **		     0 is the minimum value, 9 is the maximum value
 **
 ** Returns          0 if successful execution, error code else
 **
 *******************************************************************************/
extern int mozart_bluetooth_hs_send_battery_level(unsigned int battery_value);



/******************************** AVK Servie Interface ***********************************/

/*******************************************************************************
 **
 ** Function         mozart_bluetooth_avk_start_service
 **
 ** Description      Start bluetooth A2DP service
 **
 ** Parameters	     void
 **
 ** Returns          0 if OK, -1 otherwise.
 **
 *******************************************************************************/
extern int mozart_bluetooth_avk_start_service(void);

/*******************************************************************************
 **
 ** Function         mozart_bluetooth_avk_stop_service
 **
 ** Description      Stop bluetooth A2DP service
 **
 ** Parameters	     void
 **
 ** Returns          0 if OK, -1 otherwise.
 **
 *******************************************************************************/
extern int mozart_bluetooth_avk_stop_service(void);

/*******************************************************************************
 **
 ** Function         mozart_bluetooth_avk_start_play
 **
 ** Description      Bluetooth start and resume play
 **
 ** Parameters	     void
 **
 ** Returns          void
 **
 *******************************************************************************/
extern void mozart_bluetooth_avk_start_play(void);

/*******************************************************************************
 **
 ** Function         mozart_bluetooth_avk_stop_play
 **
 ** Description      Bluetooth stop the current play
 **
 ** Parameters	     void
 **
 ** Returns          void
 **
 *******************************************************************************/
extern void mozart_bluetooth_avk_stop_play(void);

/*******************************************************************************
 **
 ** Function         mozart_bluetooth_avk_pause_play
 **
 ** Description      Bluetooth pause the current play
 **
 ** Parameters	     void
 **
 ** Returns          void
 **
 *******************************************************************************/
extern void mozart_bluetooth_avk_pause_play(void);

/*******************************************************************************
 **
 ** Function         mozart_bluetooth_avk_play_pause
 **
 ** Description      Bluetooth execute play and pause command switch Automatically
 **
 ** Parameters	     void
 **
 ** Returns          void
 **
 *******************************************************************************/
extern void mozart_bluetooth_avk_play_pause(void);

/*******************************************************************************
 **
 ** Function         mozart_bluetooth_avk_next_music
 **
 ** Description      Bluetooth play the next music
 **
 ** Parameters	     void
 ** Returns          void
 **
 *******************************************************************************/
extern void mozart_bluetooth_avk_next_music(void);

/*******************************************************************************
 **
 ** Function         mozart_bluetooth_avk_prev_music
 **
 ** Description      Bluetooth play the previous music
 **
 ** Parameters	     void
 ** Returns          void
 **
 *******************************************************************************/
extern void mozart_bluetooth_avk_prev_music(void);

/*******************************************************************************
 **
 ** Function        mozart_bluetooth_avk_get_play_state
 **
 ** Description     get avk plat status
 **
 ** Parameters      void
 **
 ** Returns          TRUE: play   FALSE: stop
 **
 *******************************************************************************/
extern int mozart_bluetooth_avk_get_play_state(void);

/*******************************************************************************
 **
 ** Function         mozart_bluetooth_avk_set_volume_up
 **
 ** Description      This function sends absolute volume up
 **
 ** Paraneters       volume: 0~0x7F
 **
 ** Returns          None
 **
 *******************************************************************************/
extern void mozart_bluetooth_avk_set_volume_up(UINT8 volume);

/*******************************************************************************
 **
 ** Function         mozart_bluetooth_avk_set_volume_down
 **
 ** Description      This function sends absolute volume down
 **
 ** Parameters       volume: 0~0x7F
 **
 ** Returns          None
 **
 *******************************************************************************/
extern void mozart_bluetooth_avk_set_volume_down(UINT8 volume);

/*******************************************************************************
 **
 ** Function         mozart_bluetooth_app_avk_open
 **
 ** Description      Example of function to open AVK connection
 **
 ** Parameters       choice: default BSA_APP_RECONNECT,
 **		     select_num: 0 is the last connected Device
 ** Returns          0 is success, other is failed
 **
 *******************************************************************************/
extern int mozart_bluetooth_avk_open(BSA_OPEN_TYPE choice, unsigned int select_num);

/*******************************************************************************
 **
 ** Function         mozart_bluetooth_avk_close
 **
 ** Description      Function to close AVK connection
 **
 ** Returns          0 is success, otherwise is failed
 **
 *******************************************************************************/
extern int mozart_bluetooth_avk_close();

/*******************************************************************************
 **
 ** Function         mozart_bluetooth_avk_set_resampledata_callback
 **
 ** Description      Set AVK resample_data cback
 **
 ** Parameters       avk_resample_init_data
 **
 ** Returns          void
 **
 *******************************************************************************/
extern void mozart_bluetooth_avk_set_resampledata_callback(avk_resample_init_data *resample_data);



/********************************** AEC Interface ****************************************/

/*******************************************************************************
 **
 ** Function         mozart_aec_callback
 **
 ** Description      Bluetooth eliminate echo callback
 **
 ** Parameters       bt_aec_callback struct
 **
 ** Returns          void
 **
 *******************************************************************************/
extern void mozart_aec_callback(bt_aec_callback *bt_ac);

/*******************************************************************************
 **
 ** Function         mozart_aec_get_bt_default_sampledata
 **
 ** Description      AEC get Bluetooth default sample_data
 **
 ** Parameters       bt_aec_sample_init_data
 **
 ** Returns          void
 **
 *******************************************************************************/
extern void mozart_aec_get_bt_default_sampledata(bt_aec_sample_init_data *sample_data);

/*******************************************************************************
 **
 ** Function         mozart_aec_set_bt_resampledata_callback
 **
 ** Description      Bluetooth eliminate echo callback
 **
 ** Parameters       bt_aec_resample_init_data
 **
 ** Returns          void
 **
 *******************************************************************************/
extern void mozart_aec_set_bt_resampledata_callback(bt_aec_resample_init_data *resample_data);


/********************************** OPP Interface ****************************************/

/*******************************************************************************
 **
 ** Function         mozart_bluetooth_ops_start
 **
 ** Description      start OPP Server application
 **
 ** Parameters       void
 **
 ** Returns          BSA_SUCCESS success, error code for failure
 **
 *******************************************************************************/
extern tBSA_STATUS mozart_bluetooth_ops_start(void);

/*******************************************************************************
 **
 ** Function         mozart_bluetooth_ops_stop
 **
 ** Description      stop OPP Server application
 **
 ** Parameters       void
 **
 ** Returns          BSA_SUCCESS success, error code for failure
 **
 *******************************************************************************/
extern tBSA_STATUS mozart_bluetooth_ops_stop(void);

/*******************************************************************************
 **
 ** Function         mozart_bluetooth_ops_auto_accept
 **
 ** Description      set ops auto accept object
 **
 ** Parameters       void
 **
 ** Returns          BSA_SUCCESS success, error code for failure
 **
 *******************************************************************************/
extern tBSA_STATUS mozart_bluetooth_ops_auto_accept(void);

/*******************************************************************************
 **
 ** function         mozart_bluetooth_opc_start
 **
 ** description      start opp client application
 **
 ** parameters       void
 **
 ** returns          bsa_success success, error code for failure
 **
 *******************************************************************************/
extern tBSA_STATUS mozart_bluetooth_opc_start(void);

/*******************************************************************************
 **
 ** Function         mozart_bluetooth_opc_stop
 **
 ** Description      stop OPP Client application
 **
 ** Parameters       void
 **
 ** Returns          BSA_SUCCESS success, error code for failure
 **
 *******************************************************************************/
extern tBSA_STATUS mozart_bluetooth_opc_stop(void);

/*******************************************************************************
 **
 ** Function         mozart_bluetooth_opc_set_key
 **
 ** Description      Select key to set opc Function
 **
 ** Parameters       APP_OPC_KEY_XXX
 **
 ** Returns          BSA_SUCCESS success, error code for failure
 **
 *******************************************************************************/
extern tBSA_STATUS mozart_bluetooth_opc_set_key(int choice);



/********************************* BLE Interface **************************************/

/*******************************************************************************
 **
 ** Function        mozart_bluetooth_ble_start
 **
 ** Description     start BSA BLE Function
 **
 ** Parameters      None
 **
 ** Returns         status: 0 if success / -1 otherwise
 **
 *******************************************************************************/
extern int mozart_bluetooth_ble_start();

/*******************************************************************************
 **
 ** Function        mozart_bluetooth_ble_close
 **
 ** Description     Close BSA BLE Function
 **
 ** Parameters      None
 **
 ** Returns         status: 0 if success / -1 otherwise
 **
 *******************************************************************************/
extern int mozart_bluetooth_ble_close();

/*******************************************************************************
 **
 ** Function        mozart_bluetooth_ble_set_visibility
 **
 ** Description     Set the Device BLE Visibility parameters
 **
 ** Parameters      discoverable: FALSE if not discoverable
 **                 connectable: FALSE if not connectable
 **
 ** Returns         status: 0 if success / -1 otherwise
 **
 *******************************************************************************/
extern int mozart_bluetooth_ble_set_visibility(BOOLEAN discoverable, BOOLEAN connectable);

/*******************************************************************************
 **
 ** Function         mozart_bluetooth_ble_start_regular
 **
 ** Description      Start BLE Device discovery
 **
 ** Returns          0 if success, -1 if failed
 **
 *******************************************************************************/
extern int mozart_bluetooth_ble_start_regular();

/*******************************************************************************
 **
 ** Function        mozart_bluetooth_ble_configure_advertisement_data
 **
 ** Description     start BLE advertising
 **
 ** Parameters      struct tBSA_DM_BLE_ADV_CONFIG
 **
 ** Returns         status: 0 if success / -1 otherwise
 **
 *******************************************************************************/
extern int mozart_bluetooth_ble_configure_adv_data(tBSA_DM_BLE_ADV_CONFIG *adv_data);

/*
 * BLE Server functions
 */
/*******************************************************************************
 **
 ** Function        mozart_bluetooth_ble_server_register
 **
 ** Description     Register server app
 **
 ** Parameters      uuid
 **
 ** Returns         status: server_num if success / -1 otherwise
 **
 *******************************************************************************/
extern int mozart_bluetooth_ble_server_register(UINT16 uuid);

/*******************************************************************************
 **
 ** Function        mozart_bluetooth_ble_server_deregister
 **
 ** Description     Deregister server app
 **
 ** Parameters      server_num: server number
 **
 ** Returns         status: 0 if success / -1 otherwise
 **
 *******************************************************************************/
extern int mozart_bluetooth_ble_server_deregister(int server_num);

/*******************************************************************************
 **
 ** Function        mozart_bluetooth_ble_server_create_service
 **
 ** Description     create GATT service
 **
 ** Parameters      struct ble_service_info
 **
 ** Returns         status: 0 if success / -1 otherwise
 **
 *******************************************************************************/
extern int mozart_bluetooth_ble_server_create_service(ble_create_service_data *ble_create_service_data);

/*******************************************************************************
 **
 ** Function       mozart_bluetooth_ble_server_start_service
 **
 ** Description     Start Service
 **
 ** Parameters      None
 **
 ** Returns         status: 0 if success / -1 otherwise
 **
 *******************************************************************************/
extern int mozart_bluetooth_ble_server_start_service(ble_start_service_data *ble_start_service_data);

/*******************************************************************************
 **
 ** Function        mozart_bluetooth_ble_add_character
 **
 ** Description     Add character to service
 **
 ** Parameters      struct bsa_ble_add_character
 **
 ** Returns         status: 0 if success / -1 otherwise
 **
 *******************************************************************************/
extern int mozart_bluetooth_ble_server_add_character(ble_add_char_data *ble_add_char_data);

/*******************************************************************************
 **
 ** Function         mozart_bluetooth_ble_server_display
 **
 ** Description      display BLE server
 **
 ** Parameters
 **
 ** Returns          void
 **
 *******************************************************************************/
extern void mozart_bluetooth_ble_server_display();

/*******************************************************************************
 **
 ** Function        mozart_bluetooth_ble_server_send_indication
 **
 ** Description     Send indication to client
 **
 ** Parameters      None
 **
 ** Returns         status: 0 if success / -1 otherwise
 **
 *******************************************************************************/
extern int mozart_bluetooth_ble_server_send_indication(ble_server_indication *ble_indication);

/*******************************************************************************
 **
 ** Function         mozart_bluetooth_server_set_char_value
 **
 ** Description      Set BLE character value
 **
 ** Parameter        server_num: service num. character_num: character num. value: character value.
 **
 ** Returns          0 if service enabled, -1 if service not enabled
 *******************************************************************************/
extern int mozart_bluetooth_server_set_char_value(int server_num, int character_num, UINT8 *value, int vaule_num);

/*******************************************************************************
 **
 ** Function         mozart_bluetooth_server_get_char_value
 **
 ** Description      Get BLE character value
 **
 ** Parameter        server_num: service num. character_num: character num. value: character value.
 **
 ** Returns          character value length if service enabled, -1 if service not enabled
 *******************************************************************************/
extern int mozart_bluetooth_server_get_char_value(int server_num, int character_num, UINT8 *value);


/* ---------------- ble client ------------------------------- */

/*******************************************************************************
 **
 ** Function        mozart_bluetooth_ble_client_register
 **
 ** Description     This is the ble client register command
 **
 ** Parameters      UINT16 uuid
 **
 ** Returns         status: client_num if success / -1 otherwise
 **
 *******************************************************************************/
extern int mozart_bluetooth_ble_client_register(UINT16 uuid);

/*******************************************************************************
 **
 ** Function        mozart_bluetooth_ble_client_deregister
 **
 ** Description     This is the ble deregister app
 **
 ** Parameters      int client_num
 **
 ** Returns         status: 0 if success / -1 otherwise
 **
 *******************************************************************************/
extern int mozart_bluetooth_ble_client_deregister(int client_num);


/*******************************************************************************
 **
 ** Function        mozart_bluetooth_ble_client_connect_server
 **
 ** Description     This is the ble open connection to ble server
 **
 ** Parameters      struct ble_client_connect_data
 **
 ** Returns         status: 0 if success / -1 otherwise
 **
 *******************************************************************************/
extern int mozart_bluetooth_ble_client_connect_server(ble_client_connect_data *cl_connect_data);

/*******************************************************************************
 **
 ** Function        mozart_bluetooth_ble_client_disconnect_server
 **
 ** Description     This is the ble close connection
 **
 ** Parameters      client_num
 **
 ** Returns         status: 0 if success / -1 otherwise
 **
 *******************************************************************************/
extern int mozart_bluetooth_ble_client_disconnect_server(int client_num);

/*******************************************************************************
 **
 ** Function        mozart_bluetooth_ble_client_read
 **
 ** Description     This is the read function to read a characteristec or characteristic descriptor from BLE server
 **
 ** Parameters      struct ble_client_read_data
 **
 ** Returns         status: 0 if success / -1 otherwise
 **
 *******************************************************************************/
extern int mozart_bluetooth_ble_client_read(ble_client_read_data *client_data);

/*******************************************************************************
 **
 ** Function        mozart_bluetooth_ble_client_write
 **
 ** Description     This is the write function to write a characteristic or characteristic discriptor to BLE server.
 **
 ** Parameters      ble_client_write_data
 **
 ** Returns         status: 0 if success / -1 otherwise
 **
 *******************************************************************************/
extern int mozart_bluetooth_ble_client_write(ble_client_write_data *cl_write_data);

/*******************************************************************************
 **
 ** Function        app_ble_client_register_notification
 **
 ** Description     This is the register function to receive a notification
 **
 ** Parameters      None
 **
 ** Returns         status: 0 if success / -1 otherwise
 **
 *******************************************************************************/
extern int mozart_bluetooth_ble_client_register_notification(BLE_CL_NOTIFREG *cl_notireg);

/*******************************************************************************
 **
 ** Function        mozart_bluetooth_ble_clinet_get_notification_data
 **
 ** Description     This is the register function to receive a notification
 **
 ** Parameters      None
 **
 ** Returns         status: 0 if success / -1 otherwise
 **
 *******************************************************************************/
extern tBSA_BLE_CL_NOTIF_MSG *mozart_bluetooth_ble_clinet_get_notification_data();

/********************************** BLE HID Interface *******************************************/

/*******************************************************************************
 **
 ** Function         mozart_bluetooth_hh_start
 **
 ** Description      Start HID Host
 **
 ** Parameters	     void
 **
 ** Returns          0 if successful, error code otherwise
 **
 *******************************************************************************/
extern int mozart_bluetooth_hh_start(void);

/*******************************************************************************
 **
 ** Function         mozart_bluetooth_hh_stop
 **
 ** Description      Stop HID Host
 **
 ** Parameters       void
 **
 ** Returns          0 if successful, error code otherwise
 **
 *******************************************************************************/
extern int mozart_bluetooth_hh_stop(void);

/*******************************************************************************
 **
 ** Function         mozart_bluetooth_hh_connect
 **
 ** Description      Example of function to connect to HID device
 **
 ** Parameters	     ble_hh_connect_data
 **
 ** Returns          0 is successed , other is failed
 **
 *******************************************************************************/
extern int mozart_bluetooth_hh_connect(ble_hh_connect_data *connect_data);

/*******************************************************************************
 **
 ** Function         mozart_bluetooth_avk_open_dsp
 **
 ** Description      Function to open AVK dsp
 **
 ** Parameters       msec: Fade in time(msec), 0 is not use this function
 **
 ** Returns          0 is success, otherwise is failed
 **
 *******************************************************************************/
extern int mozart_bluetooth_avk_open_dsp(unsigned int msec);

/*******************************************************************************
 **
 ** Function         mozart_bluetooth_avk_close_dsp
 **
 ** Description      Function to close AVK dsp
 **
 ** Parameters       msec: Fade out time(msec), 0 is not use this function
 **
 ** Returns          0 is success, otherwise is failed
 **
 *******************************************************************************/
extern int mozart_bluetooth_avk_close_dsp(unsigned int msec);

/*******************************************************************************
 **
 ** Function         mozart_bluetooth_avk_send_get_element_att_cmd
 **
 ** Description      send BSA_AVK_RC_VD_GET_ELEMENT_ATTR cmd
 **
 ** Returns          0 if success, other is failed
 **
 *******************************************************************************/
extern int mozart_bluetooth_avk_send_get_element_att_cmd();

/*******************************************************************************
 **
 ** Function         mozart_bluetooth_avk_get_element_att
 **
 ** Description      get current set values on the target
 ** 		     for the provided player application setting attributes list
 **
 ** Returns          struct tBSA_AVK_GET_ELEMENT_ATTR_MSG
 **
 *******************************************************************************/
extern tBSA_AVK_GET_ELEMENT_ATTR_MSG *mozart_bluetooth_avk_get_element_att();

/*******************************************************************************
 **
 ** Function	     mozart_bluetooth_avk_close_arvcp_function
 **
 ** Description      This function close avrcp Features
 **
 ** Returns          None
 **
 *******************************************************************************/
extern void mozart_bluetooth_avk_close_arvcp_function();

/*******************************************************************************
 **
 ** Function         mozart_bluetooth_avk_send_get_play_status_command
 **
 ** Description      send get_play_status cmd
 **
 ** Returns          void
 **
 *******************************************************************************/
extern void mozart_bluetooth_avk_send_get_play_status_command();

/*******************************************************************************
 **
 ** Function         mozart_bluetooth_avk_get_play_status
 **
 ** Description      get current play status
 **
 ** Returns	     tBSA_AVK_GET_PLAY_STATUS_MSG
 **
 *******************************************************************************/
extern tBSA_AVK_GET_PLAY_STATUS_MSG *mozart_bluetooth_avk_get_play_status();

#endif
