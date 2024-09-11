#include <bits/stdc++.h>
using namespace std;

//Global declaration for all memory, their pages etc.

vector<int> MainMemory;
vector<int> VirtualMemory;
vector<int> MainMemoryPages;
vector<int> VirtualMemoryPages;
int MainMemorySize;
int VirtualMemorySize;
int PageSize;
int numMainPages;
int numVirtualPages;
int cur_pid=1;

//Declare input and output objects
ifstream in;
ifstream inProc;
ofstream out;
ofstream pteOut;

int INF = 1e6;

//Process Class - stores the instructions and memory, contains methods to run the instructions

class Process{
    private :
        string name;
        vector<vector<string>> instructionList;
        int processSize;
        int pid;

        map<int,int> pageTable;
        int inPhysical;
    public :

        Process(string n, vector<vector<string>> instructions, int procSiz, int pidNum){
            name = n;
            instructionList = instructions;
            processSize = procSiz;
            pid = pidNum;
        }

        int getPhysicalPage(int ProcessPageNum){
            return pageTable[ProcessPageNum];
        }

        int getPhysicalAddr(int processAddr){
            int physicalPageNum = processAddr/PageSize;
            int offset = processAddr%PageSize;
            return PageSize*physicalPageNum + offset;
        }

        void load(int value, int destProcessAddr){
            int destPhysicalAddr = getPhysicalAddr(destProcessAddr);
            MainMemory[destPhysicalAddr] = value;
            out << "Command: load " << destProcessAddr << "; Result: Value of " << value << " is now stored in addr " << 
            destProcessAddr << '\n';
        }

        void add(int op1ProcessAddr, int op2ProcessAddr, int destProcessAddr){
            int op1PhysicalAddr = getPhysicalAddr(op1ProcessAddr);
            int op2PhysicalAddr = getPhysicalAddr(op2ProcessAddr);
            int destPhysicalAddr = getPhysicalAddr(destProcessAddr);
            int sum = MainMemory[op1PhysicalAddr] + MainMemory[op2PhysicalAddr];
            MainMemory[destPhysicalAddr] = sum;
            out << "Command: add " << op1ProcessAddr << ", " << op2ProcessAddr  << ", " << destProcessAddr << "; Result: Value in addr " <<
            op1ProcessAddr << " = " << MainMemory[op1PhysicalAddr] << ", addr " << op2ProcessAddr << " = " << 
            MainMemory[op2PhysicalAddr] << ", addr " << destProcessAddr << " = " << sum << '\n';

        }

        void sub(int op1ProcessAddr, int op2ProcessAddr, int destProcessAddr){
            int op1PhysicalAddr = getPhysicalAddr(op1ProcessAddr);
            int op2PhysicalAddr = getPhysicalAddr(op2ProcessAddr);
            int destPhysicalAddr = getPhysicalAddr(destProcessAddr);
            int diff = MainMemory[op1PhysicalAddr] - MainMemory[op2PhysicalAddr];
            MainMemory[destPhysicalAddr] = diff;
            out << "Command: sub " << op1ProcessAddr << ", " << op2ProcessAddr  << ", " << destProcessAddr << "; Result: Value in addr " <<
            op1ProcessAddr << " = " << MainMemory[op1PhysicalAddr] << ", addr " << op2ProcessAddr << " = " << 
            MainMemory[op2PhysicalAddr] << ", addr " << destProcessAddr << " = " << diff << '\n';
        }

        void print(int processAddr){
            int physicalAddr = getPhysicalAddr(processAddr);
            int value = MainMemory[physicalAddr];
            out << "Command: print " << processAddr << "; Result: Value in addr " <<
            processAddr << " = " << value << "\n";
        }

        int executeInstruction(vector<string> instruction){
            string instructionType = instruction[0];
            if(instructionType == "add"){
                int op1Addr = stoi(instruction[1]);
                int op2Addr = stoi(instruction[2]);
                int destAddr = stoi(instruction[3]);
                // if(op1Addr >= processSize || op2Addr >= processSize || destAddr >= processSize)
                if(destAddr >= processSize*1024 || op1Addr >= processSize*1024 || op2Addr >= 1024*processSize){
                    out << "Invalid Memory Access";
                    return -1;
                }
                add(op1Addr, op2Addr, destAddr);
            }
            else if(instructionType == "sub"){
                int op1Addr = stoi(instruction[1]);
                int op2Addr = stoi(instruction[2]);
                int destAddr = stoi(instruction[3]);
                if(destAddr >= processSize*1024 || op1Addr >= processSize*1024 || op2Addr >= 1024*processSize){
                    out << "Invalid Memory Access";
                    return -1;
                }
                sub(op1Addr, op2Addr, destAddr);
            }
            else if(instructionType == "print"){
                int opAddr = stoi(instruction[1]);
                if(opAddr >= processSize*1024){
                    out << "Invalid Memory Access";
                    return -1;
                }
                print(opAddr);
            }
            else if(instructionType == "load"){
                int value = stoi(instruction[1]);
                int destAddr = stoi(instruction[2]);
                if(destAddr >= processSize*1024){
                    out << "Invalid Memory Access";
                    return -1;
                }
                load(value,destAddr);
            }
            return 1;
        }

        void runProcess(){
            out << "Running Process " << pid << "...\n";
            for(auto instruction : instructionList)
            {
                out << '\t';
                if(executeInstruction(instruction)==-1){
                    out << " - Killing process\n";
                    return;
                }
            }
        }
        
        int getSize(){
            return processSize;
        }

        int getID(){
            return pid;
        }

        string getName(){
            return name;
        }

        vector<vector<string>> getInstructions(){
            return instructionList;
        }

        void setState(int state){
            inPhysical = state;
        }

        void addEntry(int processPage, int physicalPage){
            pageTable[processPage] = physicalPage;
        }

        void printPageTable(){
            time_t now = time(0);
            char* dt = ctime(&now);
            pteOut << "Data and Time is " << dt;
            out << "Printing Page Table of Process " << pid;
            pteOut << "Printing Page Table of Process " << pid;
            if(inPhysical==0){
                pteOut << " - in Main Memory\n";
                out << " - in Main Memory\n";
            }
                
            else{
                pteOut << " - in Virtual Memory\n";
                out << " - in Virtual Memory\n";
            }
                
            for(auto entry : pageTable){
                pteOut << "\tVPN " << entry.first << " - PageFrame " << entry.second << "\n";
            }
            pteOut << '\n';
        }
};

//Lists to store processes

vector<Process> MainProcessList;
vector<Process> VirtualProcessList;

//Comparator for LRU set

class Compare{
    public:
        bool operator()(pair<int,int> p1, pair<int,int> p2){
                if(p1.first != p2.first)
                    return p1.first > p2.first;
                return p1.second > p2.second;
        }
};

// Memory Management Unit - Takes care of all the system commands, transfer and allocation
// of memory for processes

class MMU{

    private : 
        int MainMemoryLeft;
        int VirtualMemoryLeft;
        set<pair<int,int>> LRU;
        int LRU_cnt=1;
        
    public :
        MMU (int M, int V, int P){
            MainMemorySize = M;
            VirtualMemorySize = V;
            PageSize = P;
            numMainPages = M * 1024 / P; 
            numVirtualPages = V * 1024 / P; 
            MainMemoryPages.assign(numMainPages, 0);
            VirtualMemoryPages.assign(numVirtualPages, 0);
            MainMemory.assign(M*1024,0);
            VirtualMemory.assign(V*1024,0);
            MainMemoryLeft = M;
            VirtualMemoryLeft = V;
        }

        void loadInMain(Process p){
            int processPages = p.getSize() * 1024 / PageSize;
            int pageCnt = 0;
            for(int i=0; i<numMainPages; i++){
                if(MainMemoryPages[i]==0){
                    MainMemoryPages[i] = p.getID();
                    p.addEntry(pageCnt++,i);
                    if(pageCnt==processPages)
                        break;
                }
            }
            p.setState(0);
            MainProcessList.push_back(p);
            MainMemoryLeft -= p.getSize();
            LRU.insert({INF,p.getID()});
            out << p.getName() << " is loaded in main memory and is assigned process id " << p.getID() << '\n';
            
        }

        void loadInVirtual(Process p){
            int processPages = p.getSize() * 1024 / PageSize;
            int pageCnt = 0;
            for(int i=0; i<numVirtualPages; i++){
                if(VirtualMemoryPages[i]==0){
                    VirtualMemoryPages[i] = p.getID();
                    // processPages--;
                    p.addEntry(pageCnt++,i);
                    if(pageCnt==processPages)
                        break;
                }
            }
            p.setState(1);
            VirtualProcessList.push_back(p);
            VirtualMemoryLeft -= p.getSize();
            out << p.getName() << " is loaded in virtual memory and is assigned process id " << p.getID() << '\n';
        }

        void load(Process p){
            if(MainMemoryLeft > p.getSize()){
                loadInMain(p);
            }
            else if(VirtualMemoryLeft > p.getSize()){
                loadInVirtual(p);
            }
            else{
                out << p.getName() << " could not be loaded - memory is full\n";
                cur_pid--;
            }
        }

        int location(int pid){
            for(auto proc : MainProcessList){
                if(proc.getID()==pid)
                    return 0;
            }
                
            for(auto proc : VirtualProcessList){
                if(proc.getID()==pid)
                    return 1;
            }
                
            return -1;
        }

        void run(int pid){
            if(location(pid)==-1){
                out << "pid " << pid << " could not be run - process does not exist\n";
            }
            if(location(pid)==1)
                swapin(pid);
            for(auto &proc : MainProcessList){
                if(proc.getID()==pid){
                    proc.runProcess();
                }
            }
            pair<int,int> remPair;
            for(auto itr : LRU){
                if(itr.second==pid){
                    remPair=itr;
                    break;
                }
            }
            LRU.erase(remPair);
            LRU.insert({LRU_cnt++,pid});
        }

        void kill(int pid){
            
            if(location(pid)==-1){
                out << "pid " << pid << " could not be run - process does not exist\n";
            }
            if(location(pid)==0){
                for(int i=0; i<numMainPages; i++){
                    if(MainMemoryPages[i]==pid){
                        MainMemoryPages[i]=0;
                        for(int j=PageSize*i; j<PageSize*i + PageSize; j++){
                            MainMemory[j]=0;
                        }   
                    }   
                }
                for(auto itr = MainProcessList.begin(); itr!=MainProcessList.end(); itr++){
                    if((*itr).getID() == pid){
                        MainProcessList.erase(itr);
                        MainMemoryLeft+=(*itr).getSize();
                        break;
                    }
                }
            }
            else if(location(pid)==1){
                for(int i=0; i<numVirtualPages; i++){
                    if(VirtualMemoryPages[i]==pid){
                        VirtualMemoryPages[i]=0;
                        for(int j=PageSize*i; j<PageSize*i + PageSize; j++){
                            VirtualMemory[j]=0;
                        }
                    }
                }
                for(auto itr = VirtualProcessList.begin(); itr!=VirtualProcessList.end(); itr++){
                    if((*itr).getID() == pid){
                        VirtualProcessList.erase(itr);
                        VirtualMemoryLeft+=(*itr).getSize();
                        break;
                    }
                }
            }
        }

        void listpr(){
            vector<int> mainProcessIds;
            vector<int> virtualProcessIds;
            for(auto itr : MainProcessList){
                mainProcessIds.push_back(itr.getID());
            }
            for(auto itr : VirtualProcessList){
                virtualProcessIds.push_back(itr.getID());
            }
            sort(mainProcessIds.begin(), mainProcessIds.end());
            sort(virtualProcessIds.begin(), virtualProcessIds.end());
            out << "Main Memory Processes: ";
            for(auto id : mainProcessIds){
                out << id << ' ';
            }
            out << '\n';
            out << "Virtual Memory Processes: ";
            for(auto id : virtualProcessIds){
                out << id << ' ';
            }
            out << '\n';
        }

        void copyPageFromMain(int src, int dst){
            for(int i=0; i<PageSize; i++){
                MainMemory[src+i] = VirtualMemory[dst+i];
            }
        }

        void copyPageFromVirtual(int src, int dst){
            for(int i=0; i<PageSize; i++){
                VirtualMemory[src+i] = MainMemory[dst+i];
            }
        }

        void copyFromMain(int pid){

            vector<int> cpyAddrs;
            for(int i=0; i<numMainPages; i++){
                if(MainMemoryPages[i]==pid){
                    MainMemoryPages[i]=0;
                    cpyAddrs.push_back(i);
                }
            }

            int procSiz; 
            string procName;
            vector<vector<string>> instructions;

            for(auto itr = MainProcessList.begin(); itr!=MainProcessList.end(); itr++){
                if((*itr).getID() == pid){  
                    procSiz = (*itr).getSize();
                    procName = (*itr).getName();
                    instructions = (*itr).getInstructions();
                    MainMemoryLeft+=(*itr).getSize();
                    MainProcessList.erase(itr);
                    break;
                }
            }

            Process temp(procName, instructions, procSiz, pid);
            int processPages = cpyAddrs.size();
            int cpyCnt = 0;
            for(int i=0; i<numVirtualPages; i++){
                if(VirtualMemoryPages[i]==0){
                    VirtualMemoryPages[i] = pid;
                    copyPageFromMain(PageSize*cpyAddrs[cpyCnt],PageSize*i);
                    temp.addEntry(cpyCnt++,i);
                    if(cpyCnt==processPages)
                        break;
                }
            }
            temp.setState(1);
            VirtualProcessList.push_back(temp);
            VirtualMemoryLeft -= temp.getSize();

        }

        void copyFromVirtual(int pid){
            vector<int> cpyAddrs;
            for(int i=0; i<numVirtualPages; i++){
                if(VirtualMemoryPages[i]==pid){
                    VirtualMemoryPages[i]=0;
                    cpyAddrs.push_back(i);
                }
            }

            int procSiz; 
            string procName;
            vector<vector<string>> instructions;

            for(auto itr = VirtualProcessList.begin(); itr!=VirtualProcessList.end(); itr++){
                if((*itr).getID() == pid){ 
                    procSiz = (*itr).getSize();
                    procName = (*itr).getName();
                    instructions = (*itr).getInstructions();
                    VirtualMemoryLeft+=(*itr).getSize();
                    VirtualProcessList.erase(itr);
                    break;
                }
            }

            Process temp(procName, instructions, procSiz, pid);
            int processPages = cpyAddrs.size();
            int cpyCnt = 0;
            for(int i=0; i<numMainPages; i++){
                if(MainMemoryPages[i]==0){
                    MainMemoryPages[i] = pid;
                    copyPageFromVirtual(PageSize*cpyAddrs[cpyCnt],PageSize*i);
                    temp.addEntry(cpyCnt++,i);
                    if(cpyCnt==processPages)
                        break;
                }
            }
            temp.setState(0);
            MainProcessList.push_back(temp);
            MainMemoryLeft -= temp.getSize();
        }

        void swapin(int pid){
            if(location(pid)!=1){
                out << "pid " << pid << " could not be run - process does not exist\n";
                return;
            }

            string n;
            vector<vector<string>> instructions;
            int procSiz;
            for(auto itr : VirtualProcessList){
                if(itr.getID()==pid){
                    n = itr.getName();
                    instructions = itr.getInstructions();
                    procSiz = itr.getSize();
                }    
            }

            while(MainMemoryLeft<procSiz){
                swapout((*LRU.begin()).second);
            }

            copyFromVirtual(pid);
            out << "pid " << pid << " swapped to Main Memory\n";
            LRU.insert({INF,pid});
        }

        void swapout(int pid){
            if(location(pid)!=0){
                out << "pid " << pid << " could not be run - process does not exist\n";
                return;
            }

            copyFromMain(pid);
            out << "pid " << pid << " swapped to Virtual Memory\n";
            pair<int,int> remPair;
            for(auto itr : LRU){
                if(itr.second==pid){
                    remPair=itr;
                    break;
                }
            }
            LRU.erase(remPair);
        }

        void print(int start, int len){
            for(int i=0; i<len; i++){
                out << "Value of " << start+i << ": " << MainMemory[start+i] << '\n';
            }
        }

        void pte(int pid, string fileName){
            
            if(location(pid)==0){
                for(auto itr : MainProcessList){
                    if((itr).getID()==pid){
                        pteOut.open(fileName,ios::app);
                        itr.printPageTable();
                    }
                        
                }
                pteOut.close();
            }
            else if(location(pid)==1){
                for(auto itr : VirtualProcessList){
                    if((itr).getID()==pid){
                        pteOut.open(fileName,ios::app);
                        itr.printPageTable();
                    }
                        
                }
                pteOut.close();
            }
        }

        void pteall(string fileName){
            vector<int> pids;
            for(auto proc : MainProcessList){
                pids.push_back(proc.getID());
            }
            for(auto proc : VirtualProcessList){
                pids.push_back(proc.getID());
            }
            sort(pids.begin(), pids.end());
            for(auto itr : pids){
                pte(itr,fileName);
            }
        }
};

int getNextChar(string &s, int i)
{
    for(int j=i; j<s.length(); j++){
        if(s[j]!=' ')
            return j;
    }
    return -1;
}

vector<string> splitLine(string command)
{
    command.push_back(' ');
    // cout << command << '\n';
    // for(auto itr : command){
    //     cout << (int)itr << ' ';
    // }
    // cout << '\n';
    int prev = 0;
    vector<string> args;
    prev = getNextChar(command,0);
    int i = prev;
    for(; i<command.length(); i++){
        if(command[i]==' ' || command[i]=='\r'){
            args.push_back(command.substr(prev, i-prev));
            prev = getNextChar(command,i);
            i = prev;
            // i--;
            if(prev==-1)
                break;
            if(command[i]=='\r')
                break;
        }
    }
    return args;
}

Process createProcess(string fileName, int pidNum)
{
    int procSize;
    string temp;
    getline(inProc, temp);
    vector<string> tempVec = splitLine(temp);
    procSize = stoi(tempVec[0]);
    string cmd;
    vector<vector<string>> instructions;
    
    while(!inProc.eof())
    {
        getline(inProc,cmd);
        vector<string> instructionArgs = splitLine(cmd);
        for(auto &arg : instructionArgs){
            if(*arg.rbegin() == ',')
                arg.pop_back();
        }
        instructions.push_back(instructionArgs);
    }
    Process newProc(fileName, instructions, procSize, pidNum);
    return newProc;
}


int main(int argc, char * argv[]){

    int M,V,P;
    string inputFile, outputFile;
    for(int i=1; i<=10; i++){
        if(argv[i][0]=='-'){

            if(argv[i][1]=='M'){
                M = atoi(argv[i+1]);
            }
            else if(argv[i][1]=='V'){
                V = atoi(argv[i+1]); 
            }
            else if(argv[i][1]=='P'){
                P = atoi(argv[i+1]);
            }
            else if(argv[i][1]=='i'){
                inputFile = argv[i+1]; 
            }
            else if(argv[i][1]=='o'){
                outputFile = argv[i+1];
            }
            i++;
        }
    }

    MMU myMMU(M,V,P);
    string commandLine;
    
    in.open(inputFile);
    out.open(outputFile);

    while(!in.eof()){
        getline(in, commandLine);
        vector<string> arguments = splitLine(commandLine);
        
        string command = arguments[0];
        // cout << command << '\n';
        // cout << command.size() << '\n';
        // for(auto itr : command){
        //     cout << (int)itr << ' ';
        // }
        // cout << '\n';
        if(command == "load"){
            for(int i=1; i<arguments.size(); i++){
                
                string fileName = arguments[i];
                inProc.open(fileName);
                if(!inProc){
                    out << fileName << " could not be loaded - file does not exist\n";
                    cur_pid--;
                    continue;
                }
                
                Process p = createProcess(fileName, cur_pid++); 
                inProc.close();
                myMMU.load(p);  
            }
        }
        else if(command == "run"){
            int pid = stoi(arguments[1]);
            myMMU.run(pid);
        }
        else if(command == "kill"){
            int pid = stoi(arguments[1]);
            myMMU.kill(pid);
        }
        else if(command == "listpr"){
            // cout << "INLISTPR\n";
            myMMU.listpr();
        }
        else if(command == "pte"){
            int pid = stoi(arguments[1]);
            string fileName = arguments[2];
            myMMU.pte(pid, fileName);
        }
        else if(command == "pteall"){
            string fileName = arguments[1];
            myMMU.pteall(fileName);
        }
        else if(command == "swapout"){
            int pid = stoi(arguments[1]);
            myMMU.swapout(pid);
        }
        else if(command == "swapin"){
            int pid = stoi(arguments[1]);
            myMMU.swapin(pid);
        }
        else if(command == "print"){
            int start = stoi(arguments[1]);
            int len = stoi(arguments[2]);
            myMMU.print(start, len);
        }
        else if(command == "exit"){
            exit(0);
        }
        else{
            cout << "Invalid Command\n";
            // cout << command << '\n';
            exit(0);
        }
    }
    return 0;
}
