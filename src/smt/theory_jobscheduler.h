/*++
Copyright (c) 2018 Microsoft Corporation

Module Name:

    theory_jobscheduling.h

Abstract:

    Propagation solver for jobscheduling problems.
    It relies on an external module to tighten bounds of 
    job variables.

Author:

    Nikolaj Bjorner (nbjorner) 2018-09-08.

Revision History:

--*/
#pragma once;

#include "smt/smt_theory.h"
#include "ast/jobshop_decl_plugin.h"
#include "ast/arith_decl_plugin.h"

namespace smt {

    typedef uint64_t time_t;

    class theory_jobscheduler : public theory {

        struct job_resource {
            unsigned m_resource_id;   // id of resource
            unsigned m_capacity;      // amount of resource to use
            unsigned m_loadpct;       // assuming loadpct
            time_t   m_end;           // must run before
            job_resource(unsigned r, unsigned cap, unsigned loadpct, time_t end):
                m_resource_id(r), m_capacity(cap), m_loadpct(loadpct), m_end(end) {}
        };

        struct job_time {
            unsigned m_job;
            time_t   m_time;
            job_time(unsigned j, time_t time): m_job(j), m_time(time) {}

            struct compare {
                bool operator()(job_time const& jt1, job_time const& jt2) const {
                    return jt1.m_time < jt2.m_time;
                }
            };
        };

        struct job_info {
            vector<job_resource> m_resources; // resources allowed to run job.
            u_map<unsigned>      m_resource2index; // resource to index into vector 
            enode*               m_start;
            enode*               m_end;
            enode*               m_resource;
            job_info(): m_start(nullptr), m_end(nullptr), m_resource(nullptr) {}
        };

        struct res_available {
            unsigned m_loadpct;
            time_t m_start;
            time_t m_end;
            res_available(unsigned load_pct, time_t start, time_t end):
                m_loadpct(load_pct),
                m_start(start),
                m_end(end)
            {}
            struct compare {
                bool operator()(res_available const& ra1, res_available const& ra2) const {
                    return ra1.m_start < ra2.m_start;
                }
            };

        };

        struct res_info {
            unsigned_vector       m_jobs;      // jobs allocated to run on resource
            vector<res_available> m_available; // time intervals where resource is available
            time_t              m_end;       // can't run after
            res_info(): m_end(std::numeric_limits<time_t>::max()) {}
        };
        
        ast_manager&     m;
        jobshop_util     u;
        arith_util       a;
        unsigned_vector  m_var2index;
        vector<job_info> m_jobs;
        vector<res_info> m_resources;
        
    protected:

        theory_var mk_var(enode * n) override;        

        bool internalize_atom(app * atom, bool gate_ctx) override { return false; }

        bool internalize_term(app * term) override;

        void assign_eh(bool_var v, bool is_true) override {}

        void new_eq_eh(theory_var v1, theory_var v2) override {}

        void new_diseq_eh(theory_var v1, theory_var v2) override {}

        void push_scope_eh() override;

        void pop_scope_eh(unsigned num_scopes) override;

        final_check_status final_check_eh() override;

        bool can_propagate() override;

        void propagate() override;
        
    public:

        theory_jobscheduler(ast_manager& m);

        ~theory_jobscheduler() override {}
        
        void display(std::ostream & out) const override;
        
        void collect_statistics(::statistics & st) const override;

        void init_model(model_generator & m) override;

        model_value_proc * mk_value(enode * n, model_generator & mg) override;

        bool get_value(enode * n, expr_ref & r) override;

        theory * mk_fresh(context * new_ctx) override; // { return alloc(theory_jobscheduler, new_ctx->get_manager()); }

    public:
        // assignments:
        time_t est(unsigned j);      // earliest start time of job j
        time_t lst(unsigned j);      // last start time
        time_t ect(unsigned j);      // earliest completion time
        time_t lct(unsigned j);      // last completion time
        time_t start(unsigned j);    // start time of job j
        time_t end(unsigned j);      // end time of job j
        unsigned resource(unsigned j); // resource of job j
        
        // set up model
        void add_job_resource(unsigned j, unsigned r, unsigned cap, unsigned loadpct, time_t end);
        void add_resource_available(unsigned r, unsigned max_loadpct, time_t start, time_t end);
        void add_done();

        // validate assignment
        void validate_assignment();
        bool resource_available(unsigned r, time_t t, unsigned& load_pct, time_t& end, unsigned& idx); // load available on resource r at time t.
        time_t capacity_used(unsigned j, unsigned r, time_t start, time_t end);        // capacity used between start and end

        job_resource const& get_job_resource(unsigned j, unsigned r) const;

        std::ostream& display(std::ostream & out, res_info const& r) const;
        std::ostream& display(std::ostream & out, res_available const& r) const;
        std::ostream& display(std::ostream & out, job_info const& r) const;
        std::ostream& display(std::ostream & out, job_resource const& r) const;

    };    
};

