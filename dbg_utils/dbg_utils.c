#include <sys/socket.h>
#include <linux/netlink.h>
#include <stdio.h>
#include <stdlib.h>
#include "ieee80211_rpwahd_api.h"	// netlink message detail define

void usage(void){
	printf("==========================\n");
	printf("dbg_utils <cmd type> <payload>\n");
	printf("\tcmd type:\n");
        printf("\t1:\tATH_CMD_RPWAHD_DEAUTH_CMD\n");
        printf("\t2:\tATH_CMD_RPWAHD_BLACK_LIST_CMD\n");
        printf("\t3:\tATH_CMD_RPWAHD_11V_STEER_CMD\n");
        printf("\t4:\tATH_CMD_RPWAHD_STA_ASSOC_STATUS_CMD\n");
        printf("\t5:\tATH_CMD_RPWAHD_INTF_STATUS_CMD\n");
        printf("\t6:\tATH_CMD_RPWAHD_INTF_INFO_CMD\n");
        printf("\t100:\tATH_CMD_RPWAHD_DEBUG_CMD\n");
        printf("==========================\n");
}

void main(int argc, char** argv)
{
	int mesh_sock = -1;
	char data[100]={0};

	if(argc == 1 || argc != 3 ){
		usage();
		return;
	}

#if 1
        /* create tp-link mesh netlink socket */
        if ((mesh_sock = tp_mesh_netlink_init()) < 0)
        {
                fprintf(stderr, "Failed to create TP-Link Mesh netlink socket\n");
                return -1;
        }
#endif

	switch(atoi(argv[1])){
	case ATH_CMD_RPWAHD_DEAUTH_CMD:
	case ATH_CMD_RPWAHD_BLACK_LIST_CMD:
	case ATH_CMD_RPWAHD_11V_STEER_CMD:
	case ATH_CMD_RPWAHD_STA_ASSOC_STATUS_CMD:
	case ATH_CMD_RPWAHD_INTF_STATUS_CMD:
	case ATH_CMD_RPWAHD_INTF_INFO_CMD:
		printf("Not implement.\n");
		break;
	case ATH_CMD_RPWAHD_DEBUG_CMD:
		if(argv[2]){
			memcpy(data, argv[2], strlen(argv[2]));
		}
		tp_mesh_send_cmd(mesh_sock, ATH_CMD_RPWAHD_DEBUG_CMD, data);
//		tp_mesh_send_cmd(ATH_EVENT_RPWAHD_DEBUG_EVENT, data);
		break;
	default:
		printf("No Msg to send\n");
		usage();
	}

	tp_mesh_netlink_destory(mesh_sock);
}
