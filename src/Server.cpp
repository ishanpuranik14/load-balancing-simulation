#include "Server.h"
#include "Clock.h"

Server::Server(long long alpha, int server_no, long double startStatCollectionFrom) : stats(startStatCollectionFrom) {
    // Initializations
    utilization = 0.0;
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

long double Server::getUtilization() {
    return utilization;
}

long long Server::getPendingRequestCount() {
    return static_cast<long long>(reqQueue.size());
}

long long Server::getPendingRequestSize() {
    return pendingReqSize;
}

void Server::setPendingRequestSize(long long i) {
    pendingReqSize = i;
    stats.setPendingRequestSize(i);
}

long double Server::calculateUtilization() {
    return (long double) totalFullyProcessedBytes / (alpha * currentTime);
}

// Called during creation and forwarding requests
void Server::addRequest(long double timestamp, int respSize, int sentBy, long double forwardingTimestamp) {
    Request x = Request(timestamp, 1, sentBy, respSize);
    x.updateForwardingTimestamp(forwardingTimestamp);
    reqQueue.push(x);

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

bool Server::whenPolicy(int policyNum, int timeDelta, Server **servers, int server_count) {
    /*
    Use the when policy to determine whether to forward any request(s)
    */
    bool time_to_forward = false;
    long double policy_0_threshold = 1.5;
    spdlog::trace("\t\t\tWhen policy #{}:", policyNum);
    switch (policyNum) {
        case -1:
            break;
        case 0:
            // Using utilization
            this->utilization = calculateUtilization();
            spdlog::trace("\t\t\t\tServer #{} | utilization: {} | threshold: {}", server_no, utilization,
                          policy_0_threshold);
            if (this->utilization > policy_0_threshold) {
                time_to_forward = true;
            }
            break;
        default:
            break;
    }
    return time_to_forward;
}

std::vector<Request> Server::whatPolicy(int policyNum, int timeDelta, Server **servers, int server_count) {
    /*
    Use the what policy to determine which request(s) to forward
    */
    std::vector<Request> requestsToBeForwarded;
    // Go thru all the requests
    long long numRequests = getPendingRequestCount();
    spdlog::trace("\t\t\tWhat policy #{}:", policyNum);
    spdlog::trace("\t\t\tServer #{} has average response size: {}", server_no, avgRespSize);
    while (numRequests--) {
        // Get the request
        Request &cur = reqQueue.front();
        spdlog::trace("\t\t\t\tConsidering RequestID: {} with response size: {} and pending size: {}", cur.getReqId(),
                      cur.getRespSize(), cur.getPendingSize());
        // Dont consider partially processed/ forwarded requests
        if (cur.getRespSize() == cur.getPendingSize() && cur.getSentBy() == -1) {
            // Apply the policy
            switch (policyNum) {
                case 0:
                    // forward the ones whose size > avg
                    if (cur.getRespSize() > avgRespSize) {
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

int Server::wherePolicy(int policyNum, int timeDelta, Server **servers, int server_count,
                        Request requestToBeForwarded) {
    /*
    Use the where policy to send to the appropriate server
    */
    // TODO include alpha while computing load
    int send_to = server_no;               // Use this to determine whom to send the request to
    long long least_load;                       // Use this to store the load of the server chosen
    std::vector<int> randomly_selected_servers; // Use this for Power of k
    int k = 2;                             // Use this to play with Power of k
    spdlog::trace("\t\t\tWhere Policy #{} executing switch", policyNum);
    switch (policyNum) {
        case 0:
            /* next server */
            send_to = (server_no + 1) % server_count;
            least_load = (*servers[send_to]).getPendingRequestSize();
            break;
        case 1:
            /* least loaded */
            least_load = (*servers[(server_no + 1) % server_count]).getPendingRequestSize();
            send_to = (server_no + 1) % server_count;
            for (int i = 0; i < server_count; i++) {
                if (i != server_no) {
                    long long load = (*servers[i]).getPendingRequestSize();
                    if (load < least_load) {
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
            for (int i = 0; i < k; i++) {
                randomly_selected_server = rand() % server_count;
                // Regenerate if redundant
                while (randomly_selected_server == server_no ||
                       (find(randomly_selected_servers.begin(), randomly_selected_servers.end(),
                             randomly_selected_server) != randomly_selected_servers.end())) {
                    randomly_selected_server = rand() % server_count;
                }
                randomly_selected_servers.push_back(randomly_selected_server);
                long load = (*servers[randomly_selected_server]).getPendingRequestSize();
                if (load < least_load) {
                    least_load = load;
                    send_to = randomly_selected_server;
                }
            }
            break;
        default:
            break;
    }
    // Sanity check so as to not forward requests to other servers with higher load
    if (least_load > getPendingRequestSize()) {
        return -1;
    }
    spdlog::trace("\t\t\tWhere policy #{}: the least load is for {} and is equal to {} bytes", policyNum, send_to,
                  least_load);
    return send_to;
}

// TODO update any counters
void Server::removeRequest(Request requestToBeRemoved) {
    long long numRequests = getPendingRequestCount();
    while (numRequests--) {
        // Get the request
        Request &cur = reqQueue.front();
        reqQueue.pop();
        // Ignore if this is the one to be removed
        if (cur.getReqId() != requestToBeRemoved.getReqId()) {
            // Add to the back of the queue
            reqQueue.push(cur);
        }
    }
}

void Server::forwardRequest(int send_to, Request request, Server **servers, int server_count,
                            bool removeRequestFromQueue) {
    // purge the request fm the queue
    if (removeRequestFromQueue) {
        removeRequest(request);
        stats.decrementPendingReqCount();
    }
    // Add the request in the reciever's queue
    // Update the relevant stats as you send stuff
    (*servers[send_to]).addRequest(request.getTimestamp(), request.getRespSize(), server_no, currentTime);
}

void Server::forwardDeferredRequests(Server **servers, int server_count) {
    while (!deferredRequests.empty()) {
        auto forwardingInfo = deferredRequests.front();
        deferredRequests.pop();
        forwardRequest(forwardingInfo.first, forwardingInfo.second, servers, server_count, false);
    }
}

void Server::executeForwardingPipeline(int timeDelta, Server **servers, int server_count) {
    // Execute the when, what and where policies keeping in mind the timeUnits
    int when_policy = -1;  // Use this to control the when policy
    int what_policy = 0;  // Use this to control the what policy
    int where_policy = 0; // Use this to control the where policy
    spdlog::trace("\t\tServer #{} will execute the when policy", server_no);
    while (whenPolicy(when_policy, timeDelta, servers, server_count)) {
        int num_requests_forwarded = 0;
        spdlog::trace("\t\tServer #{} will execute the what policy", server_no);
        // Go thru and execute the what policy till it becomes inapplicable
        std::vector<Request> requestsToBeForwarded = whatPolicy(what_policy, timeDelta, servers, server_count);
        // Forward each request using the where policy
        for (long long i = 0; i < requestsToBeForwarded.size(); i++) {
            spdlog::trace("\t\tServer #{} will execute the where policy for requestID: {}", server_no,
                          requestsToBeForwarded[i].getReqId());
            int send_to = wherePolicy(where_policy, timeDelta, servers, server_count, requestsToBeForwarded[i]);
            if (send_to != server_no && send_to != -1) {
                num_requests_forwarded++;
                spdlog::trace("\t\tServer #{} will forward the requestID: {} to the server#: ", server_no,
                              requestsToBeForwarded[i].getReqId(), send_to);
                // Put in queue so that it can be forwarded once every server has executed the pipeline
                deferredRequests.push(std::make_pair(send_to, requestsToBeForwarded[i]));
                // purge the request fm the queue
                removeRequest(requestsToBeForwarded[i]);
            }
        }
        if (!num_requests_forwarded)
            break;
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
            remainingCapacityForDelta = 0;
        } else {
            // request finished processing
            remainingCapacityForDelta -= pendingSize;
            setPendingRequestSize(getPendingRequestSize() - pendingSize);
            cur.updatePendingSize(pendingSize);
            spdlog::trace(
                    "\t\t\t Else condition: Going to pop!! Server #{} processed {} / {} bytes of response for request #{}",
                    server_no,
                    (cur.getRespSize() - cur.getPendingSize()), cur.getRespSize(), cur.getReqId());
            reqQueue.pop();
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
