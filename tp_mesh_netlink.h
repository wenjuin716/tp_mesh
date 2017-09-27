//#include <asm/types.h>
#include <stdio.h>
#include <string.h>

#include <sys/socket.h>
#include <linux/netlink.h>
#include "ieee80211_rpwahd_api.h"

//#define NETLINK_RPWAHD_EVENT 22

int tp_mesh_netlink_init (void);
void tp_mesh_event_handler(int fd);
