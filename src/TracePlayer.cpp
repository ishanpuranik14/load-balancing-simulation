#include <bits/stdc++.h>
#include "TracePlayer.h"
#include "spdlog/spdlog.h"

double TracePlayer::timeDeltaFor(long long reqId) {
    long double nextTimeDelta = trace[reqId].timeDelta;
    spdlog::trace("Next request in : {} time units\n", nextTimeDelta);
    return nextTimeDelta;
}

int TracePlayer::serverFor(long long reqId) {
    return trace[reqId].server;
}

int TracePlayer::respSizeFor(long long reqId) {
    return trace[reqId].respSize;
}

RequestSpec TracePlayer::specFor(long long reqId) {
    return trace[reqId];
}

size_t TracePlayer::traceSize() {
    return trace.size();
}

void TracePlayer::record(RequestSpec spec) {
    trace.push_back(spec);
}
