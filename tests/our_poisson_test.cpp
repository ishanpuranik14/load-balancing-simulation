//
// Created by ishan on 23/02/20.
//

#include <bits/stdc++.h>
using namespace std;

int poisson_generator(double mean){
    double L = exp(-mean), k=0, p=1;
    do
    {
        k += 1;
        int random_value= rand();
        // cout<<"Random value generated: "<<random_value<<endl;
        p *= ((double)random_value)/((double)RAND_MAX);
    } while (p>L);
    return k-1;
}

int main(){
    int num_iterations = 10000000, num_buckets=1000, largest_populated_bucket=0;
    int buckets[num_buckets] = {0};
    double sum=0;
    // Seeding the random generator
    srand (static_cast <unsigned> (time(0)));
    for(int i=0; i<num_iterations; i++){
        int generated_value = poisson_generator(100);
        // cout<<"Generated value: "<<generated_value<<endl;
        sum += generated_value;
        if(generated_value<num_buckets)buckets[generated_value]++;
    }
    cout<<"Average: "<<sum/num_iterations<<endl<<endl;
    for(int i=0; i<num_buckets;i++){
        if(buckets[i]){
            largest_populated_bucket=i;
        }
    }
    cout<<"Largest populated bucket: "<<largest_populated_bucket<<endl<<endl;
    return 0;
}
