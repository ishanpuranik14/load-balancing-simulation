//
// Created by ishan on 23/02/20.
//

#include <bits/stdc++.h>
using namespace std;

random_device rd;
mt19937 gen(rd());

#include <bits/stdc++.h>
#include <unordered_map>

using namespace std;

int main()
{
    unordered_map<int, int> freqs;
    std::vector<int> keys;

    poisson_distribution<int> pd(100);
    long double sum = 0;
    int num_iterations = 10000000;
    for (int i = 0; i < num_iterations; i++)
    {
        int nextVal = pd(gen);
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
    outfile.open("std_poission.csv", ios::out);
    for (auto& key: keys) {
        outfile << key << ", " << freqs[key] << "\n";
    }
    outfile.close();
    return 0;
}
