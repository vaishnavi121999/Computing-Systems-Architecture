#include <bitset>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <math.h>
#include <vector>

using namespace std;

int main(int argc, char **argv)
{
    ifstream config;
    config.open(argv[1]);

    int m, w, h;
    config >> m;
    config >> h;
    config >> w;

    config.close();

    ofstream out;
    string out_file_name = string(argv[2]) + ".out";
    out.open(out_file_name.c_str());

    ifstream trace;
    trace.open(argv[2]);

    vector<int> pht;
    int pht_size = (int)pow(2, m);
    pht.resize(pht_size);
    pht.assign(pht_size, 2); // Initialising all the counters as weak taken

    vector<int> bht;
    int bht_size = (int)pow(2, h);
    bht.resize(bht_size);
    bht.assign(bht_size, 0); // Initialising each BHT entry to 0

    cout << "Branch Predictor Config:\n";
    cout << "m        : " << m << "\n";
    cout << "h        : " << h << "\n";
    cout << "w        : " << w << "\n";
    cout << "PHT Size : " << pht_size << "\n";
    cout << "BHT Size : " << bht_size << "\n";
    cout << "\n";

    // for (int i = 0; i < pht_size; i++)
    // {
    //     cout << "PHT Value at " << i << " : " << pht[i] << "\n";
    // }

    // for (int i = 0; i < bht_size; i++)
    // {
    //     cout << "BHT Value at " << i << " : " << bht[i] << "\n";
    // }

    string line;
    size_t space_location;
    string address_string, branch_value_string;
    int address, branch_value;

    bitset<32> address_bits, bht_index_bits, pht_index_bits, w_bits;
    int bht_index, pht_index, w_value, pht_value;

    int max_w = (int) pow(2, w);

    int branch_prediction;

    int line_number = 1;

    // TODO: Implement a two-level branch predictor
    while (!trace.eof())
    {
        getline(trace, line);

        // cout << line << endl;
        space_location = line.find(' ');
        address_string = line.substr(0, space_location);
        branch_value_string = line.substr(space_location + 1);

        sscanf(address_string.c_str(), "%x", &address);
        branch_value = stoi(branch_value_string);

        address_bits = bitset<32>(address);

        cout << "Line (" << line_number << ")\n";
        cout << "PC          : " << address_bits.to_string() << "\n";
        cout << "PC Value    : 0x" << setw(8) << setfill('0') << hex << address << "\n";
        cout << "Branch NT/T : " << dec << branch_value << "\n";

        bht_index_bits = bitset<32>(address_bits.to_string().substr(0, 30).substr(30 - h));
        bht_index = bht_index_bits.to_ulong();

        w_value = bht[bht_index];
        w_bits = bitset<32>(w_value);

        pht_index_bits = bitset<32>(address_bits.to_string().substr(0, 30).substr(30 - (m - w))) << w;
        pht_index = pht_index_bits.to_ulong();

        pht_index_bits = bitset<32>(pht_index + w_value);
        pht_index = pht_index_bits.to_ulong();

        pht_value = pht[pht_index];

        cout << "BHT Index   : " << bht_index_bits.to_string() << " (" << dec << bht_index << ")\n";
        cout << "BHT Val.(C) : " << w_bits.to_string() << " (" << dec << w_value << ")\n";
        cout << "PHT Index   : " << pht_index_bits.to_string() << " (" << dec << pht_index << ")\n";
        cout << "PHT Val.(C) : " << bitset<2>(pht_value).to_string() << " (" << dec << pht_value << ")\n";

        if (pht_value >= 2)
        {
            branch_prediction = 1;
            cout << "Branch Pred : " << branch_prediction << "\n";
        }
        else
        {
            branch_prediction = 0;
            cout << "Branch Pred : " << branch_prediction << "\n";
        }
        
        if (branch_value == 1)
        {
            // Updating PHT Table
            if (pht_value == 3)
            {
                pht[pht_index] = 3;
            }
            else
            {
                pht[pht_index] = pht_value + 1;
            }
            cout << "PHT Val.(U) : " << bitset<2>(pht[pht_index]).to_string() << " (" << dec << pht[pht_index] << ")\n";
            
            // Updating BHT Table
            bht[bht_index] = ((w_value << 1) + 1) % max_w;
            cout << "BHT Val.(U) : " << bitset<32>(bht[bht_index]).to_string() << " (" << dec << w_value << ")\n";
        }
        else
        {
            // Updating PHT Table
            if (pht_value == 0)
            {
                pht[pht_index] = 0;
            }
            else
            {
                pht[pht_index] = pht_value - 1;
            }
            cout << "PHT Val.(U) : " << bitset<2>(pht[pht_index]).to_string() << " (" << dec << pht[pht_index] << ")\n";
            
            // Updating BHT Table
            bht[bht_index] = (w_value << 1) % max_w;
            cout << "BHT Val.(U) : " << bitset<32>(bht[bht_index]).to_string() << " (" << dec << w_value << ")\n";
        }

        out << branch_prediction << endl;
        line_number++;
        cout << "\n";
    }

    trace.close();
    out.close();
}

// Path: branchsimulator_skeleton_23.cpp