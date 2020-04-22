#ifndef LOAD_BALANCING_SIMULATION_UNIFORM_H
#define LOAD_BALANCING_SIMULATION_UNIFORM_H

#include "Generator.h"

class Uniform : public Generator {
private:
    double iat;
    int lambda;
    int counter;
public:
    explicit Uniform(double iat, int lambda);

    double generate();
};

#endif //LOAD_BALANCING_SIMULATION_UNIFORM_H
