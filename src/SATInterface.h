#ifndef _SATINTERFACE_H_DEFINED_
#define _SATINTERFACE_H_DEFINED_

#include <stdint.h>
#include <lglib.h>
#include <cmsat/Solver.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <set>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <iostream>
#include <fstream>
#include "gurobi_c++.h"

// mvt edits: search for "UsingCplex" in this file

namespace sat_n
{
//#define CMSAT
#ifdef CMSAT
    typedef CMSat::Var Var;
    typedef CMSat::Lit Lit;
    typedef CMSat::lbool lbool;
    typedef CMSat::vec<Lit> vec_lit_t;
    const CMSat::lbool l_True = CMSat::l_True;
    const CMSat::lbool l_False = CMSat::l_False;
    const CMSat::lbool l_Undef = CMSat::l_Undef;

    inline Lit mkLit(Var vi) { 
        return CMSat::Lit(vi, false);
    }

    inline Var var(Lit l) {
        return l.var();
    }

    inline bool sign(Lit l) {
        return l.sign();
    }

    inline uint32_t toInt(Lit l) {
        return l.toInt();
    }

    //stupid sentinel function, I added to check if lingeling was being compiled.
    //inline void dostuff() {
    //    LGL* lgl = lglinit();
    //    lglrelease(lgl);
    //}

    class Solver {
        CMSat::Solver S;

    public:
        Solver() {}
        ~Solver() {}

        Var newVar() { return S.newVar(); }
        int nVars() const { return S.nVars(); }
        int nClauses() const { return S.nClauses(); }

        // freeze a single literal.
        void freeze(Lit l) { }
        // freeze a vector of literals.
        void freeze(const std::vector<Lit>& ys) { }
        
        bool addClause(const vec_lit_t& ps) {
            vec_lit_t ps_copy(ps);
            return S.addClause(ps_copy);
        }
        bool addClause(Lit x) { 
            vec_lit_t vs;
            vs.push(x);
            return S.addClause(vs); 
        }
        bool addClause(Lit x, Lit y) { 
            vec_lit_t vs;
            vs.push(x);
            vs.push(y);
            return S.addClause(vs);
        }
        bool addClause(Lit x, Lit y, Lit z) {
            vec_lit_t vs;
            vs.push(x);
            vs.push(y);
            vs.push(z);
            return S.addClause(vs);
        }

        lbool modelValue(Lit x) const {
            return S.modelValue(x);
        }
        lbool modelValue(Var x) const {
            return S.modelValue(mkLit(x));
        }
        bool solve() { 
            lbool result = S.solve(); 
            if(result.isDef()) {
                return result.getBool();
            } else {
                assert(false);
                return false;
            }
        }
        bool solve(const vec_lit_t& assump) { 
            lbool result = S.solve(assump); 
            if(result.isDef()) {
                return result.getBool();
            } else {
                assert(false);
                return false;
            }
        }
        bool solve(Lit x) { 
            vec_lit_t assump;
            assump.push(x);
            lbool result = S.solve(assump); 
            if(result.isDef()) {
                return result.getBool();
            } else {
                assert(false);
                return false;
            }
        }
        void writeCNF(const std::string& filename) {
            S.dumpOrigClauses(filename);
        }
        int64_t getNumDecisions() const {
            return S.getNumDecisions();
        }
    };
#else

    typedef CMSat::Var Var;
    typedef CMSat::Lit Lit;
    typedef CMSat::vec<Lit> vec_lit_t;

    // homemade lbool.
    struct lbool {
        enum x_t { p_True = 1, p_Undef = 0, p_False = -1 } x;
        lbool() : x((x_t)0) { }

        lbool(int val) : x((x_t)val) {
            assert(x == p_True || x == p_Undef || x == p_False);
        }

        inline bool isDef() { 
            return x != p_Undef;
        }
        inline bool isUndef() {
            return x == p_Undef;
        }

        inline bool getBool() {
            assert(isDef());
            return (x == p_True);
        }

        bool operator==(const lbool& other) const {
            return x == other.x;
        }
        bool operator!=(const lbool& other) const {
            return x != other.x;
        }
    };

    extern const lbool l_True;
    extern const lbool l_False;
    extern const lbool l_Undef;

    inline Lit mkLit(Var vi) { 
        return CMSat::Lit(vi, false);
    }

    inline Var var(Lit l) {
        return l.var();
    }

    inline bool sign(Lit l) {
        return l.sign();
    }

    inline uint32_t toInt(Lit l) {
        return l.toInt();
    }

    class Solver {
        LGL* solver;
        uint32_t numVars;

        static int translate(Lit l) {
            int v = (var(l) + 1);
            return sign(l) ? -v : v;
        }
        static int translate(Var v) {
            assert(v >= 0);
            return v+1;
        }

        void printSet(std::set<int> s) {
            std::cout << "\t{";
            for(std::set<int>::iterator it = s.begin(); it != s.end(); ++it) {
                std::cout << *it << " ";
            }
            std::cout << "}" << std::endl;
        }

        /* Everything in b that's not in a. There *should* be nothing in
           a that's not in b */
        void printDiff(std::set<std::set<int>> a, std::set<std::set<int>> b) {
            std::cout << "--------------------" << std::endl;
            std::cout << "In a, not in b: ";
            int ina = 0;
            for(std::set<std::set<int>>::iterator it = a.begin(); it != a.end(); ++it) {
                std::set<int> s = *it;
                if(b.find(s) == b.end()) {
                    //printSet(s);
                    ina++;
                }
            }
            std::cout << ina << std::endl << std::flush;
            std::cout << "--------------------" << std::endl;
            std::cout << "In b, not in a: ";
            int inb = 0;
            for(std::set<std::set<int>>::iterator it = b.begin(); it != b.end(); ++it) {
                std::set<int> s = *it;
                if(a.find(s) == a.end()) {
                    //printSet(s);
                    inb++;
                }
            }
            std::cout << inb << std::endl << std::flush;
            std::cout << "--------------------" << std::endl;
        }

        bool _solve() { 
            lglsetopt(solver, "dlim", -1);
            int result = lglsat(solver);

            if(result == LGL_SATISFIABLE) {
                return true;
            } else if(result == LGL_UNSATISFIABLE) {
                return false;
            } else {
                assert(false);
                return false;
            }
        }

    public:
        // Constructor.
        Solver() 
        { 
            solver = lglinit(); 
            numVars = 0; 

            nvarsUsingCplex = 0;
            varValUsingCplex = NULL;

            gurodata = new GURO;
            gurodata->guroenv = new GRBEnv();
            gurodata->guroenv->set(GRB_IntParam_OutputFlag, 0);
            gurodata->guromodel = new GRBModel(*(gurodata->guroenv));

            gurodata->guronvars = 0;
            gurodata->guroclauses.clear();
            gurodata->guroccmap.clear();
            gurodata->guroassumps.clear();
        }
        // Destrucutor.
        ~Solver() { 
            lglrelease(solver); 

            gurodata->guroclauses.clear();
            gurodata->guroccmap.clear();
            gurodata->guroassumps.clear();
            delete gurodata->guromodel;
            delete gurodata->guroenv;
            delete gurodata;
        }

        // Create a new variable.
        Var newVar() { 
            // No need to create vars in lgl, just do our own bookkeeping.
            Var v = numVars;
            numVars += 1;
            return v;
        }
        // Number of variables created so far.
        int nVars() const { return numVars; }
        // Number of clauses added so far.
        int nClauses() const { return lglnclauses(solver); }

        // freeze a single literal.
        void freeze(Lit l) {
            lglfreeze(solver, translate(l));
        }

        // freeze a vector of literals.
        void freeze(const std::vector<Lit>& ys) {
            for(unsigned i=0; i != ys.size(); i++) {
                lglfreeze(solver, translate(ys[i]));
            }
        }

        // Add a vector of literals as a clause.
        bool addClause(const vec_lit_t& ps) {
            for(unsigned i=0; i != ps.size(); i++) {
                lgladd(solver, translate(ps[i]));
            }
            lgladd(solver, 0);
            return true;
        }

        // Add a single literal as a clause.
        bool addClause(Lit x) { 
            lgladd(solver, translate(x));
            lgladd(solver, 0);
            return true;
        }

        // Add a clause with two literals.
        bool addClause(Lit x, Lit y) { 
            lgladd(solver, translate(x));
            lgladd(solver, translate(y));
            lgladd(solver, 0);
            return true;
        }

        // Add a clause with three literals.
        bool addClause(Lit x, Lit y, Lit z) {
            lgladd(solver, translate(x));
            lgladd(solver, translate(y));
            lgladd(solver, translate(z));
            lgladd(solver, 0);
            return true;
        }

        // Return the value of a literal.
        lbool modelValue(Lit x) const {
            int v = lglderef(solver, translate(x));
            if(v == 1) return l_True;
            else if(v == -1) return l_False;
            else return l_Undef;
        }

        // Return the value of a variable.
        lbool modelValue(Var x) const {
            int v = lglderef(solver, translate(x));
            if(v == 1) return l_True;
            else if(v == -1) return l_False;
            else return l_Undef;
        }

        /*
            The following added, each with suffix UsingCplex:
            (a) Data members:
                (i) nvars -- num boolean vars in CNF SAT instance
                (ii) lbool *varVal -- lbool value of a variable
            (b) 5 methods for modelValue() and solve(), and some other helper methods that should
                really be private.
        */
        int nvarsUsingCplex;
        bool *varValUsingCplex;

        lbool modelValueUsingCplex(Lit x) const {
            int l = translate(x);
            if(l < 0) l *= -1;
            if(varValUsingCplex[l-1]) return l_True;
            else return l_False;
        }

        lbool modelValueUsingCplex(Var x) const {
            int v = translate(x);
            if(varValUsingCplex[v-1]) return l_True;
            else return l_False;
        }

        static void get_clauses(void *p, int lit) {
            std::string *s = static_cast<std::string *>(p);
            (*s) += std::to_string(lit);
            if(lit) (*s) += " ";
            else (*s) += "\n";
        }

        void clauses_to_lp(FILE *fp) {
            std::string clauses;
            lgltravall(solver, (void *)(&clauses), get_clauses);
            const char *clauses_cstr = clauses.c_str();

            fprintf(fp, "Maximize\n obj: x1\nSubject To\n");
            nvarsUsingCplex = 0;
            if(varValUsingCplex != NULL) {
                delete varValUsingCplex; varValUsingCplex = NULL;
            }

            char readstr[64];
            int readstrlen = 0;
            for(int nneg = 0, isfirst = 1; sscanf(&(clauses_cstr[readstrlen]), "%s", readstr) > 0; ) {
                int lit = atoi(readstr);
                readstrlen += strlen(readstr); readstrlen++;

                if(lit == 0) {
                    // End of clause
                    fprintf(fp, " > %d\n", (1 - nneg));
                    nneg = 0; isfirst = 1;
                }
                else {
                    if (lit < 0) {
                        fprintf(fp, " - "); lit *= -1; nneg++;
                    }
                    else {
                        fprintf(fp, " ");
                        if(!isfirst) {
                            fprintf(fp, "+ ");
                        }
                    }
                    fprintf(fp, "x%d", lit);
                    if(lit > nvarsUsingCplex) nvarsUsingCplex = lit;

                    isfirst = 0;
                }

            }
        }

        void suffix_to_lp(FILE *fp) {
            fprintf(fp, "Binary\n");
            for(int i = 0; i < nvarsUsingCplex; i++) {
                fprintf(fp, " x%d\n", i+1);
            }
            fprintf(fp, "End\n");
        }

        void literal_to_lp(const Lit l, FILE *fp) {
            int v = translate(l);
            int vpos;

            if(v < 0) vpos = v * -1;
            else vpos = v;

            std::string stra = " x";
            stra += std::to_string(vpos);
            stra += " = ";

            if(v < 0) {
                stra += "0\n";
            }
            else {
                stra += "1\n";
            }
            fprintf(fp, "%s", stra.c_str());

            if(vpos > nvarsUsingCplex) nvarsUsingCplex = vpos;
        }

        void assumptions_to_lp(const vec_lit_t& assump, FILE *fp) {
            for(unsigned int i = 0; i < assump.size(); i++) {
                Lit l = assump[i];
                literal_to_lp(l, fp);
            }
        }

        bool cplex_lp(std::string cplxinstance) {
#if 0
            std::cout << "cplex_lp" << std::endl << std::flush;
#endif
            varValUsingCplex = new bool[nvarsUsingCplex];
            for(int i = 0; i < nvarsUsingCplex; i++) {
                varValUsingCplex[i] = false;
            }

            std::string runcplex = "/home/tripunit/LogicLocking-Empirical/SAT/sat-to-ilp/rungurobi";
            std::string cplxresult = cplxinstance;
            cplxresult += ".sol";

#if 1
            std::string cmd = runcplex;
            cmd += " ";
            cmd += cplxinstance;
            cmd += " > /dev/null";
            if(system(cmd.c_str())) {
            }
#else
            if(fork()) {
                if(wait((int *)0)) { }
            }
            else {
                if(execl(runcplex.c_str(), runcplex.c_str(), cplxinstance.c_str(), (char *)0)) { }
            }
#endif

            FILE *fp = fopen(cplxresult.c_str(), "r");
            bool sat = false;
            for(char readstr[64]; fscanf(fp, "%s", readstr) > 0; ) {
                if(readstr[0] != 'x') {
                    continue;
                }

                if(strlen(readstr) < 2 || readstr[1] < '1' || readstr[1] > '9') {
                    continue;
                }

                sat = true;
                int varnum = atoi(&(readstr[1]));

                // read the value
                if(fscanf(fp, "%s", readstr)) { }
#if 0
                std::cout << "readstr: " << "x" << varnum << " = " << readstr << std::endl << std::flush;
#endif
                int x;
                sscanf(readstr, "%d", &x);
                if(x == 1) varValUsingCplex[varnum-1] = true;
                else varValUsingCplex[varnum-1] = false;
            }

            fclose(fp); fp = NULL;
#if 0
            std::cout << "nvarsUsingCplex: " << nvarsUsingCplex << std::endl << std::flush;
            for(int i = 0; i < nvarsUsingCplex; i++) {
                std::cout << varValUsingCplex[i] << " " << std::flush;
            }
            std::cout << std::endl << std::flush;
            std::cout << "sat = " << sat << std::endl << std::flush;
#endif
            return sat;
        }

        bool solveUsingCplex() {
            std::string cplxinstance = "cplxinstance.lp";
            FILE *fp = fopen(cplxinstance.c_str(), "w");

            clauses_to_lp(fp);
            suffix_to_lp(fp);

            fclose(fp); fp = NULL;

            // Now that the lp instance is in a file, run
            // cplex on it
            return cplex_lp(cplxinstance);
        }

        bool solveUsingCplex(const vec_lit_t& assump) { 
            std::string cplxinstance = "cplxinstance.lp";
            FILE *fp = fopen(cplxinstance.c_str(), "w");

            clauses_to_lp(fp);
            assumptions_to_lp(assump, fp);
            suffix_to_lp(fp);

            fclose(fp); fp = NULL;

            // Now that the lp instance is in a file, run
            // cplex on it
            return cplex_lp(cplxinstance);
        }

        bool solveUsingCplex(const Lit x) {
            std::string cplxinstance = "cplxinstance.lp";
            FILE *fp = fopen(cplxinstance.c_str(), "w");

            clauses_to_lp(fp);
            literal_to_lp(x, fp);
            suffix_to_lp(fp);

            fclose(fp); fp = NULL;

            // Now that the lp instance is in a file, run
            // cplex on it
            return cplex_lp(cplxinstance);
        }

        /* Gurobi -- prefix "guro" */
        typedef struct guro {
                /* guroenv, model: opaque datastructures */
            GRBEnv *guroenv;
            GRBModel *guromodel;

                    /* # vars -- really the highest numbered var and
                       a vector of GRB vars */
            int guronvars;
            std::vector<GRBVar> gurovarsvect;

                    /* guroclauses: the set of our current clauses, each
                                    clause a set of ints. */
            std::set<std::set<int>> guroclauses;

                    /* guroccmap: map from CNF clauses to gurobi
                                  constraints. Each clause corresponds
                                  to one constraint. The keyset() of
                                  guroccmap (in Java lingo) is exactly
                                  guroclauses. */
            std::map<std::set<int>, GRBConstr> guroccmap;

                    /* guroassumps: map from positive integer to bool */
            std::map<int,bool> guroassumps;
        } GURO;

        GURO *gurodata;

                /* Callback method to get clauses and # vars */
        static void guro_getclauses(void *p, int lit) {
            /* Abuse the guroassumps member to store current set of ints
               that are members of the clause */

            GURO *g = static_cast<GURO *>(p);

            if(lit) {
                (g->guroassumps).insert(std::pair<int,bool>(lit, true));
                int v = abs(lit);
                if(v > (g->guronvars)) g->guronvars = v;
            }
            else {
                /* Take the current guroassumps and
                   and add it as a clause */
                std::set<int> s;

                for(std::map<int,bool>::iterator it = (g->guroassumps).begin(); it != (g->guroassumps).end(); ++it)
                {
                    s.insert(it->first);

                }

                (g->guroclauses).insert(s);
                (g->guroassumps).clear();
            }
        }

        void guro_setassumption(Lit a) {
            int l = translate(a);
            int v = abs(l);

            /* Need to set both LB and UB in case this is a change of assumption to this
               variable */
            if(l < 0) {
                ((gurodata->gurovarsvect).at(v-1)).set(GRB_DoubleAttr_LB, 0.0);
                ((gurodata->gurovarsvect).at(v-1)).set(GRB_DoubleAttr_UB, 0.0);
            }
            else {
                (gurodata->gurovarsvect)[v-1].set(GRB_DoubleAttr_LB, 1.0);
                (gurodata->gurovarsvect)[v-1].set(GRB_DoubleAttr_UB, 1.0);
            }
        }

        lbool guro_modelValue(Lit x) const {
            int v = abs(translate(x));
            if((gurodata->gurovarsvect).at(v-1).get(GRB_DoubleAttr_X) < 1.0) {
                return l_False;
            }
            else {
                return l_True;
            }
        }

        lbool guro_modelValue(Var x) const {
            int v = translate(x);
            if((gurodata->gurovarsvect).at(v-1).get(GRB_DoubleAttr_X) < 1.0) {
                return l_False;
            }
            else {
                return l_True;
            }
        }

        void guro_setconstraints(GURO &newgdat) {
                /* Add new variables if needed */
            int nnewvars = newgdat.guronvars - gurodata->guronvars;
            if(nnewvars > 0) {
                GRBVar *v = gurodata->guromodel->addVars(nnewvars, GRB_BINARY);

                for(int i = 0; i < nnewvars; i++) {
                    gurodata->gurovarsvect.push_back(v[i]);
                }

                gurodata->guronvars += nnewvars;
            }

                /* constraints to be removed */
            for(std::set<std::set<int>>::iterator it = gurodata->guroclauses.begin(); it != gurodata->guroclauses.end(); ++it)
            {
                std::set<int> s = std::set<int>(*it);

                if(newgdat.guroclauses.find(s) == newgdat.guroclauses.end()) {
                    /* Remove */
                    GRBConstr constr = (gurodata->guroccmap)[s];
                    gurodata->guromodel->remove(constr);
                    gurodata->guroclauses.erase(s);
                    gurodata->guroccmap.erase(s);

                }
            }

                /* constraints to be added */
            for(std::set<std::set<int>>::iterator it = newgdat.guroclauses.begin(); it != newgdat.guroclauses.end(); ++it)
            {
                std::set<int> s = std::set<int>(*it);
                if(gurodata->guroclauses.find(s) == gurodata->guroclauses.end())
                {
                        /* Add */
                    GRBLinExpr l = 0;
                    int nneg = 0; /* # negated lits */
                    for(std::set<int>::iterator s_it = s.begin(); s_it != s.end(); s_it++) {
                        int i = (int)(*s_it);
                        int v = abs(i);
                        if(i < 0) {
                            l -= (gurodata->gurovarsvect).at(v-1);
                            nneg++;

                        }
                        else {
                            l += (gurodata->gurovarsvect).at(v-1);
                        }
                    }

                    nneg = (1 - nneg);
                    GRBConstr constr = gurodata->guromodel->addConstr(l >= nneg);

                    gurodata->guroclauses.insert(s);
                    (gurodata->guroccmap)[s] = constr;
                }
            }
        }

        bool guro_basicsolve() {
                /* Set model timelimit */
#if 0
            gurodata->guromodel->set(GRB_DoubleParam_TimeLimit, 100.0);
            else if(status == GRB_TIME_LIMIT) { /* Time limit exceeded */
#endif
            gurodata->guromodel->optimize();
            int status = gurodata->guromodel->get(GRB_IntAttr_Status);

            if(status == GRB_OPTIMAL) {
                return true;
            }
            else {
                return false;
            }
        }

        bool guro_solve() {
            GURO gtmp;
            gtmp.guronvars = 0;
            gtmp.guroassumps.clear();
            lgltravall(solver, (void *)(&gtmp), guro_getclauses);

            guro_setconstraints(gtmp);
            return guro_basicsolve();
        }

        bool guro_solve(const vec_lit_t& assump) {
            GURO gtmp;
            gtmp.guronvars = 0;
            gtmp.guroassumps.clear();
            lgltravall(solver, (void *)(&gtmp), guro_getclauses);

            guro_setconstraints(gtmp);
            for(unsigned int i = 0; i < assump.size(); i++) {
                guro_setassumption(assump[i]);
            }

            return guro_basicsolve();
        }

        bool guro_solve(Lit x) { 
            GURO gtmp;
            gtmp.guronvars = 0;
            gtmp.guroassumps.clear();
            lgltravall(solver, (void *)(&gtmp), guro_getclauses);

            guro_setconstraints(gtmp);
            guro_setassumption(x);
            return guro_basicsolve();
        }

        // Solve without any assumptions.
        bool solve() { 
            return _solve();
        }

        bool solve(const vec_lit_t& assump) { 
            for(unsigned i=0; i != assump.size(); i++) {
                lglassume(solver, translate(assump[i]));
            }
            return _solve();
        }
        bool solve(Lit x) { 
            lglassume(solver, translate(x));
            return _solve();
        }

        void writeCNF(const std::string& filename, const vec_lit_t& assumps);
        void writeCNF(const std::string& filename, Lit assump);
        void writeCNF(const std::string& filename);
        void writeCNF_WithAnnotation(const std::string& filename,
            const char* prefix, int *lits, size_t nlits, int n_per_line);

        int64_t getNumDecisions() const {
            return lglgetdecs(solver);
        }
    };
#endif
}

#endif
