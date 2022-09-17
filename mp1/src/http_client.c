/*
** client.c -- a stream socket client demo
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <arpa/inet.h>

#define PORT "3490" // the port client will be connecting to 
#define PORT_80 "80"
#define FN_LENGTH 1024
#define MAXDATASIZE 1024 // max number of bytes we can get at once 
#define BYTES_SIZE 8
#define PORTLEN 10

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}
void input_extract(char* argument, char* ipaddr , char* port , char* fn){
	char* str_start = argument;
	char* str_end;
	char* str_gang_end;
	//char* str_true_end;
	printf("%s\n","fk");
	if (str_start == NULL ){
		return;
	}
	while(*str_start != ':' ){
		str_start++;
	}
	//printf("%c",*str_start);
	
	// Find the First ":" and get the addr of the first char
	str_start = str_start + 3;
	str_end   = str_start;
	
	// find second :

	if(strchr(str_start,':') == NULL){
		strcpy(port,PORT_80);
		str_gang_end=strchr(str_start,'/');
		strncpy(ipaddr,str_start,str_gang_end-str_start);
	}
	else{
		str_end=strchr(str_start,':')-1;
		strncpy(ipaddr,str_start,str_end-str_start+1);
		str_gang_end=strchr(str_start,'/');
		strncpy(port,str_end+2,str_gang_end-str_end-2);
	}
	printf("%s\n",ipaddr);
	printf("%s\n",port);
	
	//str_true_end=str_gang_end;
	//while(*str_true_end!='\n'){
	//	str_true_end++;
	//}
	strncpy(fn,str_gang_end+1,strlen(str_gang_end)-1);
	printf("%s\n",fn);
	
}
int main(int argc, char *argv[])
{
	printf("fk");
	
	int sockfd, numbytes;  
	char buf[MAXDATASIZE];
	struct addrinfo hints, *servinfo, *p;
	int rv;
	char s[INET6_ADDRSTRLEN];
	char file_name[FN_LENGTH];
	char port_name[FN_LENGTH];
	char ipad_name[FN_LENGTH];

	if (argc != 2) {
	    fprintf(stderr,"usage: client hostname\n");
	    exit(1);
	}

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	// Init for memory
	memset(file_name, 0, sizeof file_name);
	memset(port_name, 0, sizeof port_name);
	memset(ipad_name, 0, sizeof ipad_name);


	input_extract(argv[1],ipad_name,port_name,file_name);
	//exit(1);
	printf("%s\n",ipad_name);
	printf("%s\n",port_name);
	printf("%s\n",file_name);
	if ((rv = getaddrinfo(ipad_name, port_name, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// loop through all the results and connect to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
			perror("client: socket");
			continue;
		}

		if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);
			perror("client: connect");
			continue;
		}

		break;
	}

	if (p == NULL) {
		fprintf(stderr, "client: failed to connect\n");
		return 2;
	}

	inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
			s, sizeof s);
	printf("client: connecting to %s\n", s);

	freeaddrinfo(servinfo); // all done with this structure

	// design my HTTP GET
	char http_request[FN_LENGTH];

	strncpy(http_request,"GET ",4*BYTES_SIZE);
	strcat(http_request,file_name);
	strcat(http_request," HTTP/1,1\r\nUser-Agent: Wget/1.12 (linux-gnu)\r\nHost: ");
	strcat(http_request,ipad_name);
	strcat(http_request,":");
	strcat(http_request,port_name);
	strcat(http_request,"\r\nConnection: Keep-Alive\r\n\r\n");

	printf("Hostname: %s\n",ipad_name);
	printf("Portname: %s\n",port_name);
	printf("FileName: %s\n",file_name);
	//send this get to server
	if (send(sockfd,(char*)http_request,sizeof http_request,0) == -1){
		perror("send");
		exit(1);
	}

	FILE *fp = fopen("output","wb");



	// if ((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) == -1) {
	//     perror("recv");
	//     exit(1);
	// }
	// Recive file
	int headflag = 0;
	char* pos;
	
	while(1){
		memset(buf,0,MAXDATASIZE);
		if ((numbytes = recv(sockfd, buf, MAXDATASIZE, 0)) == -1) {
			fclose(fp);
		//break;
	    	perror("recv");
	    	exit(1);
		}
		
		else if (numbytes == 0){
			
			break;
		}else if (headflag == 0 && numbytes > 0){
			headflag = 1;
			pos = strstr(buf,"\r") + 4;
			printf("12%s\n",pos);
			fwrite(pos,1,strlen(pos),fp);
			printf("3%s\n",buf);
			//memset(buf,0,MAXDATASIZE);
		}else if (headflag == 1 && numbytes > 0){
			fwrite(buf,1,numbytes,fp);
			printf("q%s\n",buf);
		}
		
	}


	//printf("client: received '%s'\n",buf);
	fclose(fp);
	close(sockfd);

	return 0;
}

