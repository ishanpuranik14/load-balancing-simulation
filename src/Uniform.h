#ifndef LOAD_BALANCING_SIMULATION_UNIFORM_H
#define LOAD_BALANCING_SIMULATION_UNIFORM_H

#include "Generator.h"

class Uniform : public Generator {
private:
    double mean;
public:
    explicit Uniform(double mean);

    double generate();
};

#endif //LOAD_BALANCING_SIMULATION_UNIFORM_H
