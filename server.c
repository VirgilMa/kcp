// Server side implementation of UDP client-server model
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include "ikcp.h"

#define PORT 8080
#define MAXLINE 1024

static int sockfd;
static struct sockaddr_in servaddr, cliaddr;
static time_t seconds;

static void initSock()
{
    // Creating socket file descriptor
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&servaddr, 0, sizeof(servaddr));
    memset(&cliaddr, 0, sizeof(cliaddr));

    // Filling server information
    servaddr.sin_family = AF_INET; // IPv4
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(PORT);

    // Bind the socket with the server address
    if (bind(sockfd, (const struct sockaddr *)&servaddr,
             sizeof(servaddr)) < 0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
}

static int udp_output(const char *buf, int len, ikcpcb *kcp, void *user)
{
    sendto(sockfd, buf, len, 0, (const struct sockaddr *)&cliaddr, sizeof(cliaddr));
    return 0;
}

static uint64_t time_ms()
{
    struct timespec cur;
    clock_gettime(CLOCK_MONOTONIC_RAW, &cur);
    return cur.tv_sec * 1000 + cur.tv_nsec / 1000000;
}

int main()
{
    int n;
    socklen_t len;
    char buffer[MAXLINE];
    char *hello = "Hello from server";

    initSock();

    ikcpcb *kcp1 = ikcp_create(0x11223344, (void *)0);
    kcp1->output = udp_output;
    ikcp_wndsize(kcp1, 128, 128);
    ikcp_nodelay(kcp1, 0, 10, 0, 0);

    len = sizeof(cliaddr); // len is value/resuslt

    while (1)
    {
        ikcp_update(kcp1, time_ms());

        while (1)
        {
            n = recvfrom(sockfd, (char *)buffer, MAXLINE - 1,
                         MSG_DONTWAIT, (struct sockaddr *)&cliaddr, &len);
            if (n < 0)
                break;
            ikcp_input(kcp1, buffer, n);
        }
        while (1)
        {
            n = ikcp_recv(kcp1, buffer, MAXLINE - 1);
            if (n < 0)
                break;
            buffer[n] = '\0';
            printf("Client : %s\n", buffer);
			ikcp_send(kcp1, (const char *)hello, strlen(hello));
            printf("Hello message sent.\n");
        }
        sleep(1);
    }

    return 0;
}
