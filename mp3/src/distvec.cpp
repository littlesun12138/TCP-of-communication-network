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
// typedef unordered_map<int, pair<int, int>> entry;
// typedef unordered_map<int, entry> forward;
// unordered_map<int, unordered_map<int, pair<int, int> > forward_table;
// typedef unordered_map<int, forward_table> big_table;

set<int> iter_node;
void init_top();
void read_top(ifstream &topfile);
void change_top(int start,int end, int cost);
void dv_method( unordered_map<int, unordered_map<int, pair<int, int>>> *ft, unordered_map<int, unordered_map<int, pair<int, int>>> *neigborft,ofstream &fpOut);

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
    int src;
    int des;
    int cost;

    while(topfile >> src >> des >> cost){
        iter_node.insert(src);
        iter_node.insert(des);
        p[des][src] = cost;
        p[src][des] = cost;
    }
    noend_num = iter_node.size();
}
void change_top(int start,int end, int cost){
    p[start][end] = p[end][start] = cost;
}

void dv_method(unordered_map<int, unordered_map<int, pair<int, int>>> *ft, unordered_map< int, unordered_map<int, pair<int, int>> > *neigborft,ofstream &fpOut){
    for(auto i=iter_node.begin();i!= iter_node.end();i++){
        for(auto j=iter_node.begin();j!= iter_node.end();j++){
            (*ft)[*i][*j]=make_pair(*j,p[*i][*j]);
            (*neigborft)[*i][*j]=make_pair(*j,p[*i][*j]);
        }
    }
    // cout << "999" << (*ft)[1][1].second << endl;
    for(int i=0;i<noend_num;i++){
        for(auto a=iter_node.begin();a!= iter_node.end();a++){
            for(auto b=iter_node.begin();b!= iter_node.end();b++){
                for(auto c=iter_node.begin();c!= iter_node.end();c++){
                    //neigbor nodes are in these nodes
                    if((*neigborft)[*a][*c].second>=0 && *a != *c){
                        //neighbor nodes found
                        if((*ft)[*c][*b].second>=0){

                            //the path through node c works
                            if((*ft)[*a][*b].second==0){
                            }
                            else if((*ft)[*a][*b].second<0){
                                (*ft)[*a][*b].second=(*neigborft)[*a][*c].second+(*ft)[*c][*b].second;
                                (*ft)[*a][*b].first=*c;
                            }
                            else if((*neigborft)[*a][*c].second+(*ft)[*c][*b].second<(*ft)[*a][*b].second){
                                (*ft)[*a][*b].second=(*neigborft)[*a][*c].second+(*ft)[*c][*b].second;
                                (*ft)[*a][*b].first=*c;
                            }
                            //tie breaking condition
                            else if((*neigborft)[*a][*c].second+(*ft)[*c][*b].second==(*ft)[*a][*b].second ){
                                if (*c<(*ft)[*a][*b].first){
                                    (*ft)[*a][*b].second=(*neigborft)[*a][*c].second+(*ft)[*c][*b].second;
                                    (*ft)[*a][*b].first=*c;
                                }
                            }

                        }
                        //cout << "999" << " "<<*a << " "<<*b<< " "<<(*ft)[*a][*b].second << endl;
                    }
                }
            }
        
        }
    }

    //print all nodes
    for(auto a=iter_node.begin();a!= iter_node.end();a++){
            for(auto b=iter_node.begin();b!= iter_node.end();b++){
                if ((*ft)[*a][*b].second >=0) {
                    fpOut << *b << " " << (*ft)[*a][*b].first << " " << (*ft)[*a][*b].second << endl;
                }
            }
            //fpOut<<endl;
    }
    //print message

}

int main(int argc, char** argv) {
    //printf("Number of arguments: %d", argc);
    if (argc != 4) {
        printf("Usage: ./distvec topofile messagefile changesfile\n");
        return -1;
    }

    int start_id;
    int end_id;

    unordered_map<int, unordered_map<int, pair<int, int>>> ft;
    unordered_map<int, unordered_map<int, pair<int, int>>> neigborft;
    // unordered_map<int,pair<int,int>> ftable[MAX_NODE_VAL];
    // unordered_map<int,pair<int,int>> ctable[MAX_NODE_VAL];
    ofstream fpOut("output.txt");
    //Read topofile
    std::ifstream topfile(argv[1]);
    //Read msg file
    std::ifstream msgfile(argv[2]);
    //Read change file
    std::ifstream chgfile(argv[3]);
    // printf("090");
    //Get top graph
    init_top();
    read_top(topfile);
    //printf("090");
    // forward_table ft[MAX_NODE_VAL];//forward table
    // forward_table neigborft[MAX_NODE_VAL];    
    // for(auto a=iter_node.begin();a!= iter_node.end();a++){
    //     for(auto b=iter_node.begin();b!= iter_node.end();b++){
    //         ft[*a][*b]=make_pair(*b,p[*a][*b]);
    //         neigborft[*a][*b]=make_pair(*b,p[*a][*b]);
    //     }
        
    // }

    dv_method(&ft,&neigborft,fpOut);

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
        int cost = ft[msg_vector[i].start_id][msg_vector[i].end_id].second;

        if(cost>0){
            // cout << ft[msg_vector[i].start_id][msg_vector[i].end_id].first << endl;
            // cout << ft[msg_vector[i].start_id][msg_vector[i].end_id].second << endl;
            fpOut<<cost<<" hops ";
            for(int tem = msg_vector[i].start_id;tem!=msg_vector[i].end_id;){
                fpOut<<tem<<" ";
                tem = ft[tem][msg_vector[i].end_id].first;
                
            }
        }else if (cost==0)
        {   

            fpOut<<"0"<<" hops ";
        }else{
            fpOut<<"infinite hops unreachable ";
        }

        fpOut<<"message"<<" "<<msg_vector[i].extra_info<<endl;
        
    }
    //fpOut<<endl;

    
    int new_val;
    while(chgfile >> start_id >> end_id >> new_val){

        change_top(start_id,end_id,new_val);

        //clear original ft
        ft.clear();
        neigborft.clear();
        dv_method(&ft,&neigborft,fpOut);

        //Send msg again
        for(int i=0;i<msg_vector.size();i++){
            fpOut << "from" << " " << msg_vector[i].start_id << " " << "to" << " " << msg_vector[i].end_id << " cost ";
            int cost = ft[msg_vector[i].start_id][msg_vector[i].end_id].second;
            if(cost>0){
                fpOut<<cost<<" hops ";
                for(int tem = msg_vector[i].start_id;tem!=msg_vector[i].end_id;){
                    fpOut<<tem<<" ";
                    tem = ft[tem][msg_vector[i].end_id].first;
                }
            }else if (cost==0)
            {
                fpOut<<"0"<<" hops ";
            }else{
                fpOut<<"infinite hops unreachable ";
            }
            fpOut<<"message"<<" "<<msg_vector[i].extra_info<<endl;
            
        }
    //fpOut<<endl;
    }    

    fpOut.close();

    return 0;
}

