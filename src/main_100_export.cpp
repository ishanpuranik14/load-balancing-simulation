#include "Poisson.h"
#include "Generator.h"
#include <bits/stdc++.h>
#include <unordered_map>

using namespace std;

int main()
{
    unordered_map<int, int> freqs;
    std::vector<double> keys;

    Poisson p = Poisson(0.5/4096);
    int num_iterations = 10000000;
    double sum = 0;
    for (int i = 0; i < num_iterations; i++)
    {
        double nextVal = p.generate();
        freqs[nextVal]++;
        sum += nextVal;
    }
    cout << "Average: " << sum / num_iterations << endl
         << endl;


    keys.reserve(freqs.size());
    for (auto& pair: freqs) {
        keys.push_back(pair.first);
    }
    std::sort(keys.begin(), keys.end());
    ofstream outfile;
    outfile.open("poission.csv", ios::out);
    for (auto& key: keys) {
        outfile << key << ", " << freqs[key] << "\n";
        outfile.flush();
    }
    outfile.close();
    return 0;
}