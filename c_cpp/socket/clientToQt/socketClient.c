
/* Include for .. */
#include <inttypes.h>	/* .. "uint32_t" */
#include <signal.h>	/* .. signal() */
#include <stdio.h>
#include <stdlib.h>	/* .. exit(), atoi() */
#include <string.h>	/* .. memset() */
#include <unistd.h>	/* .. close() */

#include <asm/byteorder.h>	/* .. __cpu_to_be32p() */
#include <arpa/inet.h>		/* .. inet_pton() */
#include <netinet/in.h>		/* .. htons(), struct sockaddr_in */
#include <sys/socket.h>		/* .. socket(), connect(), recv(), send() */

#define BUFFER_LEN  300

/* Uncomment to enable debug prints */
//#define ENA_DBG 1
#if defined(ENA_DBG)
	#define DBG(FMT, ...)	printf(FMT, ##__VA_ARGS__)
#else
	#define DBG(FMT, ...)
#endif

/* Routing index that the client can use */
typedef enum MSG_IDX { PENDING, QUITING} msg_idx_t;

/* Routing variable */
msg_idx_t message_to_send = PENDING;

/**
 * @brief  Signal SIGINT handler
 *         Receive SIGINT to properly end program and end connection
 */
void sigint_handler(int sig) {
	printf(" SIGINT handler w/ code: %d\n", sig);
	message_to_send = QUITING;
}

int main(int argc, char **argv) {
	int running = 1;
	int socketFd, port;
	struct sockaddr_in srv_addr;
	/* Receiving buffer */
	char buff[BUFFER_LEN];
	int streamLen;
	uint32_t dataLen;
	int leaveFromClt = 1;
	int i;

	if (argc != 3) {
		fprintf(stderr, "usage: %s <SERVER_IP> <PORT>\n", argv[0]);
		fprintf(stderr, "\tSERVER_IP: 10's base for IPv4\n");
		fprintf(stderr, "\t           16's base for IPv6\n");
		fprintf(stderr, "\tPORT     : Port's communication\n");
		exit(EXIT_FAILURE);
	}

	/* Fully initiate srv_addr struct to 0 */
	memset(&srv_addr, 0, sizeof(srv_addr));

	/* Parsing server's IP */
	if (inet_pton(AF_INET, argv[1], &srv_addr.sin_addr) <= 0) {
		fprintf(stderr, "Converting server's IP failed\n");
		exit(EXIT_FAILURE);
	}

	/* Parsing server's port */
	port = atoi(argv[2]);

	/* Connect signal to handler */
	signal(SIGINT, sigint_handler);

	/* Socket's creation */
	socketFd = socket(AF_INET, SOCK_STREAM, 0);
	if (socketFd < 0) {
		fprintf(stderr, "Could not create socket client side\n");
		exit(EXIT_FAILURE);
	}
	DBG("Step passed: socket\n");

	/* Fill server's infos */
	srv_addr.sin_family = AF_INET;
	srv_addr.sin_port = htons(port);

	printf("Connecting to server at %s:%d...\n", argv[1], port);
	if (connect(socketFd, (struct sockaddr *) &srv_addr,
		    sizeof(srv_addr)) < 0) {
		fprintf(stderr, "FAILURE\n");
		/* Clean ressource(s) that could be used */
		close(socketFd);
		exit(EXIT_FAILURE);
	}
	DBG("Step passed: connect\n");

	printf("~~~ WELCOME TO THE PROGRAM ~~~\n");
	while (running) {
		if (message_to_send == QUITING)	running = 0;

		/* Use MSG_DONTWAIT as MSG_WAITALL wait for buffer
		 * to be filled with BUFFER_LEN elements (if no error occurs).
		 * Other flags, like MSG_TRUNC, are in evaluation
		 * as possibilities. */
		streamLen = recv(socketFd, (void *) buff, BUFFER_LEN,
				 MSG_DONTWAIT);
		//streamLen = recv(socketFd, (void *) buff, BUFFER_LEN,
		//		 MSG_WAITALL);
		//streamLen = recv(socketFd, (void *) buff, BUFFER_LEN,
		//		 MSG_TRUNC);

		if (streamLen < 4)	continue;

		/* To Big Endian, as QTcpServer send MSB -> LSB by default
		 * and my machine is Little Endian ordered
		 * That's why (uint32_t *)buff might not work as it may
		 * "invert" the value,
		 * e.g. Get "0x2A00" instead of the correct value "0x002A" */
		dataLen = __cpu_to_be32p((uint32_t *)buff);
		DBG("Server's response (length: %d, actual data: %d):\n",
		    streamLen, dataLen);

		#if defined(ENA_DBG)
		for (i = 0; i < streamLen; i++)
			printf("%02X%c", buff[i],
					 ((i+1)%10) == 0 ? '\n' : ' ');
		if (i % 10)	printf("\n");
		printf("Server's msg? %s\n", buff+5);
		#endif

		printf("SVR: ");
		for (i = 0; i < dataLen; i++)
			printf("%c", (buff+4)[i]);
		printf("\n");

		/* Detect leaving message */
		if ((dataLen == 7) && ( ! strncmp(buff+4, "Leaving", 7) )) {
			leaveFromClt = 0;
			running = 0;
		}
	}

	if (leaveFromClt) {
		/* Leaving message (matching QDataStream format)
		 * to properly close connection on server's side */
		if (send(socketFd, (void *) "\0\0\0\x07Leaving", 11, 0) != 11)
			fprintf(stderr, "Sending failed\n");
	}

	/* Resources cleaning */
	close(socketFd);

	return 0;
}
