#include<stdio.h>
#include<string.h>
#include<stdlib.h>

#include <iostream>
#include <cstring>
#include <fstream>
#include <sstream>
#include <vector>
#include <set>
#include <unordered_map>
#include <map>

using namespace std;


#define VAL_INF  -999
#define INF      99999
#define MAX_NODE 30
#define MAX_NODE_VAL 31
#define VISITED 1
#define NOT_VIS 0
//Define new structure for topo file

//Adj mat
int p[MAX_NODE_VAL][MAX_NODE_VAL];
//True noend num
int noend_num=0;

//Define new structure for msg file
typedef struct message {
    int start_id;
    int end_id;
    string extra_info;

    message(int start_id, int end_id, string extra_info) : 
    start_id(start_id), end_id(end_id), extra_info(extra_info) {}
} msgg;
vector<message> msg_vector;

set<int> iter_node;
void init_top();
void read_top(ifstream &topfile);
void change_top(int start,int end, int cost);

void init_top(){
    for(int i = 1; i < MAX_NODE; ++i){
        for(int j = 1; j < MAX_NODE; ++j){
            if(i==j){
                p[i][j] = 0;
            }else{
                p[i][j] = VAL_INF;
            }
        }
    }
}

void read_top(ifstream &topfile){
    int start;
    int end;
    int cost;

    while(topfile >> start >> end >> cost){
        iter_node.insert(start);
        iter_node.insert(end);
        p[end][start] = cost;
        p[start][end] = cost;
    }
    noend_num = iter_node.size();
}
void dj_foward_table(unordered_map<int,pair<int,int>> *ftable,unordered_map<int,pair<int,int>> *ctable,ofstream &fpOut){

    int noend_num_val = noend_num + 1;
    int *vis = new int [noend_num_val]; // 0 means not vistied
    int start,end,min_cost,min_node;
    // Init Cost table
    for(auto a=iter_node.begin();a!= iter_node.end();a++){
        for(auto b=iter_node.begin();b!= iter_node.end();b++){
            ctable[*a][*b] = make_pair(*a,p[*a][*b]);
        }
    }

    for(auto i=iter_node.begin();i!=iter_node.end();i++){
        start = *i;

        memset(vis, 0, noend_num_val*sizeof(int));
        //Set start node value 
        vis[start] = VISITED;
        min_node = start;
        min_cost = 0;
        //Tranverse all neighbor noend
        for(int k = 1;k < noend_num; k++ ){
            for(auto l=iter_node.begin();l != iter_node.end(); l++){
                end = *l;
                int new_cost = min_cost + p[min_node][end];
                int old_cost = ctable[start][end].second;

                if( (vis[end]==NOT_VIS) && (p[min_node][end] >=0) && (new_cost<old_cost || old_cost<0)){
                    ctable[start][end] = make_pair(min_node,new_cost);
                }

                new_cost = min_cost + p[min_node][end];
                old_cost = ctable[start][end].second;
                if( (vis[end]==NOT_VIS) && (p[min_node][end] >=0) && (new_cost<old_cost || old_cost<0)){
                    if(min_node < ctable[start][end].first){
                        ctable[start][end] = make_pair(min_node,new_cost);
                    }
                }
            }
            min_cost = INF;
            for(auto l=iter_node.begin();l != iter_node.end(); l++){
                end = *l;
                int temp_cost = ctable[start][end].second;
                if(temp_cost<min_cost && temp_cost>=0 && vis[end]==0){
                    min_node = end;
                    min_cost = temp_cost;
                }
            }
            vis[min_node] = VISITED;
        }
        ftable[start] = ctable[start];
        //Backtrace paths and print txt
        for(auto l=iter_node.begin();l != iter_node.end(); l++){
                end = *l;
                int next_hop = end;
                if(ftable[start][end].second >= 0){
                    for(next_hop = end;ctable[start][next_hop].first != start;){
                        next_hop = ctable[start][next_hop].first;
                    }
                    ftable[start][end].first = next_hop;
                    int cost = ftable[start][end].second;
                    fpOut << end << " " << next_hop << " " << cost << endl;
                }
            }
            fpOut<<endl;
    }
}
void change_top(int start,int end, int cost){
    p[start][end] = p[end][start] = cost;
}

int main(int argc, char** argv) {
    //printf("Number of arguments: %d", argc);
    if (argc != 4) {
        printf("Usage: ./linkstate topofile messagefile changesfile\n");
        return -1;
    }
    int start_id;
    int end_id;
    unordered_map<int,pair<int,int>> ftable[MAX_NODE_VAL];
    unordered_map<int,pair<int,int>> ctable[MAX_NODE_VAL];

    ofstream fpOut("output.txt");
    //Read topofile
    std::ifstream topfile(argv[1]);
    //Read msg file
    std::ifstream msgfile(argv[2]);
    //Read change file
    std::ifstream chgfile(argv[3]);
    
    //Get top graph
    init_top();
    read_top(topfile);
    

    //DJ now!
    dj_foward_table(ftable,ctable,fpOut);

    //Now Faward tables send out 
    //Send message from message file

    //Get msg file to vectors
    if (msgfile.is_open()) {
        //Read line by line
        std::string line;
        while(getline(msgfile, line)) {
            string extra_msg;
            sscanf(line.c_str(),"%d %d %*s", &start_id,&end_id);
            string temp = line.substr(line.find(" "));
            extra_msg = temp.substr(line.find(" ") + 1); 
            msgg new_line_msg(start_id,end_id,extra_msg);
            // printf("%s", line.c_str());
            msg_vector.push_back(new_line_msg);
        }
        msgfile.close();
    }
    
    for(int i=0;i<msg_vector.size();i++){
        fpOut << "from" << " " << msg_vector[i].start_id << " " << "to" << " " << msg_vector[i].end_id << " cost ";
        int cost = ftable[msg_vector[i].start_id][msg_vector[i].end_id].second;
        if(cost>0){
            fpOut<<cost<<" hops ";
            for(int tem = msg_vector[i].start_id;tem!=msg_vector[i].end_id;){
                fpOut<<tem<<" ";
                tem = ftable[tem][msg_vector[i].end_id].first;
            }
        }else if (cost==0)
        {
            fpOut<<"0"<<" hops ";
        }else{
            fpOut<<"infinite hops unreachable ";
        }
        fpOut<<"message"<<" "<<msg_vector[i].extra_info<<endl;
        
    }
    fpOut<<endl;

    //Now change topo
    int new_val;
    while(chgfile >> start_id >> end_id >> new_val){
        change_top(start_id,end_id,new_val);
        //DJ each change
        dj_foward_table(ftable,ctable,fpOut);

        //Send msg again
        for(int i=0;i<msg_vector.size();i++){
        fpOut << "from" << " " << msg_vector[i].start_id << " " << "to" << " " << msg_vector[i].end_id << " cost ";
        int cost = ftable[msg_vector[i].start_id][msg_vector[i].end_id].second;
        if(cost>0){
            fpOut<<cost<<" hops ";
            for(int tem = msg_vector[i].start_id;tem!=msg_vector[i].end_id;){
                fpOut<<tem<<" ";
                tem = ftable[tem][msg_vector[i].end_id].first;
            }
        }else if (cost==0)
        {
            fpOut<<"0"<<" hops ";
        }else{
            fpOut<<"infinite hops unreachable ";
        }
        fpOut<<"message"<<" "<<msg_vector[i].extra_info<<endl;
        
        }
    fpOut<<endl;
    }

    //Done!
    fpOut.close();

    return 0;
}

