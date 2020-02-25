#include "Poisson.h"
#include<bits/stdc++.h>

Poisson::Poisson(double mean):Generator(){
            this->mean = mean;
        }

int Poisson::generate(){
            double L = exp(-(this->mean)), k=0, p=1;
            do{
                k += 1;
                int random_value = rand();
                p *= generate_random_number(0,1);
            } while (p>L);
            return k-1;
        }
 