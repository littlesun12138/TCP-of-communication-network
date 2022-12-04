#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<iostream>
#include<sstream>
#include<fstream>
#include<set>
#include<vector>
using namespace std;

int N;
int L;
int R;
int M;
int T;
vector<int> Rarray;
vector<struct node> NodeArray;
char tmp;
int channelOccupied =0; 

typedef struct node
{
    int node_id;
    int collisions=0;
    int backoff=0;   
}node_t;

void set_backoff(node_t *p,vector<int>& Rarr,int t){
    int temp = t+p->node_id;
    int rvv = Rarr[p->collisions]; 
    p->backoff = temp % rvv;
}



float toy_simulator(){
    //cout << "dsfsdfs" << endl;
    int transmitime_left;
    int cur_node_id=0;//zero goes into transmission at first by generator
    int ready_node_num=0;
    vector<int> collision_array;
    int slots_num=0;
    //Init 
    for (int i = 0; i < N; i++) {
        node_t temp_node;
        temp_node.node_id=i;
        set_backoff(&temp_node,Rarray,0);
        NodeArray.push_back(temp_node);
    }
    //large time ticks loop
    for (int i=0; i<T; i++) {
        cout << i <<  endl;
        //check whether channel is idle
        if(channelOccupied==0){
            ready_node_num=0;
            collision_array.clear();            
            for(int j =0;j<NodeArray.size();j++){
                if(NodeArray[j].backoff==0){
                    ready_node_num=ready_node_num+1;
                    collision_array.push_back(j);
                }
            }
            if(ready_node_num==0){
                //cout <<  "1111" << endl;
                for(int j =0;j<NodeArray.size();j++){
                    NodeArray[j].backoff=NodeArray[j].backoff-1;
                }
                //cout <<  "000" << endl;
            }
            else if(ready_node_num==1){
                //cout <<  "2222" << endl;
                transmitime_left=L;
                int number=collision_array[0];
                //NodeArray[number].backoff=NodeArray[number].backoff-1;
                cur_node_id=number;
                channelOccupied =1;
                slots_num=slots_num+1;
                transmitime_left=transmitime_left-1;
                if(transmitime_left==0){
                    // only one sec
                    channelOccupied =0;
                    set_backoff(&NodeArray[cur_node_id],Rarray,i+1);
                }
                //cout <<  "000" << endl;
            }
            else{
                //cout <<  "3333" << endl;
                //collision happens

                for(int k =0;k<ready_node_num;k++){
                    NodeArray[collision_array[k]].collisions=NodeArray[collision_array[k]].collisions+1;
                    //When a node’s collision count reaches a maximum value of M, the node gives up and drops this packet.
                    if(NodeArray[collision_array[k]].collisions==M){
                        NodeArray[collision_array[k]].collisions=0;
                        //need a new backoff or not?
                        set_backoff(&NodeArray[collision_array[k]],Rarray,i+1);
                    }
                    else{
                        // other collision nodes also needs new backoff time
                        set_backoff(&NodeArray[collision_array[k]],Rarray,i+1);
                    }
                }
                //cout <<  "000" << endl;
            }
        }
        else{
            slots_num=slots_num+1;
            transmitime_left=transmitime_left-1;
            if(transmitime_left==0){
                channelOccupied=0;
                set_backoff(&NodeArray[cur_node_id],Rarray,i+1);
            } 

        }
    }
    return (slots_num*1.00/T);
}


int main(int argc, char** argv) {
    //printf("Number of arguments: %d", argc);


    if (argc != 2) {
        printf("Usage: ./csma input.txt\n");
        return -1;
    }
    //Read File

    ifstream f(argv[1]);
    f >> tmp >> N >> tmp >> L >> tmp >> M >> tmp;

    while (f.get() != '\n') {
        f >> R;
        Rarray.push_back(R);
    }
    f >> tmp >> T;
    //Done Read File

    float link_utilization_rate=toy_simulator();
    
    FILE* fpOut;
    fpOut = fopen("output.txt", "w");
    if(fpOut!=NULL){
        fprintf(fpOut, "%.2f", link_utilization_rate);
        //fpOut << link_utilization_rate*1.00<< endl;
    }
    fclose(fpOut);


    return 0;
}
