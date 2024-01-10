#include <stdio.h>
#include <sys/socket.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>


#define MAX_CLIENTS 3
#define PORT 32400
#define BUFFER_SIZE 128

////////////////////////////////////////////////////
// declaring a signal handler
// sig_atomic_t for reduction cmd to 1 instruction
// it helps control signal
// if there, for example, 2 instr, then between them
// may interfere some operations which change signal
volatile sig_atomic_t was_sighup = 0;

void sighup_handler(int signal)
{
	was_sighup = 1;
}
////////////////////////////////////////////////////

int main() 
{
    ////////////////////////////////////////////
    // Create socket
    // AF_INET for IPv4
    // SOCK_STREAM for TCP
    // 0 for 1 protocol
	int socket_fd = socket(AF_INET, SOCK_STREAM, 0);;
	if (socket_fd < 0)
	{
		perror("Couldn't create socket");
		exit(1);
	}
    ////////////////////////////////////////////

    ////////////////////////////////////////////////
    // assign IP and Port
	struct sockaddr_in server_addr;
	memset(&server_addr, 0, sizeof(server_addr)); // set zeros to addr
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons(PORT);
    ////////////////////////////////////////////////

    // bind socket and addr
	if (bind(socket_fd, (struct sockaddr *) &server_addr, sizeof(struct sockaddr_in)) != 0)
	{
		perror("Couldn't bind socket");
		exit(1);
	}

	listen(socket_fd, MAX_CLIENTS);
    
	printf("Server ONLINE\n");
	printf("Use \"nc localhost %d\" to connect to server\n", PORT);
	printf("To stop Server use \"kill -HUP %d\"\n", getpid());
	printf("To stop Client send \"quit\"\n\n");

    ////////////////////////////////////
    // registration of a signal handler
	struct sigaction sa;
	sigaction(SIGHUP, NULL, &sa);							// reset signal
	sa.sa_handler = sighup_handler;							// specify the action to be associated with sighum
	sa.sa_flags |= SA_RESTART;							// setting up signal behavior
	sigaction(SIGHUP, &sa, NULL);							// register signal
    ////////////////////////////////////

    /////////////////////////////////////////////
    // signal block
	sigset_t mask;									//initial mask
	sigset_t sighup_mask;
	sigemptyset(&sighup_mask);
	sigaddset(&sighup_mask, SIGHUP);
	sigprocmask(SIG_BLOCK, &sighup_mask, &mask);
    /////////////////////////////////////////////

	int clients[MAX_CLIENTS] = {0};
	int active_clients = 0;
	char message[BUFFER_SIZE] = {0};

	while(1)
	{
		//////////////////////////////////////////
		// put descriptors in set
		int maxFd = socket_fd;

		fd_set fds;
		FD_ZERO(&fds);
		FD_SET(socket_fd, &fds);
		for (int i = 0; i < active_clients; i++)
		{
			FD_SET(clients[i], &fds);
			if (clients[i] > maxFd)
			{
				maxFd = clients[i];
			}
		}
		//////////////////////////////////////////

		//////////////////////////////////////////////////////////////////////////////////////
		// pselect: unblock signal by using mask and wait activity from descriptors in set fds
		// in fact pselect create a queue for different events
	        if (pselect(maxFd + 1, &fds, NULL, NULL, NULL, &mask) < 0 && errno != EINTR)
	        {
	            perror("pselect error");
	            exit(1);
	        }
		//////////////////////////////////////////////////////////////////////////////////////

		///////////////////////////////////
		// work with signal
	        if (was_sighup)
	        {
	            printf("Signal received.\n");
	            was_sighup = 0;
	            break;						// leave while(1), end app
	        }
		///////////////////////////////////

		if (FD_ISSET(socket_fd, &fds))
		{
			int conn = accept(socket_fd, NULL, NULL);
            		// check for connection ok
			if (conn < 0)
			{
				perror("Couldn't accept new connection");
				continue;
			}

            		// check for clients limit
			if (active_clients + 1 > MAX_CLIENTS)
			{
				printf("New client: Too many clients -> Refused\n");
				close(conn);
				continue;
			}

			clients[active_clients] = conn;
			active_clients++;
            		printf("New client: Connected -> ID#%d\n", abs(MAX_CLIENTS - conn));
		}

		/////////////////////////////////////////////////////////////////////////////////////////////
        	// messaging
		for (int i = 0; i < active_clients; i++)
		{
			if (FD_ISSET(clients[i], &fds))				// if client i connected
			{
				memset(&message, 0, BUFFER_SIZE);
				ssize_t data = read(clients[i], &message, BUFFER_SIZE - 1);

				////////////////////////////
                		// if got data
				if (data > 0)
				{
					data--;
					message[data] = '\0';
				}
				////////////////////////////

				/////////////////////////////////////////////////////////////////////////
               			// if error or client send quit
				if (data <= 0 || !strcmp("quit", message))
				{
					close(clients[i]);
					printf("Connection closed: ID#%d\n", abs(MAX_CLIENTS - clients[i]));

					clients[i] = clients[active_clients - 1];
					clients[active_clients - 1] = 0;

					active_clients--;
					i--;

					continue;
				}
				/////////////////////////////////////////////////////////////////////////

				printf("ID#%d: got %ld bytes -> %s\n", abs(MAX_CLIENTS - clients[i]), data, message);
			}
		}
		/////////////////////////////////////////////////////////////////////////////////////////////
	}

	printf("Closing socket.\n");
	close(socket_fd);
	printf("Socket closed, exit app.\n");
	
	return EXIT_SUCCESS;
}
