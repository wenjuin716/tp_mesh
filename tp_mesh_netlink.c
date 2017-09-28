#include "tp_mesh_netlink.h"

int tp_mesh_netlink_init (void)
{
        int sock_fd = -1;
        struct sockaddr_nl src_addr;

        sock_fd=socket(AF_NETLINK, SOCK_RAW, NETLINK_RPWAHD_EVENT);
        if(sock_fd<0){
                printf("create netlink socket fail, group=%d\n", NETLINK_RPWAHD_EVENT);
                return -1;
        }

        memset(&src_addr, 0, sizeof(src_addr));
        src_addr.nl_family = AF_NETLINK;
        src_addr.nl_pid = getpid();  /* self pid */
        /* interested in group 1<<0 */
        bind(sock_fd, (struct sockaddr*)&src_addr, sizeof(src_addr));
        if(bind(sock_fd, (struct sockaddr*)&src_addr, sizeof(src_addr)) < 0)
        {
                printf("Could not bind netlink socket for tp_mesh\n");
                close(sock_fd);
                sock_fd = 0;
		return -1;
        }

        return sock_fd;
}

void tp_mesh_netlink_destory(int fd){
	if(fd>0)
		close(fd);
}

void tp_mesh_event_handler(int fd)
{
	int len;
	char buf[4096];     /* 8192 to avoid message truncation on
				platforms with page size > 4096 */
	struct iovec iov = { buf, sizeof(buf) };
	struct sockaddr_nl sa;
	struct msghdr msg;
	struct nlmsghdr *nh;
	ath_rpwahd_event_t *rpwahd_buf = NULL;

        if(fd<0){
                printf("invalid netlink socket\n");
                return;
        }

	/* init msghdr */
	msg.msg_name = &sa;
	msg.msg_namelen = sizeof(sa);
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;
	msg.msg_control= NULL;
	msg.msg_controllen = 0;
	msg.msg_flags = 0;

	len = recvmsg(fd, &msg, 0);
	
	for (nh = (struct nlmsghdr *) buf; NLMSG_OK (nh, len); nh = NLMSG_NEXT (nh, len)) {
		/* The end of multipart message */
		if (nh->nlmsg_type == NLMSG_DONE)
			return;

		/* Do some error handling */
		if (nh->nlmsg_type == NLMSG_ERROR){
			printf("[%s] nlmsg tpye Error.\n", __FUNCTION__);
			continue;
		}
	
		/* Continue with parsing payload */
		if(!(rpwahd_buf = NLMSG_DATA(nh))){
			printf("[%s] payload is NULL\n", __FUNCTION__);
			continue;
		}
		
		switch(rpwahd_buf->type){
		case ATH_EVENT_RPWAHD_PROBE_REQ_EVENT :
			break;
		case ATH_EVENT_RPWAHD_STA_ASSOCIATED_EVENT :
			break;
		case ATH_EVENT_RPWAHD_DEAUTH_EVENT :
			break;
		case ATH_EVENT_RPWAHD_INTF_INFO_EVENT :
			break;
		case ATH_EVENT_RPWAHD_INTF_STATUS_EVENT :
			break;
		case ATH_EVENT_RPWAHD_STA_STATUS_EVENT :
			break;
		case ATH_EVENT_RPWAHD_NONASSOC_STA_STATUS_EVENT :
			break;
		case ATH_EVENT_RPWAHD_DEBUG_EVENT :
			printf("receive \'%s\' from Kernel.\n", (rpwahd_buf->data).test_str);
			break;
		default:
			printf("Invalid Event!\n");
			break;
		}		
	}
}

size_t mesh_cmd_payload_len(ATH_RPWAHD_CMD type)
{
	size_t data_len = 0;
	switch(type)
	{
		case ATH_CMD_RPWAHD_DEAUTH_CMD :
			data_len=sizeof(struct rpwahd_deauth_ind);
			break;
		case ATH_CMD_RPWAHD_BLACK_LIST_CMD :
                        data_len=sizeof(struct rpwahd_blacklist_ind);
                        break;
		case ATH_CMD_RPWAHD_11V_STEER_CMD :
                        data_len=sizeof(struct rpwahd_11v_steer_ind);
                        break;
		case ATH_CMD_RPWAHD_STA_ASSOC_STATUS_CMD :
                        data_len=sizeof(struct rpwahd_assoc_sta_ind);
                        break;
		case ATH_CMD_RPWAHD_INTF_STATUS_CMD :
                        data_len=sizeof(struct rpwahd_nonassoc_sta_ind);	//need check
                        break;
		case ATH_CMD_RPWAHD_INTF_INFO_CMD :
                        data_len=sizeof(struct rpwahd_nonassoc_sta_ind);	//need check
                        break;
		case ATH_CMD_RPWAHD_DEBUG_CMD :
			data_len = 100*sizeof(char);	//char test_str[100]
			break;
		default:
			printf("Invalid Cmd type!\n");
			break;
	}

	return data_len;
}

void tp_mesh_send_cmd(int fd, ATH_RPWAHD_CMD type,void *data)
{

	struct nlmsghdr *nh = NULL;    /* The nlmsghdr with payload to send */
	struct sockaddr_nl sa;
	struct iovec iov;
	struct msghdr msg;
	size_t payload_len = mesh_cmd_payload_len(type);

        if(fd<0){
                printf("invalid netlink socket\n");
                return;
        }

        if(payload_len == 0){
                printf("invalid Cmd type\n");
                return;
        }

        if(!data){
                printf("payload data is NULL.\n");
                return;
        }

	/* allocat nlmsg memory */
	nh = (struct nlmsghdr *) malloc(NLMSG_SPACE(payload_len));   //NLMSG_SPACE inclide NLMSG_HDRLEN

	/* init nlmsghdr */
	nh->nlmsg_len = NLMSG_SPACE(payload_len);
	nh->nlmsg_type = NLMSG_DONE;	/* LMSG_DONE message terminates a multipart message. */
        nh->nlmsg_flags |= NLM_F_ACK;	/* Request an ack from kernel by setting NLM_F_ACK */
	//nh->nlmsg_seq = ++sequence_number;
        nh->nlmsg_pid = getpid();

	/* assign nlmsg payload */
	printf("data=%s\n", (char *)data);
	memcpy(NLMSG_DATA(nh), data, payload_len);
	printf("NLMSG_DATA(nh)=%s\n", (char *)NLMSG_DATA(nh));


	/* init iov structure */
	iov.iov_base = nh;
	iov.iov_len = nh->nlmsg_len;	//nlmsg len including header

        memset(&sa, 0, sizeof(sa));
        sa.nl_family = AF_NETLINK;

	/* init msghdr */
	msg.msg_name = &sa;
	msg.msg_namelen = sizeof(sa);
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;
	msg.msg_control= NULL;
	msg.msg_controllen = 0;
	msg.msg_flags = 0;

	if(sendmsg(fd, &msg, 0) < 0){
		printf("%s : sendmsg Error!!!!\n", __FUNCTION__);
	}else{
		printf("%s : sendmsg success.\n", __FUNCTION__);
	}
}
