#include "Generator.h"
#include<bits/stdc++.h>
Generator::Generator(){
    srand (static_cast <unsigned> (time(0)));
}

double Generator::generate_random_number(double low, double high){
        int random_value = rand();
        return (low+(static_cast<double>(random_value)/(static_cast<double>(RAND_MAX/(high-low)))));
    }

double Generator:: generate(){
        return 0;
    }
