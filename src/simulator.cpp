#include "Poisson.h"
#include "Generator.h"
#include <bits/stdc++.h>
using namespace std;

int reqIdGen = 0;

class Request
{
    int timestamp;
    int forwardingTimestamp;
    int finishedTimestamp;
    int reqSize;
    int respSize; // bytes
    int pendingSize;
    int sentBy; // server number; -1 for dispatcher
    int reqId;  // autogenerated incrementing

public:
    Request(int timestamp, int reqSize, int sentBy)
    {
        reqId = reqIdGen++;
        this->timestamp = timestamp;
        this->reqSize = reqSize;
        respSize = rand() % 100;
        pendingSize = respSize;
        this->sentBy = sentBy;
    }

    int getReqId()
    {
        return reqId;
    }

    int getRespSize()
    {
        return respSize;
    }

    int getPendingSize()
    {
        return pendingSize;
    }

    void updatePendingSize(int bytesProcessed)
    {
        pendingSize = pendingSize - bytesProcessed;
    }

    int getSentBy(){
        return sentBy;
    }

    void updateSentBy(int resentBy){
        sentBy = resentBy;
    }

    int getForwardingTimestamp(){
        return forwardingTimestamp;
    }

    void updateForwardingTimestamp(int forwardingTimestamp){
        this->forwardingTimestamp = forwardingTimestamp;
    }

    int getFinishedTimestamp(){
        return finishedTimestamp;
    }

    void updateFinishedTimestamp(int finishedTimestamp){
        this->finishedTimestamp = finishedTimestamp;
    }
};

class Server
{
    long alpha, totalRespSize, totalReqs, totalRespBytesProcessed, totalReqsProcessed;
    int server_no;
    std::queue<Request> reqQueue, processedReqQueue;
    double avgRespSize;

public:
    Server(long alpha, int server_no)
    {
        // Initializations
        totalRespSize = 0;
        totalReqs = 0;
        totalRespBytesProcessed = 0;
        this->alpha = alpha;
        this->server_no = server_no;
        cout<< "\tServer #" << server_no << " | alpha : "<< alpha << endl;
    }

    queue<Request> getReqQueue()
    {
        return reqQueue;
    }

    queue<Request> getProcessedReqQueue()
    {
        return processedReqQueue;
    }

    int getAlpha()
    {
        return alpha;
    }

    void addRequest(Request request)
    {
        reqQueue.push(request);
        totalReqs++;
        totalRespSize += request.getRespSize();
        avgRespSize = totalRespSize / totalReqs;
    }

    long getPendingRequestCount(){
        return getReqQueue().size();
    }

    long getPendingRequestSize(){
        long numRequests = getPendingRequestCount();
        long pendingReqSize = 0;
        while(numRequests--){
            Request &cur = reqQueue.front();
            pendingReqSize += cur.getPendingSize();
            reqQueue.pop();
            reqQueue.push(cur);
        }
        return pendingReqSize;
    }

    long getTotalProcessedBytes(){
        return totalRespBytesProcessed;
    }

    long getNumProcessedRequests(){
        if(!reqQueue.empty()){
            Request &top = reqQueue.front();
            if(top.getPendingSize() != top.getRespSize()){
                return 1 + processedReqQueue.size();
            }
        }
        return processedReqQueue.size();
    }

    bool whenPolicy(int policyNum, int timeDelta, Server *servers, int server_count){
        /*
        Use the when policy to determine whether to forward any request(s)
        */
       switch (policyNum)
       {
       case 0:
           return true;
           break;
       
       default:
           break;
       }
    }

    vector<Request> whatPolicy(int policyNum, int timeDelta, Server *servers, int server_count){
        /*
        Use the what policy to determine which request(s) to forward
        */
        vector<Request> requestsToBeForwarded;
        // Go thru all the requests
        long numRequests = getPendingRequestCount();
        while(numRequests--){
            // Get the request
            Request &cur = reqQueue.front();
            // Dont consider partially processed/ forwarded requests
            if(cur.getRespSize() == cur.getPendingSize() && cur.getSentBy() == -1){
                // Apply the policy
                switch (policyNum){
                    case 0:
                        // forward the ones whose size > avg
                        if(cur.getRespSize() > avgRespSize){
                            requestsToBeForwarded.push_back(cur);
                        }
                        break;
                    
                    default:
                        break;
                }
            }
            // Add to the back of the queue
            reqQueue.pop();
            reqQueue.push(cur);
        }
    }

    int wherePolicy(int policyNum, int timeDelta, Server *servers, int server_count, Request requestToBeForwarded){
        /*
        Use the where policy to send to the appropriate server
        */
        int send_to = server_no;                // Use this to determine whom to send the request to
        long least_load;                        // Use this to store the load of the server chosen
        vector<int> randomly_selected_servers;  // Use this for Power of k
        int k=2;                                // Use this to play with Power of k
        switch (policyNum){
        case 0:
            /* next server */
            send_to = (server_no+1)%server_count;
            least_load = servers[send_to].getPendingRequestSize();
            break;
        case 1:
            /* least loaded */
            least_load = servers[(server_no+1)%server_count].getPendingRequestSize();
            send_to = (server_no+1)%server_count;
            for(int i=0; i<server_count; i++){
                if(i != server_no){
                    long load = servers[i].getPendingRequestSize(); 
                    if(load<least_load){
                        least_load = load;
                        send_to = i;
                    }
                }
            }
            break;
        case 2:
            /* Power of k */
            least_load = LONG_MAX;
            int randomly_selected_server;
            for(int i=0; i<k; i++){
                randomly_selected_server = rand() % server_count;
                // Regenerate if redundant
                while(randomly_selected_server == server_no || (find(randomly_selected_servers.begin(), randomly_selected_servers.end(), randomly_selected_server) != randomly_selected_servers.end())){
                    randomly_selected_server = rand() % server_count;
                }
                randomly_selected_servers.push_back(randomly_selected_server);
                long load = servers[randomly_selected_server].getPendingRequestSize();
                if(load<least_load){
                    least_load = load;
                    send_to = randomly_selected_server;
                }
            }
            break;
        default:
            break;
        }
        // Sanity check so as to not forward requests to other servers with higher load
        if(least_load > getPendingRequestSize()){
            return -1;
        }
        return send_to;
    }

    void removeRequest(Request requestToBeRemoved){
        long numRequests = getPendingRequestCount();
        while(numRequests--){
            // Get the request
            Request &cur = reqQueue.front();
            reqQueue.pop();
            // Ignore if this is the one to be removed
            if(cur.getReqId() != requestToBeRemoved.getReqId()){
                // Add to the back of the queue
                reqQueue.push(cur);
            }
        }
    }

    void forwardRequest(int currentTime, int send_to, Request requestToBeForwarded, Server *servers, int server_count){
        // purge the request fm the queue
        removeRequest(requestToBeForwarded);
        // Add the request in the reciever's queue
        // Update the relevant stats as you send stuff
        requestToBeForwarded.updateSentBy(server_no);
        requestToBeForwarded.updateForwardingTimestamp(currentTime);
        servers[send_to].addRequest(requestToBeForwarded);
    }

    void executeForwardingPipeline(int currentTime, int timeDelta, Server *servers, int server_count){
        // Execute the when, what and where policies keeping in mind the timeUnits
        int when_policy = 0;    // Use this to control the when policy
        int what_policy = 0;    // Use this to control the what policy
        int where_policy = 0;   // Use this to control the where policy
        while(whenPolicy(when_policy, timeDelta, servers, server_count)){
            // Go thru and execute the what policy till it becomes inapplicable
            vector<Request> requestsToBeForwarded = whatPolicy(what_policy, timeDelta, servers, server_count);
            // Forward each request using the where policy
            for(int i=0; i<requestsToBeForwarded.size(); i++){
                int send_to = wherePolicy(where_policy, timeDelta, servers, server_count, requestsToBeForwarded[i]);
                if(send_to != server_no && send_to != -1){
                    forwardRequest(currentTime, send_to, requestsToBeForwarded[i], servers, server_count);
                }
            }
        }
    }

    void processData(int currentTime, int timeDelta, Server *servers, int server_count)
    {
        int maxBytes = timeDelta * alpha;
        // Conduct normal execution on this server
        cout << "\t\tServer #" << server_no << " will process " << maxBytes << " bytes in " << timeDelta << " time units" << endl;
        while (!reqQueue.empty() && maxBytes > 0)
        {
            Request &cur = reqQueue.front();
            int pendingSize = cur.getPendingSize();
            if (pendingSize > maxBytes)
            {
                cout << "\t\t\tServer #" << server_no << " processed " << maxBytes << " / " << cur.getRespSize() << " bytes of response for request #"<< cur.getReqId() << endl;
                // update
                cur.updatePendingSize(maxBytes);
                maxBytes -= maxBytes;
                totalRespBytesProcessed += maxBytes;
            }
            else
            {
                // update
                cur.updatePendingSize(pendingSize);
                cout << "\t\t\tServer #" << server_no << " processed " << (cur.getRespSize() - cur.getPendingSize()) << " / " << cur.getRespSize() << " bytes of response for request #"<< cur.getReqId() << endl;
                reqQueue.pop();
                cur.updateFinishedTimestamp(currentTime);
                processedReqQueue.push(cur);
                maxBytes -= pendingSize;
                totalRespBytesProcessed += pendingSize;
            }
        }
    }
};

int main(int argc, char **argv)
{
    // Initializations
    int maxSimulationTime = 5;
    int time = 0;
    int reqId = 0;
    int server_count = 5;
    int alpha = 50;
    Server *servers[server_count];
    cout << "Simulation parameters: "<< endl << "Simulation time: " << maxSimulationTime << endl << "Number of servers: "<< server_count << endl;
    Poisson p = Poisson(1.0 / 2.0);
    for (int i = 0; i < server_count; i++)
    {
        servers[i] = new Server(alpha, i);
    }
    // Iteration
    cout << endl << "----SIMULATION BEGINS----" << endl << endl;
    while (time < maxSimulationTime)
    {
        int t = 0, nextTimeDelta = (int)p.generate();
        cout << "\tTime elapsed " << time << " time units" << endl;
        cout << "\tNext request arrives in " << nextTimeDelta << " time units" << endl;
        cout << "\tCreating the current request" << endl;
        Request request = Request(time, 1, -1);
        cout << "\tCurrent response size = " << request.getRespSize() << endl;
        int nextServer = rand() % server_count;
        cout << "\tMapping the request on to server #" << nextServer << endl;
        (*servers[nextServer]).addRequest(request);
        while((t++ < nextTimeDelta) && (time+t <= maxSimulationTime)){
            // Execute policies to forward packets via RDMA
            for(int i = 0; i < server_count; i++){
                (*servers[i]).executeForwardingPipeline(time, 1, *servers, server_count);
            }
            // Process the requests on each server till the next request comes in
            for (int i = 0; i < server_count; i++)
            {
                (*servers[i]).processData(time, 1, *servers, server_count);
            }
            time++;
        }
        cout << endl;
    }
    cout << "----SIMULATION ENDS----" << endl << endl;
    // compute and print statistics
    cout << "----STATISTICS----" << endl << endl;
    cout << "Per server" << endl;
    long totalPendingRespSize = 0, totalPendingReqsCount = 0, totalBytesProcessed = 0, totalRequestsProcessed = 0;
    for (int i = 0; i < server_count; i++){
        // Get the data
        long pendingReqsCount = (*servers[i]).getPendingRequestCount();
        long pendingReqsSize = (*servers[i]).getPendingRequestSize();
        long bytesProcessed = (*servers[i]).getTotalProcessedBytes();
        long numProcessedRequests = (*servers[i]).getNumProcessedRequests();
        // Add to cumulative
        totalPendingReqsCount += pendingReqsCount;
        totalPendingRespSize += pendingReqsSize;
        totalBytesProcessed += bytesProcessed;
        totalRequestsProcessed += numProcessedRequests;
        // Print out
        cout << "\tServer #"<< i << endl;
        cout << "\t\t # of requests processed (even partial) : " << numProcessedRequests << endl;
        cout << "\t\t Size of processed responses : " << bytesProcessed << " bytes" << endl;
        cout << "\t\t # of pending requests : " << pendingReqsCount << endl;
        cout << "\t\t Size of pending responses : " << pendingReqsSize << " bytes" << endl;
    }
    cout << endl << "Cumulative" << endl;
    cout << "Total # of requests processed (even partial) : " << totalRequestsProcessed << endl;
    cout << "Total size of processed responses : " << totalBytesProcessed << " bytes" << endl;
    cout << "Total # of pending requests : " << totalPendingReqsCount << endl;
    cout << "Total pending response size : " << totalPendingRespSize << " bytes" << endl;

    return 0;
}
