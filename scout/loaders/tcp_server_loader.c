#include "scout/loaders/tcp_server_loader.h"
#include "scout/tcp_server.h"
#include "scout/pack.h"
#include "scout/pic/pic_wrapper.h"

/* External API function */
extern void flush_cache(uint8_t * buffer, uint32_t size);

void scout_main()
{
    struct sockaddr_in clientAddr;
    socklen_t clientAddrSize = sizeof(clientAddr);
    sock_fd clientSock;
	sock_fd serverSock;
	int32_t status;
    uint32_t size;
    uint8_t * receiveBuffer;

#ifdef SCOUT_RESTORE_FLOW
    clientAddrSize = 0;
    clientSock = 0;
	serverSock = 0;
    receiveBuffer = NULL;
#endif /* SCOUT_RESTORE_FLOW */

	/* Open the TCP server */
	status = open_tcp_server(&serverSock, SCOUT_LOADER_PORT);
	if (status != STATUS_OK)
	{
		goto free_resources;
	}

    /* Accept the client */
    clientSock = accept(serverSock, (struct sockaddr *)&clientAddr, &clientAddrSize);
    if (clientSock < 0)
    {
        goto free_resources;
    }

    /* Receive the allocation size */
    if (full_net_recv(clientSock, (uint8_t*)&size, sizeof(size)) != sizeof(size))
    {
        goto free_resources;
    }

    size = ntohl(size);

    /* Allocate the receive buffer */
    receiveBuffer = (uint8_t*)malloc(size);
    if (receiveBuffer == NULL)
    {
        goto free_resources;
    }

    /* Receive the loaded content */
    if (full_net_recv(clientSock, receiveBuffer, size) != size)
    {
        goto free_resources;
    }

    /* Flush the cache for this buffer */
    flush_cache(receiveBuffer, size);

    /* Jump into the buffer */
    ((void (*)(void))receiveBuffer)();

free_resources:
#ifdef SCOUT_RESTORE_FLOW
    /* Free the buffer */
    if (receiveBuffer != NULL)
    {
        free(receiveBuffer);
    }

    /* Close the sockets */
    if (serverSock > 0)
    {
        close(serverSock);
    }

    if (clientSock > 0)
    {
        close(clientSock);
    }
#endif /* SCOUT_RESTORE_FLOW */
    return;
}
