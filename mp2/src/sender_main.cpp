/* 
 * File:   sender_main.c
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
#include <sys/stat.h>
#include <signal.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <math.h>
#include <iostream>
#include <netdb.h>
#include <queue>
#include <deque>

using std::deque;
#define MSS 4000
//#define MSS 10
#define BUFFERSIZE 4096
#define RTT 25*1000
#define MAX_CW 130


struct timeval tv_out;
struct sockaddr_in si_other;
//struct addrinfo hints,*servinfo;
typedef struct{
    int 	length,pack_id;
    char    arr[MSS];
}pack_struct;

typedef struct{
    double 	window_size;
    int head_id;
}window_struct;

window_struct* cwindow =new window_struct;

char buffer[sizeof(pack_struct)];
deque<pack_struct*> pkt_q;
deque<pack_struct*> pkt_q_copy;
int cur_pack;
unsigned long long int bytes_count;

int SLOWSTART_CW=MAX_CW;

int s;
socklen_t slen;
int state=0;
int window_mode=0; //0 means slow start, 1 means congestion avoidance
int last_ack=0;
int last_ack_num=0;
int ack_all_flag=0;
int read_all_flag;
int package_id;
void diep(char *s) {
    perror(s);
    exit(1);
}

void dividepacket(FILE *fp);
//void sender(pack_struct* arr[]);
//void send_new(pack_struct* arr[]);
void sender(FILE *fp);
void send_new(FILE *fp);
void timeout();
void wait();
// void sender(FILE *fp, unsigned long long bytesToTransfer, int socket, struct addrinfo *receiver);

void reliablyTransfer(char* hostname, unsigned short int hostUDPport, char* filename, unsigned long long int bytesToTransfer) {


    //Open the file
    //int pack_num;
    //pkt_q = deque<pack_struct*>(); // create a queue of package pointer

	/* Determine how many bytes to transfer */

    //pack_num= ceil(bytesToTransfer * 1.0 / MSS);
    
    // dividepacket(filename,bytesToTransfer);
    //int pack_num;
    //unsigned long long int bytes_count=0;

    
    bytes_count=bytesToTransfer;
    //pack_struct* arr[pack_num];
    read_all_flag=0;
    package_id=1;
    FILE *fp;
    fp = fopen(filename, "rb");
    if (fp == NULL) {
        printf("Could not open file to send.");
        exit(1);
    }    

    dividepacket(fp);

//     int i=0;
//     while(feof(fp)==0 ){
//         //printf("3\n");
//         pack_struct* pack=new pack_struct;
//         memset((char*)pack, 0, sizeof(*pack));
//         int pck_size = fread(pack->arr, sizeof(char), MSS, fp);
//         if(pck_size<=0){
//             printf("finish reading");
//             break;
//         }
//         pack->pack_id=package_id;
//         package_id+=1;
//         bytes_count+=pck_size;
//         pack->length=pck_size; 
//         // pack->type=2; //type is data
//         pkt_q.push_back(pack); //add it into the queue
//         //arr[i]=pack;
//         i=i+1;
//         if(bytes_count>=bytesToTransfer){
//             break;
//         }
//     }
//    fclose(fp);




    slen = sizeof (si_other);

    if ((s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
        diep((char*)"socket");


    //trick starts here
    memset((char *) &si_other, 0, sizeof (si_other));
    si_other.sin_family = AF_INET;
    si_other.sin_port = htons(hostUDPport);
    if (inet_aton(hostname, &si_other.sin_addr) == 0) {
        fprintf(stderr, "inet_aton() failed\n");
        exit(1);
    }
    // clock_t startTime = clock(); 

	/* Send data and receive acknowledgements on s*/
    sender(fp);
    pack_struct* fin = new pack_struct;
    fin->pack_id = 0;
    fin->length = 0;
    if (sendto(s, fin, sizeof(pack_struct), 0, (struct sockaddr *)&si_other, sizeof(si_other)) < 0) {
	    perror("sendto failed");
    }
    //freeaddrinfo(servinfo);
    printf("Closing the socket\n");
    fclose(fp);
    close(s);
 
    return;

}

void dividepacket(FILE* fp){
    int pack_num=300;
    //unsigned long long int bytes_count=0;
    //int package_id=0;
    //pack_num= ceil(bytesToTransfer * 1.0 / MSS);// packages number need to hold the whole file
    
    while(bytes_count>0 ){
        if(pack_num==0){
            break;//already 300 packages load,send first
        }
        int pck_size = bytes_count < MSS ? bytes_count : MSS;
        pack_struct* pack=new pack_struct;
        memset((char*)pack, 0, sizeof(*pack));
        int i = fread(pack->arr, sizeof(char), MSS, fp);
        if(i>0){
            pack->pack_id=package_id;
            package_id+=1;
            bytes_count-=pck_size;
            pack->length=pck_size;      
            pkt_q.push_back(pack); //add it into the queue      
        }
        // pack->type=2; //type is data
        pack_num=pack_num-1;
        // if(bytes_count<bytes_count){
        //     break;
        // }
    }
    if(bytes_count==0){
        read_all_flag=1;
    }
}

void sender(FILE *fp){

    //int pack_num= ceil(bytesToTransfer * 1.0 / MSS);
    cur_pack=1;
    // memcpy(buffer, pkt_q->front(), sizeof(pack_struct));
    // pkt_q_copy.push_back(pkt_q->front());
    // pkt_q.pop_front();
     //intialize congestion window as size 1
    cwindow->window_size=16.0;
    cwindow->head_id=1;
    //clock_t startTime = clock(); 
    while(ack_all_flag==0){
        //printf("%d\n",state);
        switch(state){
            case 0:
                send_new(fp);
                break;
            case 1:
                timeout();
                break;
            case 2:
                wait();
                //printf("win%d\n",cwindow->window_size);
                break;

        }

        // if (pkt_q.empty()==True){
        //     break;
        // }
    }
}

void send_new(FILE *fp){
    int have_send_num = pkt_q_copy.size();
    //int current=cur_pack;
    // no pakage left
    if (pkt_q.empty()==1){
        if(read_all_flag==1){
            state=2;//go to wait for all ack
            return;            
        }
        dividepacket(fp);

    }    
    
    for (int i=0; i<int(cwindow->window_size)-have_send_num;i++){
        //start send package one by one
        if (pkt_q.empty()==1){
            state=2;//go to wait for all ack
            return;

        }  

        if (sendto(s, pkt_q.front(), sizeof(pack_struct), 0, (struct sockaddr *)&si_other, sizeof(si_other)) < 0) {
	        perror("send_error");
        }
        pkt_q_copy.push_back(pkt_q.front());
        pkt_q.pop_front();
        
        //cur_pack=cur_pack+1;
    }
    state=2;//go to wait
}
void timeout(){
    if (sendto(s, pkt_q_copy.front(), sizeof(pack_struct), 0, (struct sockaddr *)&si_other, sizeof(si_other)) < 0) {
	    perror("send_error");
    }
    
    state=2;//go to wait    
}

void wait(){
    int k;
    int ack_struct;
    tv_out.tv_sec = 0;
    tv_out.tv_usec = 25 * 1000;
    //set time out mode
    k = setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &tv_out, sizeof(tv_out));
    if(k == -1){
        diep((char*)"setsockopt_error");
    }
    //int numbytes = recvfrom(s, &ack_struct, sizeof(unsigned int) , 0,NULL, NULL);
    int numbytes = recvfrom(s, &ack_struct, sizeof(unsigned int) , 0, (struct sockaddr *)&si_other, &slen);
    //recieve nothing case
    if(numbytes<0){
        state = 1;
        if(errno==EAGAIN){
            
            //SLOWSTART_CW=cwindow->window_size/2; //congestion avoidance
            cwindow->window_size=cwindow->window_size/2;
            cwindow->head_id=pkt_q_copy.front()->pack_id;
            window_mode=0;
            return;

        }
        else{
            diep((char*)"EAGAIN");
        }
    }


    //check ack content
    if(ack_struct==last_ack){
        last_ack_num=last_ack_num+1;
        //dupack condition
        if(last_ack_num==3){ 
            //for speed
            //cwindow->window_size=cwindow->window_size/2;
            cwindow->head_id=pkt_q_copy.front()->pack_id;
            //window_mode=1;   
            state=1;         
            return;
        }
    }
    else{
        last_ack=ack_struct;
        last_ack_num=1; //update new ack
    }
    


    if(pkt_q_copy.front()->pack_id > ack_struct){
        state=2; //
    }

    //window
    else if(pkt_q_copy.front()->pack_id == ack_struct){
        pkt_q_copy.pop_front();
        if(pkt_q.empty()==1 && read_all_flag==1){//no more window shift
            if(pkt_q_copy.empty()==1){
                //all ack
                ack_all_flag=1; //all end
            }
        }
        else{//set to send more mode because window should shift
            state=0;
        }

        int window_position = ack_struct-cwindow->head_id;
        if(cwindow->window_size-1==window_position){
            if(window_mode==0){
                cwindow->head_id = cwindow->head_id + cwindow->window_size ;
                if (2*cwindow->window_size <= SLOWSTART_CW){
                    cwindow->window_size=cwindow->window_size*2;
                }
                else if(cwindow->window_size<SLOWSTART_CW){
                    //fast recovery
                    cwindow->window_size = SLOWSTART_CW;
                }
                else{
                    cwindow->window_size=cwindow->window_size+1;
                }

            }
            else{
                // all packages in window are ack
                cwindow->head_id = cwindow->head_id + cwindow->window_size ;
                cwindow->window_size=cwindow->window_size+1;
            }
        }

    }
    else{
        for(int j=0;j<ack_struct-cwindow->head_id;j++){
            pkt_q_copy.pop_front();
        }
        //(pkt_q_copy.front()->pack_id < ack_struct) impossible maybe
        //state=1; //just resend 
    }


}


// void wait(){
//     int k;
//     int ack_struct;
//     tv_out.tv_sec = 0;
//     tv_out.tv_usec = 25 * 1000;
//     //set time out mode
//     k = setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &tv_out, sizeof(tv_out));
//     if(k == -1){
//         diep((char*)"setsockopt_error");
//     }
//     //int numbytes = recvfrom(s, &ack_struct, sizeof(unsigned int) , 0,NULL, NULL);
//     int numbytes = recvfrom(s, &ack_struct, sizeof(unsigned int) , 0, (struct sockaddr *)&si_other, &slen);
//     //recieve nothing case
//     if(numbytes<0){
//         state = 1;
//         if(errno==EAGAIN){
            
//             //SLOWSTART_CW=cwindow->window_size/2; //congestion avoidance
//             cwindow->window_size=cwindow->window_size/2;
//             cwindow->head_id=pkt_q_copy.front()->pack_id;
//             window_mode=0;
//             return;

//         }
//         else{
//             diep((char*)"EAGAIN");
//         }
//     }
//     //congestion avoidance


//     //check ack content
//     if(ack_struct==last_ack){
//         last_ack_num=last_ack_num+1;
//         //dupack condition
//         if(last_ack_num==3){ 
//             //for speed
//             cwindow->window_size=SLOWSTART_CW;
//             cwindow->head_id=pkt_q_copy.front()->pack_id;
//             window_mode=1;   
//             state=1;         
//             return;
//         }
//         cwindow->window_size=cwindow->window_size+1;
//     }

//     //new ack comes
//     else if(ack_struct>last_ack){
//         last_ack=ack_struct;
//         last_ack_num=1; //update new ack
//         if(window_mode==0){
//             cwindow->window_size = (cwindow->window_size+1.0 >= 200) ? 199 : cwindow->window_size+1.0;
//             //cwindow->window_size+=1.0;
//         }
//         else{
//             cwindow->window_size = (cwindow->window_size+1.0/cwindow->window_size >= 200) ? 199 : cwindow->window_size+1.0/cwindow->window_size;
//             //cwindow->window_size= cwindow->window_size+ 1.0/ cwindow->window_size;
//         }        
//     }
    
//     if(cwindow->window_size>SLOWSTART_CW){
//         window_mode=1;
//     }

//     if(pkt_q_copy.front()->pack_id > ack_struct){
//         state=2; //
//     }

//     //window head found
//     else if(pkt_q_copy.front()->pack_id <= ack_struct){
//         //printf("2\n");
//         for(int j=0;j<=ack_struct-cwindow->head_id;j++){
//             pkt_q_copy.pop_front();
//         }
//         //printf("2\n");
//         //pkt_q_copy.pop_front();
//         if(pkt_q.empty()==1 && read_all_flag==1){//no more window shift
//             if(pkt_q_copy.empty()==1){
//                 //all ack
//                 ack_all_flag=1; //all end
//             }
//             return;
//         }
//         else{//set to send more mode because window should shift
//             state=0;
//         }
//         //printf("2\n");
//         //int window_position = ack_struct-cwindow->head_id;
//         //window is full,then shift window
//         cwindow->head_id = ack_struct+1;
//         if(window_mode==0){
//             printf("2\n");
            
//             //cwindow->head_id + cwindow->window_size ;
//             if (2*cwindow->window_size <= SLOWSTART_CW){
//                 cwindow->window_size=cwindow->window_size*2;
//             }
//             else if(cwindow->window_size<SLOWSTART_CW){
//                 //fast recovery
//                 cwindow->window_size = SLOWSTART_CW;
//             }
//             else{
//                 cwindow->window_size=cwindow->window_size+1;
//             }

//         }
        
//         //window mode 1
//         // else{
//         //     // all packages in window are ack
//         //     cwindow->head_id =ack_struct+1;
//         //     cwindow->window_size=cwindow->window_size+1;
//         // }
//     }


//     // else{
//     //     for(int j=0;j<ack_struct-pkt_q_copy.front()->pack_id;j++){
//     //         pkt_q_copy.pop_front();
//     //     }

//     //     //(pkt_q_copy.front()->pack_id < ack_struct) impossible maybe
//     //     //state=1; //just resend 
//     // }
// }




/*
 * 
 */
int main(int argc, char** argv) {

    unsigned short int udpPort;
    unsigned long long int numBytes;

    if (argc != 5) {
        fprintf(stderr, "usage: %s receiver_hostname receiver_port filename_to_xfer bytes_to_xfer\n\n", argv[0]);
        exit(1);
    }
    udpPort = (unsigned short int) atoi(argv[2]);
    numBytes = atoll(argv[4]);



    reliablyTransfer(argv[1], udpPort, argv[3], numBytes);


    return (EXIT_SUCCESS);
}

