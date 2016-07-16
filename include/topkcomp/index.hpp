#pragma once

#include "index1.hpp"
#include "index2.hpp"
#include "index3.hpp"
#include "index4.hpp"
#include "index4ci.hpp"
#include "index5.hpp"

#include <string>
#include <vector>
#include <chrono>
#include <algorithm>
#include <iostream>


namespace topkcomp{

    template<typename t_index>
    void
    generate_index_from_file(t_index& index,
                             const std::string& file,
                             const std::string& index_file,
                             const std::string& index_name) 
    {
        using namespace std;
        using namespace sdsl;
        constexpr bool case_sensitive = t_index::case_sensitive;
        using clock = chrono::high_resolution_clock;
        if ( load_from_file(index, index_file) ){
            cout << "Load index from "<<index_file << endl;
        } else {
            cout << "Index of " << file << " does not exists." << endl;
            cout << "Start generation" << endl;
            ifstream in(file.c_str());
            if ( !in ) {
                cerr << "Error: Could not open file " << file << endl;
                return;
            }
            tVPSU string_weight;
            string entry;
            while ( getline(in, entry, '\t')  ) {
                string s_weight;
                getline(in, s_weight);
                uint64_t weight = stoull(s_weight);
                string_weight.emplace_back(entry, weight);
            }
            if ( case_sensitive ) {
                sort(string_weight.begin(), string_weight.end());
            } else {
                sort(string_weight.begin(), string_weight.end(),[](const tPSU& a, const tPSU& b){
                    const uint8_t* ap = (const uint8_t*)(a.first.c_str());
                    const uint8_t* bp = (const uint8_t*)(b.first.c_str());
                    while ( std::tolower(*ap) == std::tolower(*bp) ) {
                        if ( *ap == 0 ) {
                            return false;
                        } else {
                            ++ap; ++bp;
                        }
                    }
                    return std::tolower(*ap) < std::tolower(*bp);
                });
            }
            cout << "Read and sorted " << string_weight.size() << " string." << endl;
            if ( case_sensitive ) {
                auto unique_end = unique(string_weight.begin(), string_weight.end(),
                                         [](const tPSU& a, const tPSU& b) {
                                            return a.first == b.first;
                                         });
                string_weight.resize(unique_end-string_weight.begin());
            } else {
                auto unique_end = unique(string_weight.begin(), string_weight.end(),
                                         [](const tPSU& a, const tPSU& b) {
                                            if ( a.first.size() != b.first.size() )
                                                return false;
                                            const uint8_t* ap = (const uint8_t*)(a.first.c_str());
                                            const uint8_t* bp = (const uint8_t*)(b.first.c_str());
                                            while ( std::tolower(*ap) == std::tolower(*bp) ) {
                                                if ( *ap == 0 ) {
                                                    return true;
                                                } else {
                                                    ++ap; ++bp;
                                                }
                                            }
                                            return false;
                                         });
                string_weight.resize(unique_end-string_weight.begin());
            }
            cout << "Number of unique strings is " << string_weight.size() << "." << endl;
/*            {
                ofstream out(file+".unique.txt");
                for(size_t i=0; i<string_weight.size(); ++i){
                    out << string_weight[i].first << "\t"<<string_weight[i].second << "\n";
                }
            }
*/
            {
                auto construction_start = clock::now();
                t_index topk_index(string_weight);
                auto construction_time = clock::now() - construction_start;
                auto construction_ms    = chrono::duration_cast<chrono::milliseconds>(construction_time).count();
                cout << "Construction took "<< std::setprecision(3) << construction_ms / 1000.0;
                cout << " s" << endl;
                store_to_file(topk_index, index_file);
                write_structure<HTML_FORMAT>(topk_index, file+"."+index_name+".html");
                cout << "Index size is " << size_in_mega_bytes(topk_index) << " MiB" << endl;
            }
            load_from_file(index, index_file);
        }
    }

} // end namespace
