/*
 * @@-COPYRIGHT-START-@@
 *
 * Copyright (c) 2017 TP-Link
 * All Rights Reserved.
 *
 * @@-COPYRIGHT-END-@@
 */

#ifndef _ATH_RPWAHD_API__
#define _ATH_RPWAHD_API__
//#include <ieee80211_rrm.h>
#define NETLINK_RPWAHD_EVENT 22
#define RPWAHD_INVALID_RSSI 0
#define IEEE80211_ADDR_LEN 6
#define RPWAHD_MAX_PEERS_PER_EVENT 3

/**
 * Metadata about a probe request received from a client that is useful
 * for making steering decisions.
 */
struct rpwahd_probe_req_ind {
    /* The MAC address of the client that sent the probe request.*/
    u_int8_t sender_addr[IEEE80211_ADDR_LEN];
    /*  The RSSI of the received probe request.*/
    u_int8_t rssi;
};

/**
 * Data rated related information contained in ATH_EVENT_BSTEERING_NODE_ASSOCIATED
 */
typedef struct ieee80211_rpwahd_datarate_info_t {
    /* Maximum bandwidth the client supports, valid values are enumerated
     * in enum ieee80211_cwm_width in _ieee80211.h. But the header file cannot
     * be included here because of potential circular dependency. Caller should
     * make sure that only valid values can be written/read. */
    u_int8_t max_chwidth;
    /* Number of spatial streams the client supports */
    u_int8_t num_streams;
    /* PHY mode the client supports. Same as max_chwidth field, only valid values
     * enumerated in enum ieee80211_phymode can be used here. */
    u_int8_t phymode;
    /* Maximum MCS the client supports */
    u_int8_t max_MCS;
    /* Maximum TX power the client supports */
    u_int8_t max_txpower;
    /* Set to 1 if this client is operating in Static SM Power Save mode */
    u_int8_t is_static_smps : 1;
    /* Set to 1 if this client supports MU-MIMO */
    u_int8_t is_mu_mimo_supported : 1;
} ieee80211_rpwahd_datarate_info_t;

/**
 * Metadata about a STA that has associated
 */
struct rpwahd_node_associated_ind {
    /* The MAC address of the client that is associated.*/
    u_int8_t client_addr[IEEE80211_ADDR_LEN];
    /* Set to 1 if this client supports BSS Transition Management */
    u_int8_t isBTMSupported : 1;
    /* Set to 1 if this client implements Radio Resource Manangement */
    u_int8_t isRRMSupported : 1;
    /* Band capability that tell if the client is 2.4G or 5G or both */
    u_int8_t band_cap : 2;
    /* Data rate related information supported by this client */
    ieee80211_rpwahd_datarate_info_t datarate_info;
};

/**
 * Metadata about a STA that has disassociated
 */
struct rpwahd_node_disassociated_ind {
    /* The MAC address of the client that is associated.*/
    u_int8_t client_addr[IEEE80211_ADDR_LEN];
    /*Disassociation reason code*/
    u_int8_t reason;
};

/**
 * Metadata about interface status
 */
struct rpwahd_intf_status_ind {
    /* The MAC address of the client that is associated.*/
    ieee80211_rpwahd_datarate_info_t datarate_info;
    u_int8_t ch_load;
};

/**
 * STA stats per peer
 */
struct rpwahd_sta_stats_per_peer {
    /* The MAC address of the client */
    u_int8_t client_addr[IEEE80211_ADDR_LEN];
    /* Uplink RSSI */
    u_int8_t rssi;
    /* PER */
    u_int8_t per;
    /* The Tx byte count */
    u_int64_t tx_byte_count;
    /* The Rx byte count */
    u_int64_t rx_byte_count;
    /* The Tx packet count */
    u_int32_t tx_packet_count;
    /* The Rx packet count */
    u_int32_t rx_packet_count;
    /* The last Tx rate (in Kbps) */
    u_int32_t tx_rate;
};

/**
 * Metadata for STA stats
 */
struct rpwahd_sta_stats_ind {
    /* Number of peers for which stats are provided */
    u_int8_t sta_addr[IEEE80211_ADDR_LEN];
    /* Stats per peer */
    struct rpwahd_sta_stats_per_peer sta_stats;
};

/**
 * Metadata for non associated STA stats
 */
struct rpwahd_nonassoc_sta_stats_ind {
    /* The MAC address of the client that sent the probe request.*/
    u_int8_t sta_addr[IEEE80211_ADDR_LEN];
    /*  The RSSI of the received probe request.*/
    u_int8_t rssi;
};

typedef struct ath_rpwahd_event {
    /* The type of the event: One of ATH_BSTEERING_EVENT.*/
    u_int32_t type;
    /* The OS-specific index of the VAP on which the event occurred.*/
    u_int32_t sys_index;
    /* The data for the event. Which member is valid is based on the
       type field.*/
   union {
        struct rpwahd_probe_req_ind rpwahd_probe;
        struct rpwahd_node_associated_ind rpwahd_node_associated;
        struct rpwahd_node_disassociated_ind rpwahd_node_disassociated;
        struct rpwahd_intf_status_ind rpwahd_intf_stats;
        struct rpwahd_sta_stats_ind rpwahd_sta_stats;
        struct rpwahd_nonassoc_sta_stats_ind rpwahd_nonassoc_sta_stats;
    } data;
} ath_rpwahd_event_t;

/**
 * command for deauth STA 
 */
struct rpwahd_deauth_ind {
    /* The MAC address of the client that sent the probe request.*/
    u_int8_t sta_addr[IEEE80211_ADDR_LEN];
};

/**
 * command for set blacklist for STA 
 */
struct rpwahd_blacklist_ind {
    /* The MAC address of the client that sent the probe request.*/
    u_int8_t sta_addr[IEEE80211_ADDR_LEN];
};

/**
 * command for steer STA using 11v
 */
struct rpwahd_11v_steer_ind {
    /* The MAC address of the client that sent the probe request.*/
    u_int8_t sta_addr[IEEE80211_ADDR_LEN];
    u_int8_t bssid[IEEE80211_ADDR_LEN];
};


/**
 * get current state of STA
 */
struct rpwahd_assoc_sta_ind {
    /* The MAC address of the client that sent the probe request.*/
    u_int8_t sta_addr[IEEE80211_ADDR_LEN];
};

/**
 * get current state of non assoc STA
 */
struct rpwahd_nonassoc_sta_ind {
    /* The MAC address of the client that sent the probe request.*/
    u_int8_t sta_addr[IEEE80211_ADDR_LEN];
};

typedef struct ath_rpwahd_cmd {
    /* The type of the event: One of ATH_BSTEERING_EVENT.*/
    u_int32_t type;
    /* The OS-specific index of the VAP on which the event occurred.*/
    u_int32_t sys_index;
    /* The data for the event. Which member is valid is based on the
       type field.*/
   union {
        struct rpwahd_deauth_ind rpwahd_deauth;
        struct rpwahd_blacklist_ind rpwahd_blacklist;
        struct rpwahd_11v_steer_ind rpwahd_11v_teer;
        struct rpwahd_assoc_sta_ind rpwahd_assoc_sta;
        struct rpwahd_nonassoc_sta_ind rpwahd_nonassoc_sta;
//        struct rpwahd_nonassoc_sta_stats_ind rpwahd_nonassoc_sta_stats;
    } data;
} ath_rpwahd_cmd_t;

/**
 * Event types that are asynchronously generated by the band steering
 * module.
 */
typedef enum {
    ATH_EVENT_RPWAHD_PROBE_REQ_EVENT = 1,
    ATH_EVENT_RPWAHD_STA_ASSOCIATED_EVENT = 2,
    ATH_EVENT_RPWAHD_DEAUTH_EVENT = 3,
    ATH_EVENT_RPWAHD_INTF_INFO_EVENT = 4,
    ATH_EVENT_RPWAHD_INTF_STATUS_EVENT = 5,
    ATH_EVENT_RPWAHD_STA_STATUS_EVENT = 6,
    ATH_EVENT_RPWAHD_NONASSOC_STA_STATUS_EVENT = 7,
} ATH_RPWAHD_EVENT;

#endif /* _ATH_RPWAHD_API__ */
