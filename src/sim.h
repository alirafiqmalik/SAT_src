#ifndef _SIM_H_DEFINED_
#define _SIM_H_DEFINED_

#include <vector>
#include "ckt.h"
#include "util.h"
#include "SATInterface.h"

namespace ckt_n {
    typedef std::vector<bool> bool_vec_t;

    struct simulator_t {
        virtual void eval(
            const std::vector<bool>& input_values,
            std::vector<bool>& output_values
        ) = 0;
    };

    struct eval_t {
        sat_n::Solver S;
        index2lit_map_t mappings;
        ckt_t& ckt;

        eval_t(ckt_t& c);
        ~eval_t() {}

        void set_cnst(node_t* n, int val);
        void eval(nodelist_t& input_nodes, const bool_vec_t& input_values, bool_vec_t& outputs);
    };

    struct ckt_eval_t : public simulator_t {
        eval_t sim;
        nodelist_t& inputs;

        ckt_eval_t(ckt_t& c, nodelist_t& inps) 
            : sim(c)
            , inputs(inps)
        {
            for(unsigned i=0; i != c.num_key_inputs(); i++) {
                set_cnst(c.key_inputs[i], 0);
            }
        }

        void set_cnst(node_t* n, int val) { sim.set_cnst(n, val); }
        virtual void eval(
            const std::vector<bool>& inputs,
            std::vector<bool>& outputs
        );
    };

    struct enc_ckt_eval_t : public simulator_t {
        eval_t sim;
        nodelist_t all_inputs;
        std::vector<bool> all_input_values;
        unsigned num_key_inputs;

        enc_ckt_eval_t(ckt_t& c)
            : sim(c)
            , num_key_inputs(c.num_key_inputs())
        {
            std::copy(c.key_inputs.begin(), c.key_inputs.end(), back_inserter(all_inputs));
            std::copy(c.ckt_inputs.begin(), c.ckt_inputs.end(), back_inserter(all_inputs));
            all_input_values.resize(all_inputs.size());
        }

        void set_key_inputs(std::vector<bool>& key_input_values) {
            std::copy(key_input_values.begin(), key_input_values.end(), all_input_values.begin());
        }

        virtual void eval(const std::vector<bool>& inputs, std::vector<bool>& outputs);
    };

    void convert(uint64_t v, bool_vec_t& result);
    uint64_t convert(const bool_vec_t& v);

    void dump_mappings(
        std::ostream& out, ckt_t& ckt, index2lit_map_t& mappings);
}

#endif
