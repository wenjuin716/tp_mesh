/*
 * Copyright (C) 2012-2017  B.A.T.M.A.N. contributors:
 *
 * Simon Wunderlich
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of version 2 of the GNU General Public
 * License as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA
 *
 */

#include <ctype.h>
#include <errno.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <net/if.h>
#include <stddef.h>
#include <sys/socket.h>
#include "alfred.h"
#include "packet.h"

int alfred_client_request_data(struct globals *globals)
{
	unsigned char buf[MAX_PAYLOAD], *pos;
	struct alfred_request_v0 *request;
	struct alfred_push_data_v0 *push;
	struct alfred_status_v0 *status;
	struct alfred_tlv *tlv;
	struct alfred_data *data;
	int ret, len, data_len, i;
	const size_t buf_data_len = sizeof(buf) - sizeof(*push) - sizeof(*data);

	if (unix_sock_open_client(globals))
		return -1;

	request = (struct alfred_request_v0 *)buf;
	len = sizeof(*request);

	request->header.type = ALFRED_REQUEST;
	request->header.version = ALFRED_VERSION;
	request->header.length = sizeof(*request) - sizeof(request->header);
	request->header.length = htons(request->header.length);
	request->requested_type = globals->clientmode_arg;
	request->tx_id = get_random_id();

	ret = write(globals->unix_sock, buf, len);
	if (ret != len)
		fprintf(stderr, "%s: only wrote %d of %d bytes: %s\n",
			__func__, ret, len, strerror(errno));

	push = (struct alfred_push_data_v0 *)buf;
	tlv = (struct alfred_tlv *)buf;
	while ((ret = read(globals->unix_sock, buf, sizeof(*tlv))) > 0) {
		if (ret < (int)sizeof(*tlv))
			break;

		if (tlv->type == ALFRED_STATUS_ERROR)
			goto recv_err;

		if (tlv->type != ALFRED_PUSH_DATA)
			break;

		/* read the rest of the header */
		ret = read(globals->unix_sock, buf + sizeof(*tlv),
			   sizeof(*push) - sizeof(*tlv));

		/* too short */
		if (ret < (int)(sizeof(*push) - (int)sizeof(*tlv)))
			break;

		/* read the rest of the header */
		ret = read(globals->unix_sock, buf + sizeof(*push),
			   sizeof(*data));

		if (ret < (ssize_t)sizeof(*data))
			break;

		data = push->data;
		data_len = ntohs(data->header.length);

		/* would it fit? it should! */
		if (data_len > (int)buf_data_len)
			break;

		/* read the data */
		ret = read(globals->unix_sock,
			   buf + sizeof(*push) + sizeof(*data), data_len);

		/* again too short */
		if (ret < data_len)
			break;

		pos = data->data;

		printf("{ \"%02x:%02x:%02x:%02x:%02x:%02x\", \"",
		       data->source[0], data->source[1],
		       data->source[2], data->source[3],
		       data->source[4], data->source[5]);
		for (i = 0; i < data_len; i++) {
			if (pos[i] == '"')
				printf("\\\"");
			else if (pos[i] == '\\')
				printf("\\\\");
			else if (!isprint(pos[i]))
				printf("\\x%02x", pos[i]);
			else
				printf("%c", pos[i]);
		}

		printf("\"");

		if (globals->verbose)
			printf(", %u", data->header.version);

		printf(" },\n");
	}

	unix_sock_close(globals);

	return 0;

recv_err:
	/* read the rest of the status message */
	ret = read(globals->unix_sock, buf + sizeof(*tlv),
		   sizeof(*status) - sizeof(*tlv));

	/* too short */
	if (ret < (int)(sizeof(*status) - sizeof(*tlv)))
		return -1;

	status = (struct alfred_status_v0 *)buf;
	fprintf(stderr, "Request failed with %d\n", status->tx.seqno);

	return status->tx.seqno;
}

int alfred_client_set_data(struct globals *globals)
{
	unsigned char buf[MAX_PAYLOAD];
	struct alfred_push_data_v0 *push;
	struct alfred_data *data;
	int ret, len;

	if (unix_sock_open_client(globals))
		return -1;

	push = (struct alfred_push_data_v0 *)buf;
	data = push->data;
	len = sizeof(*push) + sizeof(*data);
	while (!feof(stdin)) {
		ret = fread(&buf[len], 1, sizeof(buf) - len, stdin);
		len += ret;

		if (sizeof(buf) == len)
			break;
	}

	push->header.type = ALFRED_PUSH_DATA;
	push->header.version = ALFRED_VERSION;
	push->header.length = htons(len - sizeof(push->header));
	push->tx.id = get_random_id();
	push->tx.seqno = htons(0);

	/* we leave data->source "empty" */
	memset(data->source, 0, sizeof(data->source));
	data->header.type = globals->clientmode_arg;
	data->header.version = globals->clientmode_version;
	data->header.length = htons(len - sizeof(*push) - sizeof(*data));

	ret = write(globals->unix_sock, buf, len);
	if (ret != len)
		fprintf(stderr, "%s: only wrote %d of %d bytes: %s\n",
			__func__, ret, len, strerror(errno));

	unix_sock_close(globals);
	return 0;
}

int alfred_client_modeswitch(struct globals *globals)
{
	unsigned char buf[MAX_PAYLOAD];
	struct alfred_modeswitch_v0 *modeswitch;
	int ret, len;

	if (unix_sock_open_client(globals))
		return -1;

	modeswitch = (struct alfred_modeswitch_v0 *)buf;
	len = sizeof(*modeswitch);

	modeswitch->header.type = ALFRED_MODESWITCH;
	modeswitch->header.version = ALFRED_VERSION;
	modeswitch->header.length = htons(len - sizeof(modeswitch->header));

	switch (globals->opmode) {
	case OPMODE_SLAVE:
		modeswitch->mode = ALFRED_MODESWITCH_SLAVE;
		break;
	case OPMODE_MASTER:
		modeswitch->mode = ALFRED_MODESWITCH_MASTER;
		break;
	default:
		fprintf(stderr, "%s: unknown opmode %u in modeswitch\n",
			__func__, globals->opmode);
		return -1;
	}

	ret = write(globals->unix_sock, buf, len);
	if (ret != len)
		fprintf(stderr, "%s: only wrote %d of %d bytes: %s\n",
			__func__, ret, len, strerror(errno));

	unix_sock_close(globals);
	return 0;
}

static int check_interface(const char *iface)
{
	int sock = -1;
	struct ifreq ifr;

	if (strlen(iface) > IFNAMSIZ) {
		fprintf(stderr, "%s: interface name list too long, not changing\n",
			__func__);
		return -1;
	}

	sock = socket(PF_INET, SOCK_DGRAM, 0);
	if (sock < 0) {
		perror("can't open socket");
		return -1;
	}

	strncpy(ifr.ifr_name, iface, IFNAMSIZ);
	ifr.ifr_name[IFNAMSIZ - 1] = '\0';
	if (ioctl(sock, SIOCGIFINDEX, &ifr) == -1) {
		fprintf(stderr, "%s: can't find interface, not changing\n",
			__func__);
		close(sock);
		return -1;
	}

	close(sock);

	return 0;
}

int alfred_client_change_interface(struct globals *globals)
{
	unsigned char buf[MAX_PAYLOAD];
	struct alfred_change_interface_v0 *change_interface;
	int ret, len;
	char *input, *token, *saveptr;
	size_t interface_len;

	if (unix_sock_open_client(globals))
		return -1;

	interface_len = strlen(globals->change_interface);
	if (interface_len > sizeof(change_interface->ifaces)) {
		fprintf(stderr, "%s: interface name list too long, not changing\n",
			__func__);
		return 0;
	}

	change_interface = (struct alfred_change_interface_v0 *)buf;
	len = sizeof(*change_interface);

	change_interface->header.type = ALFRED_CHANGE_INTERFACE;
	change_interface->header.version = ALFRED_VERSION;
	change_interface->header.length = htons(len - sizeof(change_interface->header));
	strncpy(change_interface->ifaces, globals->change_interface,
		sizeof(change_interface->ifaces));
	change_interface->ifaces[sizeof(change_interface->ifaces) - 1] = '\0';

	/* test it before sending
	 * globals->change_interface is now saved in change_interface->ifaces
	 * and can be modified by strtok_r
	 */
	input = globals->change_interface;
	while ((token = strtok_r(input, ",", &saveptr))) {
		input = NULL;

		ret = check_interface(token);
		if (ret < 0)
			return 0;
	}

	ret = write(globals->unix_sock, buf, len);
	if (ret != len)
		fprintf(stderr, "%s: only wrote %d of %d bytes: %s\n",
			__func__, ret, len, strerror(errno));

	unix_sock_close(globals);

	return 0;
}
