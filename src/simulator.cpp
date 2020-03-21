#include "Poisson.h"
#include "Generator.h"
#include <bits/stdc++.h>
using namespace std;

int reqIdGen = 0;

class Request
{
    int timestamp;
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

public:
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
};

class Server
{
    long alpha;
    int server_no;
    std::queue<Request> reqQueue;
    double avgRespSize;
    long totalRespSize;
    long totalReqs;
    // utilization

public:
    Server(long alpha, int server_no)
    {
        this->alpha = alpha;
        this->server_no = server_no;
    }

public:
    queue<Request> getReqQueue()
    {
        return reqQueue;
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

    void processData(int timeUnits)
    {
        int maxBytes = timeUnits * alpha;
        cout << "Server #" << server_no << " | maxBytes " << maxBytes << endl;
        while (!reqQueue.empty() && maxBytes > 0)
        {
            Request &cur = reqQueue.front();
            int pendingSize = cur.getPendingSize();
            if (pendingSize > maxBytes)
            {
                cur.updatePendingSize(maxBytes);
            }
            else
            {
                reqQueue.pop();
            }
            maxBytes -= pendingSize;
            cout << "maxBytesLeft " << maxBytes << endl;
        }
    }
};

int main(int argc, char **argv)
{
    Poisson p = Poisson(1.0 / 2.0);
    int maxSimulationTime = 5;
    int time = 0;
    int reqId = 0;
    int server_count = 5;
    int alpha = 50;

    Server *server[server_count];
    for (int i = 0; i < server_count; i++)
    {
        server[i] = new Server(alpha, i);
    }
    while (time < maxSimulationTime)
    {
        int nextTime = (int)p.generate();
        cout << "next req in " << nextTime << endl;
        time += nextTime;

        Request request = Request(time, 1, -1);
        cout << "RespSize = " << request.getRespSize() << endl;
        int nextServer = rand() % server_count;
        cout << "selecting server " << nextServer << endl;
        (*server[nextServer]).addRequest(request);

        for (int i = 0; i < server_count; i++)
        {
            (*server[i]).processData(nextTime);
        }
        cout << endl;
    }
    cout << "----END----" << endl << endl;
    long processed = alpha * maxSimulationTime;

    int sum = 0;
    queue<Request> reqQ = (*server[0]).getReqQueue();
    int pendingReqsCount = reqQ.size();
    cout << "pendingReqs = " << pendingReqsCount << endl;
    while (!reqQ.empty())
    {
        sum += reqQ.front().getPendingSize();
        reqQ.pop();
    }
    cout << "sum = " << sum << " total = " << processed << endl;

    return 0;
}
