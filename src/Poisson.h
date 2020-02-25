#ifndef _POISSON_H
    #define _POISSON_H
    #include "Generator.h"

    class Poisson:public Generator{
        private:
            double mean;
        public:
            Poisson(double mean);
            int generate();
    };
#endif
 