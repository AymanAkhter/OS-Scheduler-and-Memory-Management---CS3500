#include <bits/stdc++.h>
#include <fstream>
#include <chrono>
// #include <ctime.h>
using namespace std;
#define int long long

struct Process{
    bool exists;
    int id;
    int startQueue;
    int queueNum;
    int arrivalTime;
    int burstTime;
    int timeInQueue;
    int endTime;
    int enterTime;
};

ifstream in;
ofstream out;
double Time = 0;
int RRShiftTime;
int Q,T;
int totalTurnaround=0;
int lastTime=0;

void printProcess(Process &cur)
{
    // cout << "Hello\n";
    out << "ID: " << cur.id << ", " << "Start Queue: " << cur.startQueue << ", " << "End Queue: " <<
    cur.queueNum << ", " << "EndTime: " << Time << ", " << "TAT: " << (Time - cur.arrivalTime) << "\n";
    totalTurnaround += (Time-cur.arrivalTime);
}


class CompareQ{
    public:
        bool operator()(Process p1, Process p2){
                if(p1.enterTime!=p2.enterTime)
                    return p1.enterTime > p2.enterTime;
                return p1.id > p2.id;
        }
};

class FCFSQueue{
    private:
        priority_queue<Process, vector<Process>, CompareQ> q;
    public:
        void insertProcess(Process p){
            q.push(p);
        }
        int queueSize(){
            return q.size();
        }
        Process getTop(){
            return q.top();
        }
        void removeTop(){
            q.pop();
        }
        Process runProcess(){
            Process cur = q.top();
            q.pop();
            Process returnProcess;
            returnProcess.exists=false;
            int runDuration = cur.burstTime;
            Time+=runDuration;
            cur.burstTime-=runDuration;
            printProcess(cur);
            return returnProcess;
        }
};

class CompareSJF{
    public:
        bool operator()(Process p1, Process p2){
                if(p1.burstTime!=p2.burstTime)
                    return p1.burstTime>p2.burstTime;
                return p1.id > p2.id;
        }
};

class SJFQueue{
    private:
        priority_queue<Process, vector<Process>, CompareSJF> pq;
    public:
        void insertProcess(Process p){
            pq.push(p);
        }
        int queueSize(){
            return pq.size();
        }
        Process getTop(){
            return pq.top();
        }
        void removeTop(){
            pq.pop();
        }
        Process runProcess(){
            Process cur = pq.top();
            pq.pop();
            Process returnProcess;
            returnProcess.exists=false;
            int runDuration = cur.burstTime;
            Time+=runDuration;
            cur.burstTime-=runDuration;
            printProcess(cur);
            return returnProcess;
        }
};

class RRQueue{
    private:
        priority_queue<Process, vector<Process>, CompareQ> q;
    public:
        void insertProcess(Process p){
            q.push(p);
        }
        int queueSize(){
            return q.size();
        }
        Process runProcess(){
            Process cur = q.top();
            q.pop();
            Process returnProcess;
            returnProcess.exists=false;
            int runDuration = min(cur.burstTime,RRShiftTime);
            Time+=runDuration;
            cur.burstTime-=runDuration;
            if(cur.burstTime!=0)
            {
                returnProcess=cur;
                returnProcess.enterTime=Time;
            }
            else
            {
                printProcess(cur);
            }
            return returnProcess;
        }
        
};

// bool compEnter(Process p1, Process p2)
// {
//     if(p1.enterTime!=p2.enterTime)
//          return (p1.enterTime < p2.enterTime);
//     return p1.id < p2.id;
// }

class MLFS{
     
    private :

        vector<Process> processList;
        int processCntr;
        FCFSQueue Q1;
        SJFQueue Q2;
        SJFQueue Q3;
        RRQueue Q4;

    public : 
        MLFS (vector<Process> inputProcessList)
        {
            processList = inputProcessList;
            processCntr = 0;
        }

        void pushToQueue(Process p)
        {
            if(p.queueNum == 1)
                Q1.insertProcess(p);
            else if(p.queueNum == 2)
                Q2.insertProcess(p);
            else if(p.queueNum == 3)
                Q3.insertProcess(p);
            else if(p.queueNum == 4)
                Q4.insertProcess(p);
        }

        void pushUpProcesses()
        {
             vector<Process> push4, push3, push2;
             vector<Process> temp3, temp2, temp1;
             while(!Q3.queueSize()==0)
             {
                Process cur = Q3.getTop();
                Q3.removeTop();
                if(Time-cur.enterTime>=T){
                    cur.enterTime = cur.enterTime+T;
                    cur.queueNum = 4;
                    push4.push_back(cur);
                }
                else{
                    temp3.push_back(cur);
                }
             }
             while(!Q2.queueSize()==0)
             {
                Process cur = Q2.getTop();
                Q2.removeTop();
                if(Time-cur.enterTime>=2*T){
                    cur.enterTime = cur.enterTime+2*T;
                    cur.queueNum=4;
                    push4.push_back(cur);
                }
                else if(Time-cur.enterTime>=T){
                    cur.enterTime = cur.enterTime+T;
                    cur.queueNum=3;
                    push3.push_back(cur);
                }
                else{
                    temp2.push_back(cur);
                }
            }
            while(!Q1.queueSize()==0)
            {
                Process cur = Q1.getTop();
                Q1.removeTop();
                if(Time-cur.enterTime>=3*T){
                    cur.enterTime = cur.enterTime+3*T;
                    cur.queueNum=4;
                    push4.push_back(cur);
                }
                else if(Time-cur.enterTime>=2*T){
                    cur.enterTime = cur.enterTime+2*T;
                    cur.queueNum=3;
                    push3.push_back(cur);
                }
                else if(Time-cur.enterTime>=T){
                    cur.enterTime = cur.enterTime+T;
                    cur.queueNum=2;
                    push2.push_back(cur);
                }
                else{
                    temp1.push_back(cur);
                }
            }
            // sort(push4.begin(), push4.end(), compEnter);
            for(int i=0; i<push4.size(); i++)
                Q4.insertProcess(push4[i]);
            for(int i=0; i<temp3.size(); i++)
                Q3.insertProcess(temp3[i]);
            for(int i=0; i<push3.size(); i++)
                Q3.insertProcess(push3[i]);
            for(int i=0; i<temp2.size(); i++)
                Q2.insertProcess(temp2[i]);
            for(int i=0; i<push2.size(); i++)
                Q2.insertProcess(push2[i]);
            for(int i=0; i<temp1.size(); i++)
                Q1.insertProcess(temp1[i]);

        }

        void insertUptoCurrent(){
            while(processCntr<(int)processList.size() && Time >= processList[processCntr].arrivalTime){
                Process curProcess = processList[processCntr];
                curProcess.enterTime = curProcess.arrivalTime;
                pushToQueue(curProcess);
                processCntr++;
            }
        }
        
        void simulateProcesses(){
            scheduleAll();
        } 

        void scheduleAll(){

            while((Q1.queueSize()!=0) || (Q2.queueSize()!=0) || (Q3.queueSize()!=0) || (Q4.queueSize()!=0) || (processCntr<(int)processList.size()))
            {
                insertUptoCurrent();
                pushUpProcesses();
                Process notCompleted;
                notCompleted.exists=false;
                if(Q4.queueSize()!=0)
                    notCompleted = Q4.runProcess();

                else if(Q3.queueSize()!=0)
                    notCompleted = Q3.runProcess();
                    
                else if(Q2.queueSize()!=0)
                    notCompleted = Q2.runProcess();
                
                else if(Q1.queueSize()!=0)
                    notCompleted = Q1.runProcess();
                    
                else{
                    if(processCntr<(int)processList.size())
                    {
                        pushToQueue(processList[processCntr]);   
                        Time = processList[processCntr].arrivalTime;
                        processCntr++;
                    }
                    else
                        return;
                }
                if(notCompleted.exists)
                {
                    insertUptoCurrent();
                    pushUpProcesses();
                    pushToQueue(notCompleted);
                }
            }  
        }      
};

bool CompArrive(Process p1, Process p2)
{
    return p1.arrivalTime < p2.arrivalTime;
}

int32_t main(int32_t argc, char *argv[]){

    Q = atoi(argv[1]);
    T = atoi(argv[2]);
    string F = argv[3];
    string P = argv[4];

    RRShiftTime = Q;
    vector<Process> processList;
    in.open(F);
    out.open(P);
    string line;
    int numberOfProcesses=0;
    while(!in.eof())
    {
        Process p;
        int num1, num2, num3, num4;
        in >> p.id >> p.queueNum >> p.arrivalTime >> p.burstTime;
        p.exists=true;
        p.timeInQueue = T;
        p.startQueue = p.queueNum;
        processList.push_back(p);
        numberOfProcesses++;
    }

    auto start_time = std::chrono::high_resolution_clock::now();

    sort(processList.begin(), processList.end(), CompArrive);
    int n = numberOfProcesses;
    MLFS myScheduler(processList);
    myScheduler.simulateProcesses();
    auto end_time = std::chrono::high_resolution_clock::now();
    double mayank = totalTurnaround;
    double mayankprocesses = 1000*n;
    out << "Mean Turnaround Time: " << fixed << setprecision(2) << (mayank)/n << "(ms), Throughput: " << mayankprocesses/Time << " processes/sec\n";
    in.close();
    out.close();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    std::cout << "Runtime: " << duration.count() << " ms" << std::endl;
}