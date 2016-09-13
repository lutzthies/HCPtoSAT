#include <iostream>
#include <fstream>
#include <string>
#include <stdlib.h>
#include <vector>
#include <mutex>
#include <cstdio>
#include <iterator>
#include <sstream>
#include <stdexcept>
#include <thread>
// PBLib
 #include "../../pblib/pb2cnf.h" // encoding interface
 #include "../../pblib/VectorClauseDatabase.h" // clause database

#include "./VarManager.h"
#include "../shared/debug.cxx"
#include "../shared/process_result.cxx"
#include "../shared/SatInterface.cxx"
#include "../shared/create_adjency_matrix.cxx"

using namespace std;
using namespace PBLib;

PBConfig config = make_shared< PBConfigClass >();
AuxVarManager auxvars(1);
VarManager vars(&auxvars); // custom encoding interface
int vertices_count;

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
                                literals.push_back(WeightedLit(vars.encode('s', j,i), 1));
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
        print("constraint (3) FINISHED (from " + to_string(from)+ " to " + to_string(to) +")");
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
// relation between successor and ordering variable
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

        VectorClauseDatabase formula(config);
        PB2CNF pb2cnf(config);
        constraint_transitivity(&formula, 1, vertices_count - 2);
        constraint_has_successor(&formula, ref(edge_matrix), ref(pb2cnf));
        constraint_is_successor(&formula, ref(edge_matrix),ref(pb2cnf));
        constraint_first_precedes(&formula);
        constraint_last_succeeds(&formula);
        constraint_relation(&formula);

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
        string line;
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

                result = generate_result(&vars,&solved, vertices_count);
                cout << "s SATISFIABLE" << endl;
                cout << result << endl;
                exit (10);
        }
        cout << "ERROR" << endl;
        exit(1);
}
