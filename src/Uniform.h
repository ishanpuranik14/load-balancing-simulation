#ifndef LOAD_BALANCING_SIMULATION_UNIFORM_H
#define LOAD_BALANCING_SIMULATION_UNIFORM_H

#include "Generator.h"

class Uniform : public Generator {
private:
    double iat;
    int numRequests;
    int lambda;
    int counter;
public:
    explicit Uniform(double iat,int lambda);
    double generate(int numRequests);
};

#endif //LOAD_BALANCING_SIMULATION_UNIFORM_H
