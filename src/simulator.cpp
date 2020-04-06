#include <cstdio>
#include "Poisson.h"
#include "Uniform.h"
#include "Generator.h"
#include <bits/stdc++.h>
#include "spdlog/spdlog.h"
#include "spdlog/cfg/env.h"
#include "spdlog/sinks/stdout_color_sinks.h"

using namespace std;

int reqIdGen = 0;
ofstream outputFile;
char const *serverStats = "serverStats.csv";
char const *overallStats = "overallStats.csv";


class Request
{
    int timestamp;
    int forwardingTimestamp;
    double finishedTimestamp;
    int reqSize;
    int respSize; // bytes
    int pendingSize;
    int sentBy; // server number; -1 for dispatcher
    int reqId;  // autogenerated incrementing

public:
    double generate_random_number(double low, double high){
        int random_value = rand();
        return (low+(static_cast<double>(random_value)/(static_cast<double>(RAND_MAX/(high-low)))));
    }

    Request(int timestamp, int reqSize, int sentBy)
    {
        reqId = reqIdGen++;
        this->timestamp = timestamp;
        this->reqSize = reqSize;
        respSize = (int)floor(generate_random_number(200, 500));
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

    int getTimestamp()
    {
        return timestamp;
    }

    void updatePendingSize(int bytesProcessed)
    {
        pendingSize = pendingSize - bytesProcessed;
    }

    int getSentBy()
    {
        return sentBy;
    }

    void updateSentBy(int resentBy)
    {
        sentBy = resentBy;
    }

    int getForwardingTimestamp()
    {
        return forwardingTimestamp;
    }

    void updateForwardingTimestamp(int forwardingTimestamp)
    {
        this->forwardingTimestamp = forwardingTimestamp;
    }

    double getFinishedTimestamp()
    {
        return finishedTimestamp;
    }

    void updateFinishedTimestamp(double finishedTimestamp)
    {
        this->finishedTimestamp = finishedTimestamp;
    }
};

class Server
{
    long alpha, totalRespSize, totalReqs, totalRespBytesProcessed, totalReqsProcessed, cumulativePendingCount, totalFullyProcessedBytes;
    int server_no;
    std::queue<Request> reqQueue, processedReqQueue;
    std::queue<pair<int, Request>> deferredRequests;
    double avgRespSize, utilization, totalBusyTime, avgRespTime, totalRespTime, totalWaitingTime;

public:
    Server(long alpha, int server_no)
    {
        // Initializations
        totalRespSize = 0;
        totalReqs = 0;
        totalRespBytesProcessed = 0;
        cumulativePendingCount = 0;
        utilization = 0.0;
        totalBusyTime = 0.0;
        totalRespTime = 0.0;
        totalFullyProcessedBytes = 0.0;
        this->alpha = alpha;
        this->server_no = server_no;
        spdlog::trace("\tServer #{} | alpha : {}", server_no, alpha);
    }

    queue<Request> getReqQueue()
    {
        return reqQueue;
    }

    queue<Request> getProcessedReqQueue()
    {
        return processedReqQueue;
    }

    long getAlpha()
    {
        return alpha;
    }

    double calculateUtilization(double timeElapsed)
    {
        return totalFullyProcessedBytes / (alpha * timeElapsed);
    }

    double getTotalWaitingTime() {
        return totalWaitingTime;
    }

    double getUtilization()
    {
        return utilization;
    }

    double getTotalBusyTime(){
        return totalBusyTime;
    }

    void addRequest(Request request)
    {
        reqQueue.push(request);
        totalReqs++;
        totalRespSize += request.getRespSize();
        avgRespSize = totalRespSize / totalReqs;
    }

    long getPendingRequestCount()
    {
        return getReqQueue().size();
    }

    long getPendingRequestSize()
    {
        long numRequests = getPendingRequestCount();
        long pendingReqSize = 0;
        while (numRequests--)
        {
            Request &cur = reqQueue.front();
            pendingReqSize += cur.getPendingSize();
            reqQueue.pop();
            reqQueue.push(cur);
        }
        return pendingReqSize;
    }

    void updatePendingCount(){
        cumulativePendingCount += getPendingRequestCount();
    }

    long getCumulativePendingCount(){
        return cumulativePendingCount;
    }

    long getTotalProcessedBytes()
    {
        return totalRespBytesProcessed;
    }

    double getAverageServiceRate(){
        return (double)getNumProcessedRequests()/totalBusyTime;
    }

    double getAvgRespTime() {
        return totalRespTime / processedReqQueue.size();
    }

    long getNumProcessedRequests()
    {
        return processedReqQueue.size();
    }

    bool whenPolicy(int policyNum, double currentTime, int timeDelta, Server *servers[], int server_count)
    {
        /*
        Use the when policy to determine whether to forward any request(s)
        */
        bool time_to_forward = false;
       double policy_0_threshold = 1.5;
       spdlog::trace("\t\t\tWhen policy #{}:", policyNum);
       switch (policyNum)
       {
        case -1:
            break;
        case 0:
            // Using utilization
            this->utilization = calculateUtilization(currentTime);
            spdlog::trace("\t\t\t\tServer #{} | utilization: {} | threshold: {}", server_no, utilization, policy_0_threshold);
            if(this->utilization > policy_0_threshold){
                time_to_forward = true;
            }
            break;
        default:
            break;
        }
        return time_to_forward;
    }

    vector<Request> whatPolicy(int policyNum, int timeDelta, Server *servers[], int server_count)
    {
        /*
        Use the what policy to determine which request(s) to forward
        */
        vector<Request> requestsToBeForwarded;
        // Go thru all the requests
        long numRequests = getPendingRequestCount();
        spdlog::trace("\t\t\tWhat policy #{}:", policyNum);
        spdlog::trace("\t\t\tServer #{} has average response size: {}", server_no, avgRespSize);
        while (numRequests--)
        {
            // Get the request
            Request &cur = reqQueue.front();
            spdlog::trace("\t\t\t\tConsidering RequestID: {} with response size: {} and pending size: {}", cur.getReqId(), cur.getRespSize(), cur.getPendingSize());
            // Dont consider partially processed/ forwarded requests
            if (cur.getRespSize() == cur.getPendingSize() && cur.getSentBy() == -1)
            {
                // Apply the policy
                switch (policyNum)
                {
                case 0:
                    // forward the ones whose size > avg
                    if (cur.getRespSize() > avgRespSize)
                    {
                        spdlog::trace("\t\t\t\t\tRequestID: {}  qualifies for forwarding", cur.getReqId());
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
        return requestsToBeForwarded;
    }

    int wherePolicy(int policyNum, int timeDelta, Server *servers[], int server_count, Request requestToBeForwarded)
    {
        /*
        Use the where policy to send to the appropriate server
        */
        int send_to = server_no;               // Use this to determine whom to send the request to
        long least_load;                       // Use this to store the load of the server chosen
        vector<int> randomly_selected_servers; // Use this for Power of k
        int k = 2;                             // Use this to play with Power of k
        spdlog::trace("\t\t\tWhere Policy #{} executing switch", policyNum);
        switch (policyNum)
        {
        case 0:
            /* next server */
            send_to = (server_no + 1) % server_count;
            least_load = (*servers[send_to]).getPendingRequestSize();
            break;
        case 1:
            /* least loaded */
            least_load = (*servers[(server_no + 1) % server_count]).getPendingRequestSize();
            send_to = (server_no + 1) % server_count;
            for (int i = 0; i < server_count; i++)
            {
                if (i != server_no)
                {
                    long load = (*servers[i]).getPendingRequestSize();
                    if (load < least_load)
                    {
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
            for (int i = 0; i < k; i++)
            {
                randomly_selected_server = rand() % server_count;
                // Regenerate if redundant
                while (randomly_selected_server == server_no || (find(randomly_selected_servers.begin(), randomly_selected_servers.end(), randomly_selected_server) != randomly_selected_servers.end()))
                {
                    randomly_selected_server = rand() % server_count;
                }
                randomly_selected_servers.push_back(randomly_selected_server);
                long load = (*servers[randomly_selected_server]).getPendingRequestSize();
                if (load < least_load)
                {
                    least_load = load;
                    send_to = randomly_selected_server;
                }
            }
            break;
        default:
            break;
        }
        // Sanity check so as to not forward requests to other servers with higher load
        if (least_load > getPendingRequestSize())
        {
            return -1;
        }
        spdlog::trace("\t\t\tWhere policy #{}: the least load is for {} and is equal to {} bytes", policyNum, send_to, least_load);
        return send_to;
    }

    void removeRequest(Request requestToBeRemoved)
    {
        long numRequests = getPendingRequestCount();
        while (numRequests--)
        {
            // Get the request
            Request &cur = reqQueue.front();
            reqQueue.pop();
            // Ignore if this is the one to be removed
            if (cur.getReqId() != requestToBeRemoved.getReqId())
            {
                // Add to the back of the queue
                reqQueue.push(cur);
            }
        }
    }

    void forwardRequest(int currentTime, int send_to, Request requestToBeForwarded, Server *servers[], int server_count, bool removeRequestFromQueue = true)
    {
        // purge the request fm the queue
        if (removeRequestFromQueue)
        {
            removeRequest(requestToBeForwarded);
        }
        // Add the request in the reciever's queue
        // Update the relevant stats as you send stuff
        requestToBeForwarded.updateSentBy(server_no);
        requestToBeForwarded.updateForwardingTimestamp(currentTime);
        (*servers[send_to]).addRequest(requestToBeForwarded);
    }

    void forwardDeferredRequests(int currentTime, Server *servers[], int server_count)
    {
        while (!deferredRequests.empty())
        {
            pair<int, Request> forwardingInfo = deferredRequests.front();
            deferredRequests.pop();
            forwardRequest(currentTime, forwardingInfo.first, forwardingInfo.second, servers, server_count, false);
        }
    }

    void executeForwardingPipeline(int currentTime, int timeDelta, Server *servers[], int server_count)
    {
        // Execute the when, what and where policies keeping in mind the timeUnits
        int when_policy = -1;  // Use this to control the when policy
        int what_policy = 0;  // Use this to control the what policy
        int where_policy = 0; // Use this to control the where policy
        spdlog::trace("\t\tServer #{} will execute the when policy", server_no);
        while (whenPolicy(when_policy, currentTime, timeDelta, servers, server_count))
        {
            int num_requests_forwarded = 0;
            spdlog::trace("\t\tServer #{} will execute the what policy", server_no);
            // Go thru and execute the what policy till it becomes inapplicable
            vector<Request> requestsToBeForwarded = whatPolicy(what_policy, timeDelta, servers, server_count);
            // Forward each request using the where policy
            for (int i = 0; i < requestsToBeForwarded.size(); i++)
            {
                spdlog::trace("\t\tServer #{} will execute the where policy for requestID: {}", server_no, requestsToBeForwarded[i].getReqId());
                int send_to = wherePolicy(where_policy, timeDelta, servers, server_count, requestsToBeForwarded[i]);
                if (send_to != server_no && send_to != -1)
                {
                    num_requests_forwarded++;
                    spdlog::trace("\t\tServer #{} will forward the requestID: {} to the server#: ", server_no, requestsToBeForwarded[i].getReqId(), send_to);
                    // Put in queue so that it can be forwarded once every server has executed the pipeline
                    deferredRequests.push(make_pair(send_to, requestsToBeForwarded[i]));
                    // purge the request fm the queue
                    removeRequest(requestsToBeForwarded[i]);
                }
            }
            if (!num_requests_forwarded)
                break;
        }
    }

    void processData(int currentTime, int timeDelta, Server *servers[], int server_count)
    {
        int bytesProcessedInDelta = 0;
        int maxBytes = timeDelta * alpha;
        // Conduct normal execution on this server
        spdlog::trace("\t\tServer #{} will process {} bytes in {} time units", server_no, maxBytes, timeDelta);
        while (!reqQueue.empty() && maxBytes > 0)
        {
            Request &cur = reqQueue.front();
            int pendingSize = cur.getPendingSize();
            if (pendingSize == cur.getRespSize()) {
                totalWaitingTime += currentTime - cur.getTimestamp() + ((double)bytesProcessedInDelta) / maxBytes;
            }
            if (pendingSize > maxBytes)
            {
                // update
                cur.updatePendingSize(maxBytes);
                spdlog::trace("\t\t\tServer #{} processed {} / {} bytes of response for request #{}", server_no, (cur.getRespSize() - cur.getPendingSize()), cur.getRespSize(), cur.getReqId());
                totalRespBytesProcessed += maxBytes;
                bytesProcessedInDelta += maxBytes;
                maxBytes -= maxBytes;
            }
            else
            {
                // update
                double timestamp = currentTime + (pendingSize * 1.0) / maxBytes;
                maxBytes -= pendingSize;
                cur.updatePendingSize(pendingSize);
                spdlog::trace("\t\t\tServer #{} processed {} / {} bytes of response for request #{}", server_no, (cur.getRespSize() - cur.getPendingSize()), cur.getRespSize(), cur.getReqId());
                reqQueue.pop();

                cur.updateFinishedTimestamp(timestamp); // +1 because it finishes at the end of current time unit
                processedReqQueue.push(cur);
                totalFullyProcessedBytes += cur.getRespSize();
                totalRespBytesProcessed += pendingSize;
                bytesProcessedInDelta += pendingSize;
                totalRespTime += cur.getFinishedTimestamp() - cur.getTimestamp();
                spdlog::trace("\t\tRequest Id {} started at {} finished at {}, diff = {}", cur.getReqId(), cur.getTimestamp(), cur.getFinishedTimestamp(), cur.getFinishedTimestamp()-cur.getTimestamp());
            }
        }
        spdlog::trace("\t\t\tServer #{} this iteration: bytes processed: {} | Busytime this iteration: {}", server_no, bytesProcessedInDelta, (bytesProcessedInDelta*1.0)/(alpha*1.0));
        totalBusyTime += (bytesProcessedInDelta*1.0)/(alpha*1.0);
    }

    double getPartiallyProcessedRequestCount() {
        Request &r = reqQueue.front();
        return r.getPendingSize() < r.getRespSize() ? 1 : 0;
    }
};

void printStatistics(Server *servers[], int server_count, double time){
    cout << endl;
    spdlog::info("----STATISTICS----");
    cout << endl;
    spdlog::info("Per server");

    ifstream infile(serverStats);
    if(!infile.good()){
        outputFile.open(serverStats);
        outputFile << "Time" << "," << "Server_no" << ","  << "Pending_Requests" << "," << "Pending_req_size"<<","<<"Processed Requests"<<","<<"Processed req size"<<","<<"Utilization"<<","<<"Busy Time"<<","<<"Average Service Rate"<<","<<"Average # requests in system"<<","<<"Average Response Time"<<endl;
        outputFile.close();
        outputFile.open(overallStats);
        outputFile << "Time" << "," << "Total_reqs_processed" << ","  << "Avg_utilization" << "," << "total_bytes_processed"<<","<<"total_pending_reqs"<<","<<"total_pending_respSize"<<","<<"Consolidated average # requests in system"<<endl;
        outputFile.close();
    }
    outputFile.open(serverStats,std::ios_base::app);

    long totalPendingRespSize = 0, totalPendingReqsCount = 0, totalBytesProcessed = 0, totalRequestsProcessed = 0, consolidatedCumulativePendingCount = 0;
    double totalUtilization = 0.0;
    for (int i = 0; i < server_count; i++)
    {
        // Get the data
        long pendingReqsCount = (*servers[i]).getPendingRequestCount();
        long pendingReqsSize = (*servers[i]).getPendingRequestSize();
        long bytesProcessed = (*servers[i]).getTotalProcessedBytes();
        long numProcessedRequests = (*servers[i]).getNumProcessedRequests();
        double utilization =  (*servers[i]).calculateUtilization(time);
        double busyTime = (*servers[i]).getTotalBusyTime();
        double averageServiceRate = (*servers[i]).getAverageServiceRate();
        long cumulativePendingCount = (*servers[i]).getCumulativePendingCount();
        double avgRespTime = (*servers[i]).getAvgRespTime();
        double numPartiallyProcessedRequests = (*servers[i]).getPartiallyProcessedRequestCount();
        double avgWaitingTime = (*servers[i]).getTotalWaitingTime() / (numProcessedRequests + numPartiallyProcessedRequests);
        // Add to cumulative
        totalPendingReqsCount += pendingReqsCount;
        totalPendingRespSize += pendingReqsSize;
        totalBytesProcessed += bytesProcessed;
        totalRequestsProcessed += numProcessedRequests;
        totalUtilization += utilization;
        consolidatedCumulativePendingCount += cumulativePendingCount;
        // Print out
        spdlog::info("\tTime: {}", time);
        spdlog::info("\tServer #{}", i);
        spdlog::info("\t\t # of requests processed : {}", numProcessedRequests);
        spdlog::info("\t\t Size of processed responses : {} bytes", bytesProcessed);
        spdlog::info("\t\t # of pending requests : {}", pendingReqsCount);
        spdlog::info("\t\t Size of pending responses : {} bytes", pendingReqsSize);
        spdlog::info("\t\t Utilization : {} ", utilization);
        spdlog::info("\t\t Busy time: {}", busyTime); 
        spdlog::info("\t\t Average Service Rate: {}", averageServiceRate);
        spdlog::info("\t\t Average number of requests in the system: {}", (cumulativePendingCount * 1.0)/(time * 1.0));
        spdlog::info("\t\t Average Response Time: {}", avgRespTime);
        spdlog::info("\t\t Average Waiting Time: {}", avgWaitingTime);
        outputFile << time << "," << i << ","  << pendingReqsCount << "," << pendingReqsSize<<","<<numProcessedRequests<<","<<bytesProcessed<<","<<utilization<<","<<busyTime<<","<<averageServiceRate<<","<<(cumulativePendingCount*1.0)/(time*1.0)<<","<<avgRespTime<<endl;
    }
    cout << endl;
    outputFile.close();
    outputFile.open(overallStats,std::ios_base::app);
    outputFile << time << "," << totalRequestsProcessed << ","  << totalUtilization/double(server_count) << "," << totalBytesProcessed<<","<<totalPendingReqsCount<<","<<totalPendingRespSize<<","<<(consolidatedCumulativePendingCount*1.0)/(time*1.0)<<endl;
    outputFile.close();
    spdlog::info("Cumulative");
    spdlog::info("Total # of requests processed : {}", totalRequestsProcessed);
    spdlog::info("Total size of processed responses : {} bytes", totalBytesProcessed);
    spdlog::info("Total # of pending requests : {}", totalPendingReqsCount);
    spdlog::info("Total pending response size : {} bytes", totalPendingRespSize);
    spdlog::info("Consolidated average number of requests in the system: {} / {} = {}", consolidatedCumulativePendingCount, time, (consolidatedCumulativePendingCount*1.0)/time);
}

int main(int argc, char **argv)
{
    spdlog::cfg::load_env_levels();
    // Initializations
    double maxSimulationTime = 10000000;
    double time = 0;
    double snapshotInterval = 10.0;     // Percentage value
    double snapshotTime = ((snapshotInterval/100) *maxSimulationTime);
    double checkTime = snapshotTime;
    // Delete stat files
    if(remove(serverStats) != 0){
        spdlog::error("Couldn't delete server stat file");
    }
    if(remove(overallStats) != 0){
        spdlog::error("Couldn't delete server stat file");
    }
    const int server_count = 5;
    int alpha[server_count] = {100, 100, 100, 100, 100};
    Server *servers[server_count];
    spdlog::trace("Simulation parameters");
    spdlog::trace("Simulation time: {}", maxSimulationTime);
    spdlog::trace("Number of server: {}", server_count);
    Poisson p = Poisson(0.2);
    for (int i = 0; i < server_count; i++)
    {
        servers[i] = new Server(alpha[i], i);
    }
    // Iteration
    cout << endl;
    spdlog::trace("----SIMULATION BEGINS----\n\n");
    while (time < maxSimulationTime)
    {
        int t = 0;
        int nextTimeDelta = (int)p.generate();
        if (time != 0) {
            spdlog::trace("----------------------------------------");
            spdlog::trace("\tTime elapsed {} time units", time);
            spdlog::trace("\tNext request arrives in {} time units", nextTimeDelta);
            Request request = Request(time, 1, -1);
            spdlog::trace("\tCreated the current request with ID: {}", request.getReqId());
            spdlog::trace("\tCurrent response size = {}", request.getRespSize());
            int nextServer = rand() % server_count;
            spdlog::trace("\tMapping the request on to server #{}", nextServer);
            (*servers[nextServer]).addRequest(request);
        }
        while ((t++ < nextTimeDelta) && (time < maxSimulationTime))
        {
            spdlog::trace("\t\tTime elapsed {} time units", time);
            // Execute policies to forward packets via RDMA
            for (int i = 0; i < server_count; i++)
            {
                (*servers[i]).executeForwardingPipeline(time, 1, servers, server_count);
            }
            // Forward requests
            for (int i = 0; i < server_count; i++)
            {
                (*servers[i]).forwardDeferredRequests(time, servers, server_count);
            }
            // Process the requests on each server till the next request comes in
            for (int i = 0; i < server_count; i++)
            {
                (*servers[i]).processData(time, 1, servers, server_count);
                (*servers[i]).updatePendingCount();
            }
            time++;
            if (time == checkTime) {
                printStatistics(servers , server_count, time);
                checkTime += snapshotTime;
            }
        }
        if (time == checkTime) {
            printStatistics(servers , server_count, time);
            checkTime += snapshotTime;
        }
    }
    spdlog::trace("----SIMULATION ENDS----");
    cout << endl;
    return 0;
}
