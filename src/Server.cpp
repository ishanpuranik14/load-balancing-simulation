#include "Server.h"
#include "Clock.h"
#include <map>

using namespace std;

Server::Server(long long alpha, int server_no, long double startStatCollectionFrom) : stats(startStatCollectionFrom) {
    // Initializations
    avgRespSize = 0.0;
    totalFullyProcessedBytes = 0;
    this->alpha = alpha;
    this->server_no = server_no;
    totalReqs = 0;
    totalRespSize = 0;
    pendingReqSize = 0;
    spdlog::trace("\tServer #{} | alpha : {}", server_no, alpha);
}


long long Server::getAlpha() {
    return alpha;
}

long double Server::getUtilization(bool historic=false) {
    // TODO Read the time units for history from the config and put them as a server variable. Use that to return 0 when we dont have enough history. 
    if(historic){
        return utilizations.front();
    } else {
        return calculateUtilization();
    }
}

long long Server::getPendingRequestCount() {
    return static_cast<long long>(reqQueue.size());
}

long long Server::getPendingRequestSize() {
    return pendingReqSize;
}

long double Server::getServerLoad(bool historic){
    // TODO Read the time units for history from the config and put them as a server variable. Use that to return 0 when we dont have enough history. 
    if(historic){
        return loads.front();
    } else {
        return ((double)getPendingRequestSize())/alpha;
    }
}

void Server::setPendingRequestSize(long long i) {
    pendingReqSize = i;
    stats.setPendingRequestSize(i);
}

long double Server::calculateUtilization() {
    return (long double) totalFullyProcessedBytes / (alpha * currentTime);
}

void Server::storeHistoricData(long timeUnits){
    // Add utilization
    long double utilization = calculateUtilization();
    spdlog::trace("\t\t\tFor Server #{}, storing utilization as {}", server_no, utilization);
    utilizations.push_back(utilization);
    if(utilizations.size() > timeUnits){
        utilizations.pop_front();
    }
    // Add load
    long double load = getServerLoad();
    spdlog::trace("\t\t\tFor Server #{}, storing server load as {}", server_no, load);
    loads.push_back(load);
    if(loads.size() > timeUnits){
        loads.pop_front();
    }
}

// Called during creation and forwarding requests
void Server::addRequest(long double timestamp, int respSize, int sentBy, long double forwardingTimestamp, long long id) {
    Request x = Request(timestamp, 1, sentBy, respSize, id);
    x.updateForwardingTimestamp(forwardingTimestamp);
    reqQueue.push_back(x);
    if(sentBy != -1){
        stats.setNumRequestsForwardedToThisServer(stats.getNumRequestsForwardedToThisServer()+1);
        stats.setTotalSizeForwardedToThisServer(stats.getTotalSizeForwardedToThisServer()+respSize);
        stats.setPendingForwardedRequestsToThisServer(stats.getPendingForwardedRequestsToThisServer()+1);
        stats.setPendingSizeForwardedRequestsToThisServer(stats.getPendingSizeForwardedRequestsToThisServer()+respSize);
    }
    stats.incrementPendingReqCount();
    stats.setTotalReqs(stats.getTotalReqs() + 1);
    stats.setTotalRespSize(stats.getTotalRespSize() + respSize);
    stats.setAvgRespSize((long double) stats.getTotalRespSize() / stats.getTotalReqs());

    setPendingRequestSize(getPendingRequestSize() + respSize);
    totalReqs++;
    totalRespSize += respSize;
    avgRespSize = (long double) totalRespSize / totalReqs;
}

void Server::updatePendingCount() {
    stats.setCumulativePendingCount(stats.getCumulativePendingCount() + getPendingRequestCount());
}

bool Server::whenPolicy(int policyNum, int timeDelta, Server **servers, int server_count, std::map<std::string,int> &policies, std::deque<RequestSpec> &requestSpecs) {
    /*
    Use the when policy to determine whether to forward any request(s)
    */
    bool time_to_forward = false;
    long policy_2_time_units = 3 * policies["granularity"];
    double serverLoad;
    long double policy_0_threshold = 0.75, policy_1_threshold = 1.5, policy_2_threshold = 6.0;
    std::deque<RequestSpec> seenTimeDeltas;
    spdlog::trace("\t\t\tWhen policy #{}:", policyNum);
    switch (policyNum) {
        case -1:
            break;
        case 0: {
            // Using utilization
            long double utilization = getUtilization();
            spdlog::trace("\t\t\t\tServer #{} | utilization: {} | threshold: {}", server_no, utilization,
                          policy_0_threshold);
            if (utilization > policy_0_threshold) {
                time_to_forward = true;
            }
            break;
        }
        case 1: {
            // Using the pending request queue size as a proportion of current processing capacity
            // To take care of tails
            serverLoad = getServerLoad();
            spdlog::trace("\t\t\t\tServer #{} | server load: {} | threshold: {}", server_no, serverLoad, policy_1_threshold);
            if(serverLoad > policy_1_threshold){
                time_to_forward = true;
            }
            break;
        }
        case 2: case 3: {
            // Proactively assessing server load after a given time and taking preventive measures
            int currentDelta;
            long t = 0;
            long long forecastedPendingCount = reqQueue.size();
            // Go thru the time deltas to compute pending size till minimum(available time deltas, desired time gets elapsed)
            // We should never run out available time deltas. the queue needs to be populated enough
            while(!requestSpecs.empty() && (t + (currentDelta = requestSpecs.front().timeDelta)) < policy_2_time_units){
                t += currentDelta;
                RequestSpec currentSpec = requestSpecs.front();
                requestSpecs.pop_front();
                seenTimeDeltas.push_back(currentSpec);
                // update counts
                forecastedPendingCount++;
            }
            // Put all requests back in the original queue
            while(!seenTimeDeltas.empty()){
                requestSpecs.push_front(seenTimeDeltas.back());
                seenTimeDeltas.pop_back();
            }
            // use the ratio (The subtraction takes care of requests processed in policy_2_time_units)
            serverLoad = ((double)((forecastedPendingCount)*avgRespSize)/(double)alpha) - policy_2_time_units;
            spdlog::trace("\t\t\t\tServer #{} | Forecasting for: {} time units", server_no, policy_2_time_units);
            spdlog::trace("\t\t\t\tServer #{} | Current pending count: {} | Current Forecasted incoming: {} | Outgoing in those time units: {}", server_no, reqQueue.size(), forecastedPendingCount, (double)(policy_2_time_units*alpha)/(double)avgRespSize);
            spdlog::trace("\t\t\t\tServer #{} | Current server load: {}", server_no, getServerLoad());
            spdlog::trace("\t\t\t\tServer #{} | forecasted server load: {} | threshold: {}", server_no, serverLoad, policy_2_threshold);
            if(policyNum == 3){
                // Hybrid policy
                serverLoad = 0.5*serverLoad + 0.5*getServerLoad();
                if(serverLoad > 0.5*policy_2_threshold + 0.5*policy_1_threshold){
                    time_to_forward = true;
                }
            } else {
                if(serverLoad > policy_2_threshold){
                    time_to_forward = true;
                }
            }
            break;
        }
        default:
            break;
    }
    return time_to_forward;
}

std::vector<std::_List_iterator<Request>> Server::whatPolicy(int policyNum, int timeDelta, Server **servers, int server_count) {
    /*
    Use the what policy to determine which request(s) to forward
    */
    std::vector<std::_List_iterator<Request>> requestsToBeForwarded;
    double policy_1_fraction = 0.2, policy_2_fraction = 0.15;
    // Go thru all the requests
    spdlog::trace("\t\t\tWhat policy #{}:", policyNum);
    spdlog::trace("\t\t\tServer #{} has average response size: {}", server_no, avgRespSize);
    long k=0;
    for (auto it=reqQueue.begin(); it != reqQueue.end(); ++it, k++) {
        auto cur = *it;
        spdlog::trace("\t\t\t\tConsidering RequestID: {} with response size: {} and pending size: {}", cur.getReqId(),
                      cur.getRespSize(), cur.getPendingSize());
        // Dont consider partially processed/ forwarded requests
        if (cur.getRespSize() == cur.getPendingSize() && cur.getSentBy() == -1) {
            // Apply the policy
            bool break_flag = false;
            switch (policyNum) {
                case 0: case 1: case 2: {
                    // forward the ones whose size > avg
                    if (cur.getRespSize() >= avgRespSize) {
                        spdlog::trace("\t\t\t\t\tRequestID: {}  qualifies for forwarding", cur.getReqId());
                        requestsToBeForwarded.push_back(it);
                    }
                    if(policyNum == 2){
                        if(k >= (int)(policy_2_fraction*reqQueue.size())){
                            break_flag = true;
                        }
                    }
                    break;
                }
                default:
                    break;
            }
            if(break_flag)break;
        }
    }
    switch (policyNum){
        // Slice the vector, to limit the number of requests being forwarded. NOTE: makes sense for lambda > mu. probably not for mu > lambda
        case 1: {
            spdlog::trace("\t\t\t\t\tConsidering only {} eligible requests", ceil(policy_1_fraction*requestsToBeForwarded.size()));
            return std::vector<std::_List_iterator<Request>>(requestsToBeForwarded.begin(), requestsToBeForwarded.begin() + (int)ceil(policy_1_fraction*requestsToBeForwarded.size()));
        }
        default:
            break;
    }
    return requestsToBeForwarded;
}

int Server::wherePolicy(int policyNum, int timeDelta, Server **servers, int server_count,
                        Request requestToBeForwarded,std::map<std::string,int> &policies) {
    /*
    Use the where policy to send to the appropriate server
    */
    int send_to = server_no;               // Use this to determine whom to send the request to
    long double load, least_load;                       // Use this to store the load of the server chosen
    std::vector<int> randomly_selected_servers; // Use this for Power of k
    int k = policies["k"];                           // Use this to play with Power of k
    spdlog::trace("\t\t\tWhere Policy #{} executing switch", policyNum);
    switch (policyNum) {
        case 0: {
            /* next server */
            send_to = (server_no + 1) % server_count;
            least_load = (*servers[send_to]).getServerLoad();
            break;
        }
        case 1: {
            /* least loaded */
            least_load = (*servers[(server_no + 1) % server_count]).getServerLoad();
            send_to = (server_no + 1) % server_count;
            for (int i = 0; i < server_count; i++) {
                if (i != server_no) {
                    load = (*servers[i]).getServerLoad();
                    spdlog::trace("\t\t\t\tFor least loaded, Comparing the load of server #{}, load: {}", i, load);
                    if (load < least_load) {
                        least_load = load;
                        send_to = i;
                    }
                }
            }
            break;
        }
        case 2: {
            /* Power of k */
            least_load = LONG_MAX;
            if(server_count>k){    
                int randomly_selected_server;
                for (int i = 0; i < k; i++) {
                    randomly_selected_server = rand() % server_count;
                    // Regenerate if redundant
                    while (randomly_selected_server == server_no ||
                        (find(randomly_selected_servers.begin(), randomly_selected_servers.end(),
                                randomly_selected_server) != randomly_selected_servers.end())) {
                        randomly_selected_server = rand() % server_count;
                    }
                    randomly_selected_servers.push_back(randomly_selected_server);
                    load = (*servers[randomly_selected_server]).getServerLoad();
                    spdlog::trace("\t\t\t\tPower of {}, Comparing the load of server #{}, load: {}", k, randomly_selected_server, load);
                    if (load < least_load) {
                        least_load = load;
                        send_to = randomly_selected_server;
                    }
                }
            }
            break;
        }
        case 3: {
            /* least loaded - historic values*/
            least_load = (*servers[(server_no + 1) % server_count]).getServerLoad(true);
            send_to = (server_no + 1) % server_count;
            for (int i = 0; i < server_count; i++) {
                if (i != server_no) {
                    load = (*servers[i]).getServerLoad(true);
                    spdlog::trace("\t\t\t\tFor historic least loaded, Comparing the load of server #{}, load: {}", i, load);
                    if (load < least_load) {
                        least_load = load;
                        send_to = i;
                    }
                }
            }
            break;
        }
        case 4: {
            /* Power of k - historic values */
            least_load = LONG_MAX;
            if(server_count>k){    
                int randomly_selected_server;
                for (int i = 0; i < k; i++) {
                    randomly_selected_server = rand() % server_count;
                    // Regenerate if redundant
                    while (randomly_selected_server == server_no ||
                        (find(randomly_selected_servers.begin(), randomly_selected_servers.end(),
                                randomly_selected_server) != randomly_selected_servers.end())) {
                        randomly_selected_server = rand() % server_count;
                    }
                    randomly_selected_servers.push_back(randomly_selected_server);
                    load = (*servers[randomly_selected_server]).getServerLoad(true);
                    spdlog::trace("\t\t\t\tPower of {} - historic, Comparing the load of server #{}, load: {}", k, randomly_selected_server, load);
                    if (load < least_load) {
                        least_load = load;
                        send_to = randomly_selected_server;
                    }
                }
            }
            break;
        }
        default:
            break;
    }
    // Sanity check so as to not forward requests to other servers with higher load
    if (least_load > getServerLoad()) {
        return -1;
    }
    spdlog::trace("\t\t\tWhere policy #{}: the least load is for {} and is equal to {} bytes", policyNum, send_to,
                  least_load);
    return send_to;
}

void Server::removeRequest(std::_List_iterator<Request> requestIter) {
    setPendingRequestSize(getPendingRequestSize() - (*requestIter).getPendingSize());
    stats.decrementPendingReqCount();
    reqQueue.erase(requestIter);
}

void Server::forwardRequest(int send_to, std::_List_iterator<Request> requestIter, Server **servers, int server_count,
                            bool removeRequestFromQueue) {
    // purge the request fm the queue
    // Add the request in the reciever's queue
    auto request = *requestIter;
    stats.setTotalForwardedRequests(stats.getTotalForwardedRequests()+1);
    stats.setTotalForwardedSize(stats.getTotalForwardedSize() + request.getRespSize());
    (*servers[send_to]).addRequest(request.getTimestamp(), request.getRespSize(), server_no, currentTime, request.getReqId());

    if (removeRequestFromQueue) {
        removeRequest(requestIter);
    }
}

void Server::forwardDeferredRequests(Server **servers, int server_count) {
    while (!deferredRequests.empty()) {
        auto forwardingInfo = deferredRequests.front();
        forwardRequest(forwardingInfo.first, forwardingInfo.second, servers, server_count, true);
        deferredRequests.pop_front();
    }
}

void Server::executeForwardingPipeline(int timeDelta, Server **servers, int server_count, std::map<std::string,int> &policies, std::deque<RequestSpec> &requestSpecs) {
    // Execute the when, what and where policies keeping in mind the timeUnits
    int when_policy = policies["when"];  // Use this to control the when policy
    int what_policy = policies["what"];  // Use this to control the what policy
    int where_policy = policies["where"]; // Use this to control the where policy
    spdlog::trace("\t\tServer #{} will execute the when policy", server_no);
    if (whenPolicy(when_policy, timeDelta, servers, server_count, policies, requestSpecs)) {
        int num_requests_forwarded = 0;
        spdlog::trace("\t\tServer #{} will execute the what policy", server_no);
        // Go thru and execute the what policy till it becomes inapplicable
        auto requestsToBeForwarded = whatPolicy(what_policy, timeDelta, servers, server_count);
        // Forward each request using the where policy
        // Iterating in reverse because we remove requests based on index
        for (auto requestIter=requestsToBeForwarded.rbegin(); requestIter != requestsToBeForwarded.rend(); ++requestIter) {
            auto request = *(*requestIter);
            spdlog::trace("\t\tServer #{} will execute the where policy for requestID: {}", server_no,
                          request.getReqId());
            int send_to = wherePolicy(where_policy, timeDelta, servers, server_count, request,policies);
            if (send_to != server_no && send_to != -1) {
                num_requests_forwarded++;
                spdlog::trace("\t\tServer #{} will forward the requestID: {} to the server#: {}", server_no,
                              request.getReqId(), send_to);
                // Put in queue so that it can be forwarded once every server has executed the pipeline
                deferredRequests.push_front(std::make_pair(send_to, *requestIter));
            }
        }
    }
}

void Server::processData(int timeDelta, Server **servers, int server_count) {
    long long bytesProcessedInDelta = 0;
    long long remainingCapacityForDelta = timeDelta * alpha;
    // Conduct normal execution on this server
    spdlog::trace("\t\tServer #{} will process {} bytes in {} time units", server_no, remainingCapacityForDelta,
                  timeDelta);
    while (!reqQueue.empty() && remainingCapacityForDelta > 0) {
        Request &cur = reqQueue.front();
        long long pendingSize = cur.getPendingSize();
        // Calculate the time spent in the queue, if applicable
        if (pendingSize == cur.getRespSize()) {
            stats.setTotalWaitingTime(stats.getTotalWaitingTime() + currentTime - cur.getTimestamp() +
                                      ((long double) bytesProcessedInDelta) / (long double) (timeDelta * alpha));
        }

        if (pendingSize > remainingCapacityForDelta) {
            // request partially processed
            cur.updatePendingSize(remainingCapacityForDelta);
            spdlog::trace("\t\t\tServer #{} processed {} / {} bytes of response for request #{}", server_no,
                          (cur.getRespSize() - cur.getPendingSize()), cur.getRespSize(), cur.getReqId());
            stats.setTotalRespBytesProcessed(stats.getTotalRespBytesProcessed() + remainingCapacityForDelta);
            bytesProcessedInDelta += remainingCapacityForDelta;
            setPendingRequestSize(getPendingRequestSize() - remainingCapacityForDelta);
            if(cur.getSentBy() != -1){
                stats.setPendingSizeForwardedRequestsToThisServer(stats.getPendingSizeForwardedRequestsToThisServer()-remainingCapacityForDelta);
            }
            remainingCapacityForDelta = 0;
        } else {
            // request finished processing
            remainingCapacityForDelta -= pendingSize;
            setPendingRequestSize(getPendingRequestSize() - pendingSize);
            if(cur.getSentBy() != -1){
                stats.setPendingForwardedRequestsToThisServer(stats.getPendingForwardedRequestsToThisServer()-1);
                stats.setPendingSizeForwardedRequestsToThisServer(stats.getPendingSizeForwardedRequestsToThisServer()-pendingSize);
            }
            cur.updatePendingSize(pendingSize);
            spdlog::trace(
                    "\t\t\t Else condition: Going to pop!! Server #{} processed {} / {} bytes of response for request #{}",
                    server_no,
                    (cur.getRespSize() - cur.getPendingSize()), cur.getRespSize(), cur.getReqId());
            reqQueue.pop_front();
            stats.decrementPendingReqCount();
            bytesProcessedInDelta += pendingSize;
            long double timestamp = currentTime + ((long double) bytesProcessedInDelta) /
                                                  ((long double) (long double) (timeDelta * alpha));
            cur.updateFinishedTimestamp(timestamp); // +1 because it finishes at the end of current time unit
            totalFullyProcessedBytes += cur.getRespSize();

            stats.pushProcessedReqQueueForStats(cur);
            stats.setTotalRespBytesProcessed(stats.getTotalRespBytesProcessed() + pendingSize);
            stats.setTotalRespTime(stats.getTotalRespTime() + cur.getFinishedTimestamp() - cur.getTimestamp());

            spdlog::trace("\t\tRequest Id {} started at {} finished at {}, diff = {}", cur.getReqId(),
                          cur.getTimestamp(), cur.getFinishedTimestamp(),
                          cur.getFinishedTimestamp() - cur.getTimestamp());
        }
    }
    spdlog::trace("\t\t\tServer #{} this iteration: bytes processed: {} | Busytime this iteration: {}", server_no,
                  bytesProcessedInDelta, ((long double) bytesProcessedInDelta) / ((long double) alpha));
    stats.setTotalBusyTime(stats.getTotalBusyTime() + (((long double) bytesProcessedInDelta) / ((long double) alpha)));
}

double Server::getPartiallyProcessedRequestCount() {
    if (!reqQueue.empty()) {
        Request &r = reqQueue.front();
        return r.getPendingSize() < r.getRespSize() ? 1 : 0;
    } else {
        return 0;
    }
}

Stats &Server::getStats() {
    return stats;
}
