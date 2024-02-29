#include <bitset>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

#define MemSize (65536)

class PhyMem
{
  public:
    bitset<32> readdata;
    PhyMem()
    {
        DMem.resize(MemSize);
        ifstream dmem;
        string line;
        int i = 0;
        dmem.open("pt_initialize.txt");
        if (dmem.is_open())
        {
            while (getline(dmem, line))
            {
                DMem[i] = bitset<8>(line);
                i++;
            }
        }
        else
            cout << "Unable to open page table init file";
        dmem.close();
    }
    bitset<32> outputMemValue(bitset<12> Address)
    {
        bitset<32> readdata;
        /**TODO: implement!
         * Returns the value stored in the physical address
         */

        // cout << "Address         : " << Address.to_string() << "\n";
        
        for (int i = 0; i < 4; i++)
        {
            bitset<8> data_part = DMem[Address.to_ulong() + i];
            readdata |= (data_part.to_ulong() << (8 * (3 - i)));
        }

        return readdata;
    }

  private:
    vector<bitset<8> > DMem;
};

// int right_shift(bitset<14> request, int shift_amount) 
// {
//     return (request >> shift_amount).to_ulong();
// }

int main(int argc, char *argv[])
{
    PhyMem myPhyMem;

    ifstream traces;
    ifstream PTB_file;
    ofstream tracesout;

    string outname;
    outname = "pt_results.txt";

    traces.open(argv[1]);
    PTB_file.open(argv[2]);
    tracesout.open(outname.c_str());

    // Initialize the PTBR
    bitset<12> PTBR;
    PTB_file >> PTBR;

    string line;
    bitset<14> virtualAddr;

    /*********************************** ↓↓↓ Todo: Implement by you ↓↓↓ ******************************************/

    // Read a virtual address form the PageTable and convert it to the physical address - CSA23
    if (traces.is_open() && tracesout.is_open())
    {
        int line_number = 0;

        int outer_valid_bit;
        int inner_valid_bit;

        bitset<12> physical_address;
        int physical_address_value;
        
        int memory_value;

        int PTBR_val = PTBR.to_ulong();
        cout << "PTBR = " << PTBR.to_string() << "\n";
        cout << "\n";

        while (getline(traces, line))
        {
            cout << "Line (" << line_number << ")\n";
            // TODO: Implement!
            //  Access the outer page table
            virtualAddr = bitset<14>(line);
            cout << "Virtual Add.    : " << virtualAddr.to_string() << "\n"; 

            bitset<4> outer_page_address = bitset<4>(virtualAddr.to_string().substr(0,4));
            bitset<4> inner_page_address = bitset<4>(virtualAddr.to_string().substr(4,4));
            bitset<6> offset = bitset<6>(virtualAddr.to_string().substr(8));

            cout << "Outer Page Add. : " << outer_page_address << "\n";
            cout << "Inner Page Add. : " << inner_page_address << "\n";
            cout << "Offset          : " << offset << "\n";

            bitset<12> translated_outer_address = bitset<12>(PTBR_val + (outer_page_address.to_ulong() << 2));
            cout << "Outer Page Act. : " << translated_outer_address.to_string() << "\n";

            bitset<32> outer_page_table_data = myPhyMem.outputMemValue(translated_outer_address);
            cout << "Outer Page Data : " << outer_page_table_data.to_string() << "\n";
            
            if (outer_page_table_data.to_string().substr(31) == "1")
            {
                outer_valid_bit = 1;
                cout << "Outer Valid Bit : " << outer_valid_bit << "\n";
                
                bitset<12> translated_inner_address = bitset<12>(outer_page_table_data.to_string().substr(0,12));
                cout << "Inner Page PTBR : " << translated_inner_address.to_string() << "\n";

                translated_inner_address = bitset<12>(translated_inner_address.to_ulong() + (inner_page_address.to_ulong() << 2));
                cout << "Inner Page Act. : " << translated_inner_address.to_string() << "\n";

                bitset<32> inner_page_table_data = myPhyMem.outputMemValue(translated_inner_address);
                cout << "Inner Page Data : " << inner_page_table_data.to_string() << "\n";

                if (inner_page_table_data.to_string().substr(31) == "1")
                {
                    inner_valid_bit = 1;
                    cout << "Inner Valid Bit : " << inner_valid_bit << "\n";

                    bitset<6> frame = bitset<6>(inner_page_table_data.to_string().substr(0,6));
                    cout << "Frame           : " << frame << "\n";
                    
                    physical_address = bitset<12>(frame.to_string() + offset.to_string());
                    memory_value = myPhyMem.outputMemValue(physical_address).to_ulong();
                    
                    cout << "Physical Add.   : " << physical_address << "\n";
                    cout << "Value           : " << memory_value << "\n";
                }
                else
                {
                    inner_valid_bit = 0;
                    cout << "Inner Valid Bit : " << inner_valid_bit << "\n";

                    physical_address = bitset<12>(string(12,'0'));
                    memory_value = 0;
                }
            }
            else
            {
                outer_valid_bit = 0;
                cout << "Outer Valid Bit : " << outer_valid_bit << "\n";

                inner_valid_bit = 0;
                cout << "Inner Valid Bit : " << inner_valid_bit << "\n";

                physical_address = bitset<12>(string(12,'0'));
                memory_value = 0;
            }
            
            // If outer page table valid bit is 1, access the inner page table

            // Return valid bit in outer and inner page table, physical address, and value stored in the physical
            // memory.
            //  Each line in the output file for example should be: 1, 0, 0x000, 0x00000000
            
            tracesout << outer_valid_bit << ", " << inner_valid_bit << ", 0x" << setw(3) << setfill('0') << hex << physical_address.to_ulong() << ", 0x" << setw(8) << setfill('0') << hex << memory_value << "\n";
            
            cout << "\n";
            line_number++;
        }
        traces.close();
        tracesout.close();
    }

    else
        cout << "Unable to open trace or traceout file ";

    return 0;
}
