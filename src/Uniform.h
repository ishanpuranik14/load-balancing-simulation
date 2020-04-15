#ifndef LOAD_BALANCING_SIMULATION_UNIFORM_H
#define LOAD_BALANCING_SIMULATION_UNIFORM_H

#include "Generator.h"

class Uniform : public Generator {
private:
    double mean;
    int numRequests;
    int currentRequests;
public:
    explicit Uniform(double mean);
    explicit Uniform(double mean,int currentRequests);
    double generate();
    double generate(int numRequests);
};

#endif //LOAD_BALANCING_SIMULATION_UNIFORM_H
