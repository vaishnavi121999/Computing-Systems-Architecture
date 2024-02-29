/*
Cache Simulator

Level one L1 and level two L2 cache parameters are read from file (block size,
line per set and set per cache). The 32 bit address is divided into tag bits
(t), set index bits (s) and block offset bits (b) s = log2(#sets)   b =
log2(block size in bytes)  t=32-s-b 32 bit address (MSB -> LSB): TAG || SET ||
OFFSET

Tag Bits   : the tag field along with the valid bit is used to determine whether
the block in the cache is valid or not.

Index Bits : the set index field is used to determine which set in the cache the
block is stored in.

Offset Bits: the offset field is used to determine which byte in the block is
being accessed.
*/

#include <algorithm>
#include <bitset>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdlib.h>
#include <string>
#include <vector>

using namespace std;

// access state:
#define NA 0         // no action
#define RH 1         // read hit
#define RM 2         // read miss
#define WH 3         // Write hit
#define WM 4         // write miss
#define NOWRITEMEM 5 // no write to memory
#define WRITEMEM 6   // write to memory

// custom access states:
#define FULLCACHE 10 // the cache is full

struct config
{
    int L1blocksize;
    int L1setsize;
    int L1size;
    int L2blocksize;
    int L2setsize;
    int L2size;
};

struct CacheBlock
{
    // we don't actually need to allocate space for data, because we only need to
    // simulate the cache action or else it would have looked something like this:
    // vector<number of bytes> Data;
    int tag_bits; // Using a tag int, instead of tag bits as it is easier
                  // to operate.
    bool valid_bit;
    bool dirty_bit;
};

struct EvictionBlock
{
    int tag_bits;
    bool valid_bit;
    bool dirty_bit;
    int index_bits;
};

struct CacheSet
{
    vector<CacheBlock> cache_blocks;
    int next_eviction;
};

class Cache
{
  private:
    int block_size;
    int associativity;
    int cache_size;

    int number_of_sets;

    vector<CacheSet> cache_sets;

    int index;
    int tag;

    int getTag(bitset<32> address)
    {
        return rightShift(address, (32 - tag_size)).to_ulong();
    }

    int getIndex(bitset<32> address)
    {
        return rightShift(leftShift(address, tag_size), (32 - index_size)).to_ulong();
    }

  public:
    int tag_size;
    int index_size;
    int offset_size;

    bitset<32> leftShift(bitset<32> address, int shift_index)
    {
        return address << shift_index;
    }

    bitset<32> rightShift(bitset<32> address, int shift_index)
    {
        return address >> shift_index;
    }

    Cache(int blocksize, int setsize, int cachesize)
        : block_size(blocksize), associativity(setsize), cache_size(cachesize)
    {
        offset_size = log2(block_size);
        cache_size = cache_size * (int)pow(2, 10);

        if (associativity == 0) // Fully Associative Memory
        {
            index_size = 0;
            tag_size = 32 - offset_size;
            number_of_sets = cache_size / block_size;
            cache_sets.resize(1);
            cache_sets[0].cache_blocks.resize(number_of_sets);

            for (int i = 0; i < number_of_sets; i++)
            {
                cache_sets[0].cache_blocks[i].valid_bit = 0;
                cache_sets[0].cache_blocks[i].dirty_bit = 0;
                cache_sets[0].cache_blocks[i].tag_bits = 0;
            }
        }
        else
        {
            number_of_sets = cache_size / (block_size * associativity);
            index_size = log2(number_of_sets);
            tag_size = 32 - index_size - offset_size;

            cache_sets.resize(number_of_sets);

            for (int i = 0; i < number_of_sets; i++)
            {
                cache_sets[i].cache_blocks.resize(associativity);
                for (int j = 0; j < associativity; j++)
                {
                    cache_sets[i].cache_blocks[j].valid_bit = 0;
                    cache_sets[i].cache_blocks[j].dirty_bit = 0;
                    cache_sets[i].cache_blocks[j].tag_bits = 0;
                }
            }
        }

        cout << "Tag Size    : " << tag_size << "\n";
        cout << "Index Size  : " << index_size << "\n";
        cout << "Offset Size : " << offset_size << "\n";
        cout << "# Sets      : " << number_of_sets << "\n";
    }

    int write(bitset<32> address)
    {
        tag = getTag(address);
        index = getIndex(address);

        cout << "Address : " << address << " (" << hex << address.to_ulong() << ")"
             << "\n";
        cout << "Tag     : " << bitset<32>(tag) << " (" << hex << tag << ")"
             << "\n";
        cout << "Index   : " << bitset<32>(index) << " (" << hex << index << ")"
             << "\n";

        if (associativity == 0)
        {
            for (int i = 0; i < number_of_sets; i++)
            {
                if (cache_sets[0].cache_blocks[i].tag_bits == tag &&
                    cache_sets[0].cache_blocks[i].valid_bit == 1)
                {
                    cache_sets[0].cache_blocks[i].dirty_bit = 1;
                    return WH;
                }
            }
            return WM;
        }
        else 
        {
            for (int i = 0; i < associativity; i++)
            {
                if (cache_sets[index].cache_blocks[i].tag_bits == tag &&
                    cache_sets[index].cache_blocks[i].valid_bit == 1)
                {
                    cache_sets[index].cache_blocks[i].dirty_bit = 1;
                    return WH;
                }
            }
            return WM;
        }
    }

    int read(bitset<32> address)
    {
        tag = getTag(address);
        index = getIndex(address);

        cout << "Address : " << address << " (" << hex << address.to_ulong() << ")"
             << "\n";
        cout << "Tag     : " << bitset<32>(tag) << " (" << hex << tag << ")"
             << "\n";
        cout << "Index   : " << bitset<32>(index) << " (" << hex << index << ")"
             << "\n";

        if (associativity == 0)
        {
            for (int i = 0; i < number_of_sets; i++)
            {
                if (cache_sets[0].cache_blocks[i].tag_bits == tag &&
                    cache_sets[0].cache_blocks[i].valid_bit == 1)
                {
                    return RH;
                }
            }
            return RM;
        }
        else 
        {
            for (int i = 0; i < associativity; i++)
            {
                if (cache_sets[index].cache_blocks[i].tag_bits == tag &&
                    cache_sets[index].cache_blocks[i].valid_bit == 1)
                {
                    return RH;
                }
            }
            return RM;
        }
    }

    int insert_data(bitset<32> address)
    {
        tag = getTag(address);
        index = getIndex(address);

        if (associativity == 0)
        {
            for (int i = 0; i < number_of_sets; i++)
            {
                if (cache_sets[0].cache_blocks[i].valid_bit == 0)
                {
                    cache_sets[0].cache_blocks[i].tag_bits = tag;
                    cache_sets[0].cache_blocks[i].valid_bit = 1;
                    cache_sets[0].cache_blocks[i].dirty_bit = 0;

                    cout << "Data Inserted       : ";
                    cout << cache_sets[0].cache_blocks[i].tag_bits << " ";
                    cout << cache_sets[0].cache_blocks[i].valid_bit << " ";
                    cout << cache_sets[0].cache_blocks[i].dirty_bit << "\n";

                    return 0;
                }
            }
            // Looped through all, found that the Cache is full...
            // return FULLCACHE to System so that we can evict data...
            return FULLCACHE;
        }
        else 
        {
            for (int i = 0; i < associativity; i++)
            {
                if (cache_sets[index].cache_blocks[i].valid_bit == 0)
                {
                    cache_sets[index].cache_blocks[i].tag_bits = tag;
                    cache_sets[index].cache_blocks[i].valid_bit = 1;
                    cache_sets[index].cache_blocks[i].dirty_bit = 0;
                    
                    cout << "Data Inserted       : ";
                    cout << cache_sets[index].cache_blocks[i].tag_bits << " ";
                    cout << cache_sets[index].cache_blocks[i].valid_bit << " ";
                    cout << cache_sets[index].cache_blocks[i].dirty_bit << "\n";
                    
                    return 0;
                }
            }
            // Looped through all, found that the Cache is full...
            // return FULLCACHE to System so that we can evict data...
            return FULLCACHE;
        }
    }

    EvictionBlock evict_block(bitset<32> address)
    {
        tag = getTag(address);
        index = getIndex(address);

        if (associativity == 0)
        {
            // Get the current eviction number
            int eviction_block = cache_sets[0].next_eviction % number_of_sets;
            cout << "Evicting Block at   : " << index << "\n";
            cout << "Eviction Counter    : " << eviction_block << "\n";

            // Store the details of the evicted block
            EvictionBlock evicted_block = {cache_sets[0].cache_blocks[eviction_block].tag_bits,
                                        cache_sets[0].cache_blocks[eviction_block].valid_bit,
                                        cache_sets[0].cache_blocks[eviction_block].dirty_bit,
                                        0};

            // Reset the values of the evicted block
            cache_sets[0].cache_blocks[eviction_block].tag_bits = 0;
            cache_sets[0].cache_blocks[eviction_block].valid_bit = 0;
            cache_sets[0].cache_blocks[eviction_block].dirty_bit = 0;

            // Udpate the eviction counter
            cache_sets[0].next_eviction = eviction_block + 1;

            return evicted_block;
        }
        else 
        {
            // Get the current eviction number
            int eviction_block = cache_sets[index].next_eviction % associativity;
            cout << "Evicting Block at   : " << index << "\n";
            cout << "Eviction Counter    : " << eviction_block << "\n";

            // Store the details of the evicted block
            EvictionBlock evicted_block = {cache_sets[index].cache_blocks[eviction_block].tag_bits,
                                        cache_sets[index].cache_blocks[eviction_block].valid_bit,
                                        cache_sets[index].cache_blocks[eviction_block].dirty_bit,
                                        index};

            // Reset the values of the evicted block
            cache_sets[index].cache_blocks[eviction_block].tag_bits = 0;
            cache_sets[index].cache_blocks[eviction_block].valid_bit = 0;
            cache_sets[index].cache_blocks[eviction_block].dirty_bit = 0;

            // Udpate the eviction counter
            cache_sets[index].next_eviction = eviction_block + 1;

            return evicted_block;
        }
    }

    int insert_data(bitset<32> address, EvictionBlock &inserting_block)
    {
        tag = getTag(address);
        index = getIndex(address);

        if (associativity == 0)
        {
            for (int i = 0; i < number_of_sets; i++)
            {
                if (cache_sets[0].cache_blocks[i].valid_bit == 0)
                {
                    cache_sets[0].cache_blocks[i].tag_bits = tag;
                    cache_sets[0].cache_blocks[i].valid_bit = inserting_block.valid_bit;
                    cache_sets[0].cache_blocks[i].dirty_bit = inserting_block.dirty_bit;
                    
                    cout << "Data Inserted       : ";
                    cout << cache_sets[0].cache_blocks[i].tag_bits << " ";
                    cout << cache_sets[0].cache_blocks[i].valid_bit << " ";
                    cout << cache_sets[0].cache_blocks[i].dirty_bit << " ";
                    cout << 0 << "\n";

                    return 0;
                }
            }
            return FULLCACHE;
        }
        else 
        {
            for (int i = 0; i < associativity; i++)
            {
                if (cache_sets[index].cache_blocks[i].valid_bit == 0)
                {
                    cache_sets[index].cache_blocks[i].tag_bits = tag;
                    cache_sets[index].cache_blocks[i].valid_bit = inserting_block.valid_bit;
                    cache_sets[index].cache_blocks[i].dirty_bit = inserting_block.dirty_bit;
                    
                    cout << "Data Inserted       : ";
                    cout << cache_sets[index].cache_blocks[i].tag_bits << " ";
                    cout << cache_sets[index].cache_blocks[i].valid_bit << " ";
                    cout << cache_sets[index].cache_blocks[i].dirty_bit << " ";
                    cout << index << "\n";

                    return 0;
                }
            }
            return FULLCACHE;
        }
    }

    void print_cache(bitset<32> address)
    {
        tag = getTag(address);
        index = getIndex(address);

        cout << "Associativity: " << associativity << "\n";

        if (associativity == 0)
        {
            for (int i = 0; i < number_of_sets; i++)
            {
                cout << "cache_sets[" << hex << 0 << "].cache_blocks[" << dec << i << "].tag_bits  : " << hex
                     << cache_sets[0].cache_blocks[i].tag_bits << "\n";
                cout << "cache_sets[" << hex << 0 << "].cache_blocks[" << dec << i
                     << "].valid_bit : " << cache_sets[0].cache_blocks[i].valid_bit << "\n";
                cout << "cache_sets[" << hex << 0 << "].cache_blocks[" << dec << i
                     << "].dirty_bit : " << cache_sets[0].cache_blocks[i].dirty_bit << "\n";
            }
        }
        else
        {
            for (int i = 0; i < associativity; i++)
            {
                cout << "cache_sets[" << hex << index << "].cache_blocks[" << dec << i << "].tag_bits  : " << hex
                     << cache_sets[index].cache_blocks[i].tag_bits << "\n";
                cout << "cache_sets[" << hex << index << "].cache_blocks[" << dec << i
                     << "].valid_bit : " << cache_sets[index].cache_blocks[i].valid_bit << "\n";
                cout << "cache_sets[" << hex << index << "].cache_blocks[" << dec << i
                     << "].dirty_bit : " << cache_sets[index].cache_blocks[i].dirty_bit << "\n";
            }
        }
    }
};

class CacheSystem
{
  private:
    Cache *L1;
    Cache *L2;

  public:
    int L1AccessState;
    int L2AccessState;
    int MemAccessState;

    bitset<32> translateToL1Address(int tag, int index)
    {
        bitset<32> tag_bits = bitset<32>(tag);
        tag_bits = L2->leftShift(tag, 32 - L2->tag_size);

        bitset<32> index_bits = bitset<32>(index);
        index_bits = L2->leftShift(index, 32 - L2->tag_size - L2->index_size);

        bitset<32> address = bitset<32>(tag_bits.to_ulong() + index_bits.to_ulong());
        return address;
    }

    bitset<32> translateToL2Address(int tag, int index)
    {
        bitset<32> tag_bits = bitset<32>(tag);
        tag_bits = L1->leftShift(tag, 32 - L1->tag_size);

        bitset<32> index_bits = bitset<32>(index);
        index_bits = L1->leftShift(index, 32 - L1->tag_size - L1->index_size);

        bitset<32> address = bitset<32>(tag_bits.to_ulong() + index_bits.to_ulong());
        return address;
    }

    CacheSystem(int L1blocksize, int L1setsize, int L1size, int L2blocksize, int L2setsize, int L2size)
    {
        cout << "Creating Cache L1: \n";
        L1 = new Cache(L1blocksize, L1setsize, L1size);
        cout << "\n";
        cout << "Creating Cache L2: \n";
        L2 = new Cache(L2blocksize, L2setsize, L2size);
        cout << "\n";

        L1AccessState = NA;
        L2AccessState = NA;
        MemAccessState = NOWRITEMEM;
    }

    void read(bitset<32> address)
    {
        cout << "Reading from L1 Cache \n";

        L1AccessState = L1->read(address);

        if (L1AccessState == RM)
        {
            // L1 Read Miss... Reading from L2...
            cout << "Reading from L2 Cache \n";

            L2AccessState = L2->read(address);

            if (L2AccessState == RM)
            {
                cout << "Missing in L1 and L2...\n";
                cout << "Adding data from memory...\n";
                // L2 Read Miss... Reading data from memory and inserting in L1
                int L1_insert_result = L1->insert_data(address);

                // Bool to check if we are evicting any block to memory...
                bool eviction_to_memory = 0;

                if (L1_insert_result == FULLCACHE)
                {
                    // L1 Cache is Full, we run eviction in L1 and insert old data into L2
                    cout << "L1 Cache is Full!\n";
                    cout << "Evicting data from L1 to L2...\n";
                    
                    EvictionBlock evicted_block = L1->evict_block(address);

                    cout << "Evicted Block       : ";
                    cout << evicted_block.tag_bits << " ";
                    cout << evicted_block.valid_bit << " ";
                    cout << evicted_block.dirty_bit << " ";
                    cout << evicted_block.index_bits << "\n";

                    // L1 eviction completed, now inserting old data into L2
                    bitset<32> new_address = translateToL2Address(evicted_block.tag_bits, evicted_block.index_bits);
                    int L2_insert_result = L2->insert_data(new_address, evicted_block);

                    if (L2_insert_result == FULLCACHE)
                    {
                        cout << "L2 Cache is Full! \n";
                        cout << "Evicting data from L2 to memory...\n";
                        // L2 is Full, we run eviction in L2 and insert older data into memory
                        EvictionBlock evicted_block_2 = L2->evict_block(new_address);

                        cout << "Evicted Block       : ";
                        cout << evicted_block_2.tag_bits << " ";
                        cout << evicted_block_2.valid_bit << " ";
                        cout << evicted_block_2.dirty_bit << " ";
                        cout << evicted_block_2.index_bits << "\n";

                        if (evicted_block_2.dirty_bit == 1)
                        {
                            eviction_to_memory = 1;
                        }
                        else
                        {
                            eviction_to_memory = 0;
                        }

                        // L2 eviction completed, now inserting old data into L2
                        cout << "Data evicted, inserting data from L1 into L2...\n";
                        int L2_insert_result = L2->insert_data(new_address, evicted_block);
                    }

                    cout << "Data evicted, inserting data from memory into L1...\n";
                    // L1 eviction completed, inserting new data into L1
                    int L1_insert_result = L1->insert_data(address);
                }

                if (eviction_to_memory)
                {
                    MemAccessState = WRITEMEM;
                }
                else
                {
                    MemAccessState = NOWRITEMEM;
                }
            }
            else
            {
                cout << "L1 Miss, L2 hit...\n";
                cout << "Moving data from L2 to L1...\n";

                bool eviction_to_memory = 0;

                // We get a L2 Hit, we move the data to L1
                EvictionBlock evicted_block = L2->evict_block(address);

                cout << "Evicted Block       : ";
                cout << evicted_block.tag_bits << " ";
                cout << evicted_block.valid_bit << " ";
                cout << evicted_block.dirty_bit << " ";
                cout << evicted_block.index_bits << "\n";

                bitset<32> new_address = translateToL1Address(evicted_block.tag_bits, evicted_block.index_bits);
                int L1_insert_result = L1->insert_data(new_address, evicted_block);

                if (L1_insert_result == FULLCACHE)
                {
                    cout << "L1 Cache is Full! \n";
                    cout << "Evicting data from L1 to L2...\n";
                    // L1 is Full, we run eviction in L1 and insert data into L2
                    EvictionBlock evicted_block_2 = L1->evict_block(address);

                    cout << "Evicted Block       : ";
                    cout << evicted_block_2.tag_bits << " ";
                    cout << evicted_block_2.valid_bit << " ";
                    cout << evicted_block_2.dirty_bit << " ";
                    cout << evicted_block_2.index_bits << "\n";

                    cout << "Data evicted, inserting old data from L1 into L2...\n";
                    bitset<32> new_address_2 = translateToL2Address(evicted_block_2.tag_bits, evicted_block_2.index_bits);
                    int L2_insert_result = L2->insert_data(new_address_2, evicted_block_2);
                    if (L2_insert_result == FULLCACHE) 
                    {
                        cout << "L2 Cache is Full! \n";
                        cout << "Evicting data from L2 to memory...\n";
                        // L2 is Full, we run eviction in L2 and insert older data into memory
                        EvictionBlock evicted_block_3 = L2->evict_block(new_address_2);

                        cout << "Evicted Block       : ";
                        cout << evicted_block_3.tag_bits << " ";
                        cout << evicted_block_3.valid_bit << " ";
                        cout << evicted_block_3.dirty_bit << " ";
                        cout << evicted_block_3.index_bits << "\n";

                        if (evicted_block_3.dirty_bit == 1)
                        {
                            eviction_to_memory = 1;
                        }
                        else
                        {
                            eviction_to_memory = 0;
                        }

                        // L2 eviction completed, now inserting old data into L2
                        cout << "Data evicted, inserting data from L1 into L2...\n";
                        int L2_insert_result = L2->insert_data(new_address_2, evicted_block_2);
                    }
                    cout << "Data evicted, inserting new data from L2 into L1...\n";
                    int L1_insert_result = L1->insert_data(new_address, evicted_block);
                }

                if (eviction_to_memory)
                {
                    MemAccessState = WRITEMEM;
                }
                else
                {
                    MemAccessState = NOWRITEMEM;
                }
            }
        }
        else
        {
            L2AccessState = NA;
            MemAccessState = NOWRITEMEM;
        }
    }

    void write(bitset<32> address)
    {
        cout << "Writing to L1 Cache \n";
        L1AccessState = L1->write(address);
        if (L1AccessState == WM)
        {
            cout << "Writing to L2 Cache \n";
            L2AccessState = L2->write(address);
            if (L2AccessState == WM)
            {
                MemAccessState = WRITEMEM;
            }
            else
            {
                MemAccessState = NOWRITEMEM;
            }
        }
        else
        {
            L2AccessState = NA;
            MemAccessState = NOWRITEMEM;
        }
    }

    void print_caches(bitset<32> address)
    {
        L1->print_cache(address);
        L2->print_cache(address);
    }
};

int main(int argc, char *argv[])
{
    config cacheconfig;
    ifstream cache_params;
    string dummyLine;
    cache_params.open(argv[1]);
    while (!cache_params.eof()) // read config file
    {
        cache_params >> dummyLine;               // L1:
        cache_params >> cacheconfig.L1blocksize; // L1 Block size
        cache_params >> cacheconfig.L1setsize;   // L1 Associativity
        cache_params >> cacheconfig.L1size;      // L1 Cache Size
        cache_params >> dummyLine;               // L2:
        cache_params >> cacheconfig.L2blocksize; // L2 Block size
        cache_params >> cacheconfig.L2setsize;   // L2 Associativity
        cache_params >> cacheconfig.L2size;      // L2 Cache Size
    }
    ifstream traces;
    ofstream tracesout;
    string outname;
    outname = string(argv[2]) + ".out";
    traces.open(argv[2]);
    tracesout.open(outname.c_str());
    string line;
    string accesstype;     // the Read/Write access type from the memory trace;
    string xaddr;          // the address from the memory trace store in hex;
    unsigned int addr;     // the address from the memory trace store in unsigned int;
    bitset<32> accessaddr; // the address from the memory trace store in the bitset;

    if (cacheconfig.L1blocksize != cacheconfig.L2blocksize)
    {
        printf("please test with the same block size\n");
        return 1;
    }

    cout << "L1 Cache Config : \n";
    cout << "L1 Block Size   : " << cacheconfig.L1blocksize << "\n";
    cout << "L1 Set Size     : " << cacheconfig.L1setsize << "\n";
    cout << "L1 Size         : " << cacheconfig.L1size << "\n";
    cout << "\n";
    cout << "L2 Cache Config : \n";
    cout << "L2 Block Size   : " << cacheconfig.L2blocksize << "\n";
    cout << "L2 Set Size     : " << cacheconfig.L2setsize << "\n";
    cout << "L2 Size         : " << cacheconfig.L2size << "\n";
    cout << "\n";

    CacheSystem CashSys(cacheconfig.L1blocksize, cacheconfig.L1setsize, cacheconfig.L1size, cacheconfig.L2blocksize,
                        cacheconfig.L2setsize, cacheconfig.L2size);

    int L1AcceState = NA;          // L1 access state variable, can be one of NA, RH, RM, WH, WM;
    int L2AcceState = NA;          // L2 access state variable, can be one of NA, RH, RM, WH, WM;
    int MemAcceState = NOWRITEMEM; // Main Memory access state variable, can be either NA or WH;

    int i = 1;

    if (traces.is_open() && tracesout.is_open())
    {
        while (getline(traces, line))
        { // read mem access file and access Cache
            cout << "(" << dec << i << ")\n";
            // if (i > 86)
            // {
            //     break;
            // };

            istringstream iss(line);
            if (!(iss >> accesstype >> xaddr))
            {
                break;
            }
            stringstream saddr(xaddr);
            saddr >> std::hex >> addr;
            accessaddr = bitset<32>(addr);

            // access the L1 and L2 Cache according to the trace;
            if (accesstype.compare("R") == 0) // a Read request
            {
                cout << "Reading : " << accessaddr << "\n";
                CashSys.read(accessaddr);
                L1AcceState = CashSys.L1AccessState;
                L2AcceState = CashSys.L2AccessState;
                MemAcceState = CashSys.MemAccessState;
            }
            else
            {
                cout << "Writing : " << accessaddr << "\n";
                CashSys.write(accessaddr);
                L1AcceState = CashSys.L1AccessState;
                L2AcceState = CashSys.L2AccessState;
                MemAcceState = CashSys.MemAccessState;
            }
            cout << "--------------------------------------------------\n";
            // CashSys.print_caches(accessaddr);
            cout << "--------------------------------------------------\n";
            i++;
            cout << "\n";

            // Grading: don't change the code below.
            // We will print your access state of each cycle to see if your simulator
            // gives the same result as ours.
            tracesout << L1AcceState << " " << L2AcceState << " " << MemAcceState
                      << "\n"; // Output hit/miss results for L1 and L2 to the output file;
        }
        traces.close();
        tracesout.close();
    }
    else
        cout << "Unable to open trace or traceout file ";

    return 0;
}
