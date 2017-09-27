#include "tp_mesh_netlink.h"

int tp_mesh_netlink_init (void)
{
        int sock_fd;
        struct sockaddr_nl src_addr;

        sock_fd=socket(PF_NETLINK, SOCK_RAW, NETLINK_RPWAHD_EVENT);
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
        }else{
                printf("bind netlink socket Success, sock=%d\n", sock_fd);
        }

        return sock_fd;
}

void tp_mesh_event_handler(int fd)
{
	int len;
	char buf[4096];     /* 8192 to avoid message truncation on
				platforms with page size > 4096 */
	struct iovec iov = { buf, sizeof(buf) };
	struct sockaddr_nl sa;
	//struct msghdr msg = { &sa, sizeof(sa), &iov, 1, NULL, 0,0};
	struct msghdr msg;
	struct nlmsghdr *nh;

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

		}
	
		/* Continue with parsing payload */	
	}
}
