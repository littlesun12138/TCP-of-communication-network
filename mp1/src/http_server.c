/*
** server.c -- a stream socket server demo
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>

#define PORT "3490"  // the port users will be connecting to

#define BACKLOG 10	 // how many pending connections queue will hold
#define FN_LENGTH 1024

void sigchld_handler(int s)
{
	while(waitpid(-1, NULL, WNOHANG) > 0);
}

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(void)
{
	int sockfd, new_fd,numbytes,bytescount;  // listen on sock_fd, new connection on new_fd
	struct addrinfo hints, *servinfo, *p;
	struct sockaddr_storage their_addr; // connector's address information
	socklen_t sin_size;
	struct sigaction sa;
	int yes=1;
	char s[INET6_ADDRSTRLEN];
	int rv;

    	char http_request[FN_LENGTH];
	char readbuf[FN_LENGTH];
    	memset(http_request,0,FN_LENGTH);
	memset(readbuf, 0, FN_LENGTH);
	
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE; // use my IP

	if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// loop through all the results and bind to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
			perror("server: socket");
			continue;
		}

		if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
				sizeof(int)) == -1) {
			perror("setsockopt");
			exit(1);
		}

		if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);
			perror("server: bind");
			continue;
		}

		break;
	}

	if (p == NULL)  {
		fprintf(stderr, "server: failed to bind\n");
		return 2;
	}

	freeaddrinfo(servinfo); // all done with this structure

	if (listen(sockfd, BACKLOG) == -1) {
		perror("listen");
		exit(1);
	}

	sa.sa_handler = sigchld_handler; // reap all dead processes
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	if (sigaction(SIGCHLD, &sa, NULL) == -1) {
		perror("sigaction");
		exit(1);
	}

	printf("server: waiting for connections...\n");

	while(1) {  // main accept() loop
		sin_size = sizeof their_addr;
		new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
		if (new_fd == -1) {
			perror("accept");
			continue;
		}

		inet_ntop(their_addr.ss_family,
			get_in_addr((struct sockaddr *)&their_addr),
			s, sizeof s);
		printf("server: got connection from %s\n", s);

		if (!fork()) { // this is the child process
			close(sockfd); // child doesn't need the listener
            char correct_sig[FN_LENGTH]="HTTP/1.1 200 OK\r\n\r\n";
            char noexist_sig[FN_LENGTH]="HTTP/1.1 404 Not Found\r\n\r\n";
            char error_sig[FN_LENGTH]="HTTP/1.1 400 Bad Request\r\n\r\n";
            if((numbytes = recv(new_fd, http_request, FN_LENGTH, 0)) <0){
	    		perror("recv");
	    		close(new_fd);
	    		exit(1);
			}
            char file_name[FN_LENGTH];
            memset(file_name, 0, sizeof file_name);

			char* str_end;
            //now start install filename
			str_end=strchr(http_request+5,' ');		
			//no pathname is download
			if(str_end-http_request-5==0){
				if (send(new_fd, error_sig, strlen(error_sig), 0) == -1){
					perror("send");
				}
                close(new_fd);
                exit(1);
			}	
			strncpy(file_name, http_request+5, str_end-http_request-5);
			printf("%s\n",file_name);
			FILE *fp = fopen(file_name, "rb");

			//path not exist
			if (fp==NULL){
				//printf("%s\n","qq");
				if (send(new_fd, noexist_sig, strlen(noexist_sig), 0) == -1){
					perror("send");
				}
                close(new_fd);    
                exit(1);				
			}
			//printf("%s",correct_sig);
			//if file exists, send correct header
			if (send(new_fd, correct_sig, strlen(correct_sig), 0) == -1){
                perror("send");
                close(new_fd);
                exit(1);
            }

            //keep reading from file until no bits left
            while(1){
            	bytescount = fread(readbuf, sizeof(char), FN_LENGTH, fp);
            	if(bytescount<=0){
            		break;
            	}
				else{
					if (send(new_fd, readbuf, bytescount, 0) == -1) {
						close(new_fd);
						perror("send");
						exit(1);
					}	
					memset(readbuf, 0, FN_LENGTH);
				}
            }
			//all done
			fclose(fp);	
			close(new_fd);
			exit(0);
		}
		close(new_fd);  // parent doesn't need this
	}

	return 0;
}
