#ifndef LOAD_BALANCING_SIMULATION_UNIFORM_H
#define LOAD_BALANCING_SIMULATION_UNIFORM_H

#include "Generator.h"

class Uniform : public Generator {
private:
    int iat;
    int lambda;
    int counter;
public:
    explicit Uniform(int lambda, int iat);

    double generate();
};

#endif //LOAD_BALANCING_SIMULATION_UNIFORM_H
