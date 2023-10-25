//
//  main.cpp
//  TSN_2023_Parallel
//
//  Created by pardo jérémie on 14/10/2023.
//

#include <iostream>
#include <chrono>
#include <fstream>
#include <functional>

#include "workerPool.hpp"

auto wrong_input_msg (const char * program_call) {
    std::cerr << "Usage: " << program_call << " <input> <output> [-b <B>] [-t <T>] [-p]\n";
    return EXIT_FAILURE;
}


void func_partial_cp (const std::string infile_str, const std::string outfile_str, uint_fast64_t blocksSize, const uint_fast64_t infile_size, const uint_fast64_t pos_blocksSize) {    // allocate memory for file content
    if (infile_size < blocksSize + pos_blocksSize)
        blocksSize = infile_size - pos_blocksSize;

    char* buffer = new char[blocksSize];

    // read content of infile
    std::ifstream infile (infile_str,std::ifstream::binary);
    infile.seekg(pos_blocksSize);
    infile.read (buffer,blocksSize);
    infile.close();

    // write to outfile
    std::fstream outfile(outfile_str, std::ios::binary | std::ios::in| std::ios::out | std::ios::ate);
    outfile.seekp(pos_blocksSize);
    outfile.write (buffer,blocksSize);
    outfile.close();
}

int main(int argc, const char * argv[]) {
    // 1 - creer les taches
    // * parse input param
    if (argc < 3)
        return wrong_input_msg (argv[0]);
    
    auto infile_str = std::string(argv[1]);
    auto outfile_str = std::string(argv[2]);
    auto printStates = false;
    
    uint_fast16_t nbWorkers = 4;
    uint_fast64_t blocksSize = 0, infile_size = 0;
    
    for(uint_fast8_t i = 3; i < argc; i++) {
        auto param = std::string(argv[i]);

        if(param == "-p")
            printStates = true;
        else if (i+1 < argc) { // two params input
            auto param2 = strtol(argv[++i],NULL,10); // If no valid conversion could be performed, a zero value is returned from strtol
            
            if (param2 > 0 && param == "-b") // param2 must be positif
                blocksSize = static_cast<uint_fast64_t>(param2);
            else if (param2 > 0 && param == "-t") // param2 must be positif
                nbWorkers = static_cast<uint_fast16_t>(param2);
            else
                return wrong_input_msg (argv[0]);
        }
        else
            return wrong_input_msg (argv[0]);
    }
    
    // * check if the input file exist and find its size
    std::ifstream infile (infile_str,std::ifstream::binary);
    if(! infile.is_open()) {
        std::cerr << "bad input file";
        return EXIT_FAILURE;
    }
    
    infile.seekg (0, infile.end);
    infile_size = infile.tellg();
    infile.close();
    
    // * force the output file to the correct size
    std::ofstream outfile (outfile_str,std::ofstream::binary );
    if(infile_size > 0) {
        outfile.seekp(infile_size-1);
        outfile<<'\0';
    }
    outfile.close();
    
    // * create pool
    WorkerPool pool(nbWorkers);
    
    // * calculate blocksSize if param -b not was not used
    if(!blocksSize)
        blocksSize = infile_size / nbWorkers;
    
    // 2 - demarrer chrono
    auto start = std::chrono::steady_clock::now();
    

    // 3 - ajouter toutes les taches
    for(uint_fast64_t pos = 0; pos < infile_size; pos += blocksSize)
        pool.add_task(func_partial_cp,infile_str, outfile_str, blocksSize, infile_size, pos);
    
    // 4 - attendre terminaison des taches
    pool.wait_all();
    
    // 5 - arreter chrono
    auto end = std::chrono::steady_clock::now();
    
    // 6 - afficher chrono
    std::cout << "Elapsed time in nanoseconds: "
            << std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count()
            << " ns\n";
    
    // 7 - afficher stats (si besoin)
    if(printStates)
        pool.printStates();
}
