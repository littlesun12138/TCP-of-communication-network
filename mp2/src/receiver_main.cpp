/* 
 * File:   receiver_main.c
 * Author: 
 *
 * Created on
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <pthread.h>
#include <netdb.h>
// #define MSS 2048
#define MSS 10
#define BUFFSIZE 4096


typedef struct{
    int 	length,pack_id;
                        
    char    arr[MSS];
}pack_struct;
struct sockaddr_in si_me, si_other;
int s;
socklen_t slen;
int old_pid;
int zero_pid;




void diep(char *s) {
    perror(s);
    exit(1);
}



void reliablyReceive(unsigned short int myUDPport, char* destinationFile) {
    //address problem

    slen = sizeof (si_other);
    int bytes;
    char buffer[BUFFSIZE];
    zero_pid=0;
    old_pid=0;
    if ((s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
        diep((char*)"socket");

    memset((char *) &si_me, 0, sizeof (si_me));
    si_me.sin_family = AF_INET;
    si_me.sin_port = htons(myUDPport);
    si_me.sin_addr.s_addr = htonl(INADDR_ANY);
    printf("Now binding\n");
    if (bind(s, (struct sockaddr*) &si_me, sizeof (si_me)) == -1)
        diep((char*)"bind");


	/* Now receive data and send acknowledgements */    
    FILE* fp;
    fp = fopen(destinationFile, "wb");
    if(fp==NULL){
        diep((char*)"can't open file");
    }
    int ack_array[500];
    pack_struct* pck_array[500];
    for (int i=0;i<500;i++){
        ack_array[i]=0;
        pck_array[i]=nullptr;
    }

    while(1){
        printf("1\n");
        if ((bytes = recvfrom(s, buffer, BUFFSIZE, 0,(struct sockaddr *)&si_other, &slen)) == -1) {
            diep((char*)"recvfrom error");
        }

        pack_struct* packet= new pack_struct;
        memcpy(packet, buffer, sizeof(pack_struct));
        int pid=packet->pack_id;
        if (bytes==0 || pid==zero_pid){
            break;
            //fin FIN
        }



        if(pid!=old_pid+1){
            if(ack_array[pid-1]==0){
                ack_array[pid-1]=1;
                pck_array[pid-1]=packet;
            }
            //send old ack information
            if(sendto(s, &old_pid, sizeof(old_pid), 0, (struct sockaddr *)&si_other, slen) == -1){
                perror("send_error1");
            }
            continue;
        }



        old_pid=pid;
        ack_array[pid-1]=1;
        pck_array[pid-1]=packet;
        //problem
        // if(sendto(s, &old_pid, sizeof(old_pid), 0, (struct sockaddr *)&aaddr, alen) == -1){
        //     perror("send error");
        // }
        int count=0;
        for(int i=old_pid-1;i<500;i++){
            if(ack_array[i]==1){
                count++;
            }
            else{
                break;
            }
        }
        pid=old_pid+count-1;
        //change ack and send
        
        if(sendto(s, &pid, sizeof(pid), 0, (struct sockaddr *)&si_other, slen) == -1){
            printf("2\n");
            perror("send error2");
        }        
        for(int i=old_pid-1;i<old_pid-1+count;i++){

            size_t bit = fwrite(pck_array[i]->arr,sizeof(char),pck_array[i]->length,fp);
            if(bit!=pck_array[i]->length){
                diep((char*)"fwrite error");
            }
        }

        
    }


    fclose(fp);
    close(s);
	printf("%s received.", destinationFile);
    return;
}

/*
 * 
 */
int main(int argc, char** argv) {

    unsigned short int udpPort;

    if (argc != 3) {
        fprintf(stderr, "usage: %s UDP_port filename_to_write\n\n", argv[0]);
        exit(1);
    }

    udpPort = (unsigned short int) atoi(argv[1]);

    reliablyReceive(udpPort, argv[2]);
}
