#include <iostream>
#include <fstream>
#include <string>
#include <stdlib.h>
#include <vector>
#include <cstdio>
#include <iterator>
#include <sstream>
#include <stdexcept>
#include <thread>
// PBLib
 #include "../../pblib/pb2cnf.h" // encoding interface
 #include "../../pblib/VectorClauseDatabase.h" // clause database

#include "./VarManager.h"
#include "../shared/SatInterface.cxx"
#include "../shared/debug.cxx"
#include "../shared/process_result.cxx"
#include "../shared/create_adjency_matrix.cxx"

using namespace std;
using namespace PBLib;

PBConfig config = make_shared< PBConfigClass >();
AuxVarManager auxvars(1);
int vertices_count = 0;
VarManager vars(&auxvars, &vertices_count); // custom encoding interface

// constraint (1)
// each vertex has at exactly one successor
// NOTE: except the last
void constraint_has_successor(VectorClauseDatabase *formula, vector<vector<int> > & edge_matrix, PB2CNF & pb2cnf){
        vector< WeightedLit > literals;
        for (int i = 0; i < vertices_count - 1; i++) {
                literals.clear();
                for (int j = 0; j < vertices_count; j++) {
                        if (edge_matrix[i][j] == 1 && i != j) {
                                literals.push_back(WeightedLit(vars.encode('s', i + 1, j + 1), 1));
                        }
                }
                PBConstraint constraint(literals, BOTH, 1, 1);
                pb2cnf.encode(constraint, *formula, auxvars);
        }
        print("constraint (1) finished");
}
// constraint (2)
// each vertex is the successor to exactly one vertex
// NOTE: except the first
void constraint_is_successor(VectorClauseDatabase *formula, vector<vector<int> > & edge_matrix, PB2CNF & pb2cnf){
        vector< WeightedLit > literals;
        for (int i = 2; i < vertices_count; i++) {
                literals.clear();
                for (int j = 1; j < vertices_count; j++) {
                        if (edge_matrix[j - 1][i - 1] == 1 && i != j) {
                                literals.push_back(WeightedLit(vars.encode('s', j, i), 1));
                        }
                }
                PBConstraint constraint(literals, BOTH, 1, 1);
                pb2cnf.encode(constraint, *formula, auxvars);
        }
        print("constraint (2) finished");
}
// constraint (3)
// transitivity
void constraint_transitivity(VectorClauseDatabase *formula, int from, int to){
        for (int i = from; i <= to; i++) {
                for (int j = i + 1; j <= vertices_count - 1; j++) {
                        for (int k = j + 1; k <= vertices_count; k++) {
                                (*formula).addClause(-vars.encode('o', i, j),-(vars.encode('o', j, k)), vars.encode('o', i, k));
                                (*formula).addClause(vars.encode('o', i, j),vars.encode('o', j, k),-(vars.encode('o', i, k)));
                        }
                }
        }
        print("constraint (3) finished (from " + to_string(from)+ " to " + to_string(to) +")");
}
// constraint (4)
// the first vertex precedes all others
// NOTE: s1,1 not allowed
void constraint_first_precedes(VectorClauseDatabase *formula){
        for (int i = 2; i <= vertices_count; i++) {
                (*formula).addClause(vars.encode('o', 1, i));
        }
        print("constraint (4) finished");
}
// constraint (5)
// the last vertex succeeds all others
// NOTE: s5,5 not allowed
void constraint_last_succeeds(VectorClauseDatabase *formula){
        for (int i = 1; i < vertices_count; i++) {
                (*formula).addClause(vars.encode('o', i, vertices_count));
        }
        print("constraint (5) finished");
}
// constraint (6)
// relation between ordering and successor variables
void constraint_relation(VectorClauseDatabase *formula){
        for (int i = 1; i <= vertices_count; i++) {
                for (int j = 1; j <= vertices_count; j++) {
                        if( i != j ) {
                                if (i > j) {
                                        (*formula).addClause(-(vars.encode('s', i, j)), -(vars.encode('o', j, i)));

                                } else {
                                        (*formula).addClause(-(vars.encode('s', i, j)), vars.encode('o', i, j));
                                }
                        }
                }
        }
        print("constraint (6) finished");
}

int main( int argc, char* argv[] )
{
        if ( argc != 3 ) {
                // argc should be 3 for correct execution
                print("Error, you need to provide a path to the graph and a command how to use the solver");
                print("Usage: ./helga.out <graph> <solver_command>");
                print("Terminating...");
                exit(1);
        }
        string graph_path = argv[1]; // path to graph
        const char * solver_command = argv[2]; // command to solver
        // open graph under given path
        ifstream graph_raw;
        graph_raw.open(graph_path);
        // create adjency matrix
        vector<vector<int> > edge_matrix;
        create_adjency_matrix(&edge_matrix, &vertices_count, &graph_raw);

        print("INFO - preparing thread environment");
        vars.prepare();
        VectorClauseDatabase formula1(config),
        formula2(config),
        formula3_1(config),
        formula3_2(config),
        formula3_3(config),
        formula3_4(config),
        formula4(config),
        formula5(config),
        formula6(config),
        formula7(config);

        PB2CNF pb2cnf1(config),
        pb2cnf2(config);

        // calculate loop boundaries, for equal distribution of work to the threads
        int percent = ((vertices_count * 100) / 100),
            bound1 = (10 * percent)/100,
            bound2 = (20 * percent)/100,
            bound3 = (40 * percent)/100;
        print("INFO - spawning threads");
        thread t1 (constraint_transitivity, &formula3_1, 1, bound1),
        t2 (constraint_transitivity, &formula3_2, bound1 + 1, bound2),
        t3 (constraint_transitivity, &formula3_3, bound2 + 1, bound3),
        t4 (constraint_transitivity, &formula3_4, bound3 + 1, vertices_count - 2);

        constraint_has_successor( &formula1, ref(edge_matrix), ref(pb2cnf1));
        constraint_is_successor( &formula2, ref(edge_matrix),ref(pb2cnf2));
        constraint_first_precedes(&formula4);
        constraint_last_succeeds(&formula5);
        constraint_relation(&formula7);

        // merge clauses from threads into a single database
        VectorClauseDatabase formula(config);
        formula.addClauses(formula1.getClauses());
        formula.addClauses(formula2.getClauses());
        formula.addClauses(formula4.getClauses());
        formula.addClauses(formula5.getClauses());
        formula.addClauses(formula7.getClauses());

        t1.join(); t2.join(); t3.join(); t4.join();
        print("SUCCESS - all threads finished, preparing result");

        formula.addClauses(formula3_1.getClauses());
        formula.addClauses(formula3_2.getClauses());
        formula.addClauses(formula3_3.getClauses());
        formula.addClauses(formula3_4.getClauses());

        // output for SAT-solvers (contains generated clauses)
        print("INFO - writing results to file");
        fstream output;
        output.open("in.txt", fstream::in | fstream::out | fstream::trunc);
        // write header
        int variable_count = auxvars.getBiggestReturnedAuxVar();
        int clauses_count = formula.getClauses().size();
        output << "p cnf " << std::to_string(variable_count) << " " << std::to_string(clauses_count) << endl;
        // write clauses
        for(auto clause : formula.getClauses()) {
                for(auto lit : clause) {
                        output << to_string(lit) << " ";
                }
                output << "0" << endl;
        }
        output.close();

        // start solver
        print("INFO - starting solver with provided command");
        std::stringstream solved;
        solved << execToString(solver_command) << endl;
        print("SUCCESS - solver finished and returned result");

        string solve = "no result";
        solved.seekg(0, std::ios::beg);
        std::string line;
        while (getline(solved,line)) {
                if(line[0] == 's') {
                        cout << line << endl;
                        solve = line;
                        break;
                }
        }
        if (solve == "s UNSATISFIABLE") {
                cout << "s UNSATISFIABLE" << endl;
                exit(20);
        } else if (solve == "s SATISFIABLE") {
                string result = "";
                solved.seekg(0, std::ios::beg);
                result = generate_result(&vars, &solved, vertices_count);
                cout << "s SATISFIABLE" << endl;
                cout << result << endl;
                exit(10);
        }
        cout << "ERROR" << endl;
        exit(1);
}
