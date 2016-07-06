#pragma once

#include "index_common.hpp"
#include <sdsl/bit_vectors.hpp>
#include <algorithm>
#include <queue>
#include <locale> // required by std::tolower

namespace topkcomp {

template<typename t_bv = sdsl::bit_vector,
         typename t_sel= typename t_bv::select_1_type>
class index2 {
    sdsl::int_vector<8> m_text;  // stores the concatenation of all entries
    sdsl::int_vector<>  m_priority; // priorities of entries
    t_bv                m_start_bv;  // bitvector which represents the start of entries in m_text
    t_sel               m_start_sel; // select structure for m_start_bv

    public:
        typedef size_t size_type;

        // Constructor
        index2(const tVSI& entry_priority=tVSI()) {
            if ( entry_priority.size() == 0 ) {
                return;
            }
            // get the length of the concatenation of all strings
            uint64_t n = std::accumulate(entry_priority.begin(), entry_priority.end(),
                            0, [](uint64_t a, std::pair<std::string, uint64_t> ep){
                                    return a + ep.first.size();
                               });
            // get maximum of priorities
            auto max_priority = std::max_element(entry_priority.begin(), entry_priority.end(),
                                    [] (const tSI& a, const tSI& b){
                                        return a.second < b.second;
                                    })->second;
            // initialize m_text
            m_text = sdsl::int_vector<8>(n);
            // initialize bitvector for starting positions
            sdsl::bit_vector start(n+1, 0);
            // initialize m_priority
            m_priority = sdsl::int_vector<>(entry_priority.size(), 0, sdsl::bits::hi(max_priority)+1);

            size_t idx = 0;
            for (size_t i=0; i < entry_priority.size(); ++i) {
                start[idx] = 1;
                m_priority[i] = entry_priority[i].second;
                for (auto c : entry_priority[i].first) {
                    m_text[idx++] = c;
                }
            }
            start[idx] = 1;
            m_start_bv = t_bv(std::move(start));
            m_start_sel = t_sel(&m_start_bv);
        }

        // Return range [lb, rb) of matching entries
        std::array<size_t,2> prefix_range(const std::string& prefix) const {
            std::array<size_t,2> res = {{0, m_priority.size()}};
            id_rac id(m_priority.size());
            for (size_t i=0; i<prefix.size(); ++i) {
                // use binary search at each step to narrow the interval
                res[0] = std::lower_bound(id.begin()+res[0], id.begin()+res[1],
                            prefix[i],  [&](size_t idx, char c){
                                return m_text[m_start_sel(idx+1)+i] < c;
                            }) - id.begin();
                res[1] = std::upper_bound(id.begin()+res[0], id.begin()+res[1],
                            prefix[i],  [&](char c, size_t idx){
                                return c < m_text[m_start_sel(idx+1)+i];
                            }) - id.begin();
            }
            return res;
        }


        // k > 0
        tVSI top_k(const std::string& prefix, size_t k) const {
            auto range = prefix_range(prefix);

            tVSI result_list;
            // min-priority queue holds (priority, index)-pairs
            std::priority_queue<tII, std::vector<tII>, std::greater<tII>> pq;
            for (size_t i=range[0]; i<range[1]; ++i){
                if ( pq.size() < k ) {
                    pq.emplace(m_priority[i], i);
                } else if ( m_priority[i] > pq.top().first ) {
                    pq.pop();
                    pq.emplace(m_priority[i], i);
                }
            }
            while ( !pq.empty() ) {
                auto idx = pq.top().second;
                auto entry = std::string(m_text.begin()+m_start_sel(idx+1), m_text.begin()+m_start_sel(idx+2));
                result_list.emplace_back(entry, m_priority[idx]);
                pq.pop();
            }
            std::reverse(result_list.begin(), result_list.end());
            return result_list; 
        }

        // Serialize method
        size_type
        serialize(std::ostream& out, sdsl::structure_tree_node* v=nullptr,
                  std::string name="") const {
            using namespace sdsl;
            structure_tree_node* child = structure_tree::add_child(v, name, util::class_name(*this));
            size_type written_bytes = 0;
            written_bytes += m_text.serialize(out, child, "text");
            written_bytes += m_start_bv.serialize(out, child, "start_bv");
            written_bytes += m_start_sel.serialize(out, child, "start_sel");
            written_bytes += m_priority.serialize(out, child, "priority");
            structure_tree::add_size(child, written_bytes);
            return written_bytes;
        }

        // Load method
        void load(std::istream& in) {
            m_text.load(in);
            m_start_bv.load(in);
            m_start_sel.load(in);
            m_start_sel.set_vector(&m_start_bv);
            m_priority.load(in);
        }
};

} // end namespace topkcomp
