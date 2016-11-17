#include "topkcomp/index.hpp"
#include <cstdio>
#include <cstring>
#include <iostream>
#include <string>
#include <vector>
#include <chrono>

using namespace std;
using namespace sdsl;
using namespace topkcomp;

typedef INDEX_TYPE t_index;


void write_in_file(const std::string& filename, const std::string& prefix, tVPSU& string_weight) {
    std::ofstream out(filename, std::ios_base::app | std::ios_base::out);
    if ( !out ) {
        std::cerr << "Error: Could not open file " << filename << endl;
        exit(EXIT_FAILURE);
    }
    out << prefix << "\n";
    if ( string_weight.size() > 0 )
        for (auto it = string_weight.begin(); it != string_weight.end(); it++) {
            out << it->first << " " << it->second << "\n";
        }
}

void trunc_file(const std::string& filename) {
    std::ofstream ofs(filename, std::ofstream::out | std::ofstream::trunc);
}

void build_strings(std::string file, std::vector<std::string> &strings) {
    std::ifstream in(file);
    for (std::string entry; getline(in, entry); ) {
        strings.push_back(std::move(entry));
    }
}

int main(int argc, char* argv[]){
    using clock = chrono::high_resolution_clock;
    const string index_name = INDEX_NAME;
    const string index_file = std::string(argv[1])+"."+INDEX_NAME+".sdsl";
    if ( argc < 2 ) {
        cout << "Usage1: ./" << argv[0] << " File" << endl;
        cout << "  Constructs a top-k completion index." << endl;
        cout << "  The index will be stored in File.";
        cout << index_name << ".sdsl" << endl << endl;
        
        cout << "Usage2: ./" << argv[0] << " File -test K TestCaseFile" << endl;
        return 1;
    }
    
    t_index topk_index;
    generate_index_from_file(topk_index, argv[1], index_file, index_name);
    
    bool test = false;
    if ( argc == 5 && strcmp(argv[2], "-test") == 0 ) {
        test = true;
    }

    if ( !test ) {
        std::string word;
        cout << "Please enter queries line by line." << endl;
        cout << "Pressing Crtl-D will quit the program." << endl;
        string prefix;
        while ( getline(cin, prefix) ) {
            word.resize(prefix.size());
            std::transform(prefix.begin(), prefix.end(), word.begin(), ::tolower);
            auto query_start = chrono::high_resolution_clock::now(); 
            auto result_list = topk_index.top_k(word, 5);
            auto query_time  = chrono::high_resolution_clock::now() - query_start;
            auto query_us    = chrono::duration_cast<chrono::microseconds>(query_time).count();
            
            cout << "-- top results:" << endl;
            for (size_t i=0; i<result_list.size(); ++i) {
                cout << result_list[i].first << "  " << result_list[i].second << endl;
            }
            cout << "-- (" << std::setprecision(3) << query_us / 1000.0;
            cout << " ms)" << endl;
        }
    } else {
        
        int k = atoi(argv[3]);
        std::string word;
        std::string out_file("outfile."+index_name);
        std::string test_file(argv[4]);
        std::vector<std::string> prefixes;
        build_strings(test_file, prefixes);
        double total_us = 0;
        size_t found = 0;
        trunc_file(out_file);
        auto start = chrono::high_resolution_clock::now();
        for ( auto& prefix : prefixes ) {
            word.resize(prefix.size());
            std::transform(prefix.begin(), prefix.end(), word.begin(), ::tolower);
            auto query_start = chrono::high_resolution_clock::now(); 
            auto result_list = topk_index.top_k(word, k);
            auto query_time  = chrono::high_resolution_clock::now() - query_start;
            auto query_us    = chrono::duration_cast<chrono::microseconds>(query_time).count();
            total_us        += query_us;
            write_in_file(out_file, prefix, result_list);
            found           += result_list.size();
        } 
        auto end = chrono::high_resolution_clock::now();
        auto average = total_us/prefixes.size();
        
        std::ifstream t(out_file);
        std::stringstream buffer;
        buffer << t.rdbuf();
        std::remove(out_file.c_str());
        
        std::hash<std::string> hash_fn;
        size_t buffer_hash = hash_fn(buffer.str());
        std::cout << k << "\t" << average << "\t" << found << "\t" << prefixes.size() << "\t" << buffer_hash << endl;
    }
}
