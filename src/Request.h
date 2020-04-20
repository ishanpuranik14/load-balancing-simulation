#ifndef LOAD_BALANCING_SIMULATION_REQUEST_H
#define LOAD_BALANCING_SIMULATION_REQUEST_H
extern long long reqIdGen;

class Request {
    long double timestamp;
    long double forwardingTimestamp;
    long double finishedTimestamp;
    int reqSize;
    int respSize; // bytes
    int pendingSize;
    int sentBy; // server number; -1 for dispatcher
    long long reqId;  // autogenerated incrementing
    Request *nextRequest, *nextNormalRequest,*prevRequest, *prevNormalRequest;

public:
    static double generate_random_number(double low, double high);

    Request(long double timestamp, int reqSize, int sentBy, int respSize);

    long long getReqId();

    int getRespSize();

    long long getPendingSize();

    long double getTimestamp();

    void updatePendingSize(long long bytesProcessed);

    int getSentBy();

    void updateSentBy(int resentBy);

    long double getForwardingTimestamp();

    void updateForwardingTimestamp(long double forwardingTimestamp);

    long double getFinishedTimestamp();

    void updateFinishedTimestamp(long double finishedTimestamp);

    Request *getNextRequest();

    void setNextRequest(Request *next);

    Request *getNextNormalRequest();

    void setNextNormalRequest(Request *nextNormal);

    Request *getPrevRequest();

    void setPrevRequest(Request *prev);

    Request *getPrevNormalRequest();

    void setPrevNormalRequest(Request *prevNormal);
};

#endif //LOAD_BALANCING_SIMULATION_REQUEST_H
