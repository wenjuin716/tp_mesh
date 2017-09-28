//#include <asm/types.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <linux/netlink.h>
#include "ieee80211_rpwahd_api.h"

int tp_mesh_netlink_init (void);
void tp_mesh_netlink_destory(int fd);
void tp_mesh_event_handler(int fd);
size_t mesh_cmd_payload_len(ATH_RPWAHD_CMD type);
void tp_mesh_send_cmd(int fd, ATH_RPWAHD_CMD type,void *data);
