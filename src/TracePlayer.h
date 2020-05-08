#ifndef LOAD_BALANCING_SIMULATION_TRACEPLAYER_H
#define LOAD_BALANCING_SIMULATION_TRACEPLAYER_H

#include "Generator.h"
#include "RequestSpec.h"

class TracePlayer {
private:
    std::vector<RequestSpec> trace;
public:
    double timeDeltaFor(long long reqId);

    int serverFor(long long reqId);

    int respSizeFor(long long reqId);

    void record(RequestSpec spec);

    size_t traceSize();

    RequestSpec specFor(long long int reqId);
};

#endif //LOAD_BALANCING_SIMULATION_TRACEPLAYER_H
