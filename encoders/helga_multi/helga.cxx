#include <iostream>
#include <fstream>
#include <string>
#include <stdlib.h>
#include <vector>
#include <mutex>
#include <cstdio>
#include <set>
#include <iterator>
#include <sstream>
#include <stdexcept>
#include <thread>

#include "../shared/debug.cxx"
#include "../shared/SatInterface.cxx"
#include "../shared/create_adjency_matrix.cxx"

using namespace std;

int vertices_count = 0;

// encode our variables into coherent natural numbers
string encode(int i, int j, int vertices_count, string type){
        int result = (i - 1) * vertices_count + j;

        if (type == "o") {
                result += vertices_count * vertices_count;
        }
        return to_string(result);
}
int xy_to_satsolve(int position, int vertex, int maxvertex, string type){
        int result = (position -1 ) * maxvertex + vertex;

        if (type == "o") {
                result += maxvertex * maxvertex;
        }
        return result;
}

struct variable {
        string type;
        int x;
        int y;
};
// decode
variable satsolve_to_xy(int satsolve, int maxvertex){
        variable coords;
        if (satsolve >= maxvertex * maxvertex) {
                satsolve -= maxvertex * maxvertex;
                coords.type = "o";
        } else {
                coords.type = "s";
        }
        coords.x = (1 + ((satsolve - 1)/maxvertex)); // from vertex i
        coords.y = (((satsolve - 1) % maxvertex) + 1); // to vertex j

        return coords;
}

variable searchvec(int flast, std::vector<variable> v){
        variable found;
        for(unsigned int i = 0; i < v.size(); i++) {
                if(v.at(i).x == flast) {
                        found = v.at(i);
                        break;
                }
        }
        return found;
}

string generate_result(std::stringstream* input, int vertex_count){
        /*std::stringstream input;
           input << test << endl;*/
        string result = "v ";
        string tmp = "";
        string line = "";
        int length;
        string path = "\nPfad: ";
        std::vector<variable> successorconstraints;
        std::vector<variable> orderingconstraints;

        while (getline(*input,line)) {
                length = line.length();
                if(line[0] == 'v') {
                        int z = 1;
                        while ((z + 1) < length) {
                                if ((line[z] == ' ') && (line[z + 1] != '-')) {
                                        while ((line[z + 1] != ' ') && ((z + 1) < length)) {
                                                tmp = tmp + line[z + 1];
                                                z++;
                                        }
                                        variable tempvec = satsolve_to_xy(stoi(tmp), vertex_count);
                                        tmp = "";
                                        if (tempvec.type == "s") {
                                                successorconstraints.push_back(tempvec);
                                        } else if (tempvec.type == "o") {
                                                orderingconstraints.push_back(tempvec);
                                        }
                                }
                                z++;
                        }
                }
        }

        std::vector<variable> printing;
        int index = 1;
        for(unsigned int i = 0; i < vertex_count - 1; i++) {
                variable returned = searchvec(index, successorconstraints);
                printing.push_back(returned);
                index = returned.y;
        }
        for(unsigned int i = 0; i < printing.size(); i++) {
                variable returned = printing.at(i);
                result = result + to_string(returned.x) + " ";
        }
        cout << path + "\n" << endl;
        return result;
}
// constraint (1)
// each vertex except the last has at least one successor
void condition1(stringstream *output, vector<vector<int> > & edge_matrix, int *clauses_count, int vertices_count){
        for (int i = 0; i < vertices_count - 1; i++) {
                string cnf = "";
                for (int j = 0; j < vertices_count; j++) {
                        if (edge_matrix[i][j] == 1 && i != j) {
                                cnf += encode(i + 1, j + 1, vertices_count, "s") + " ";
                        }
                }
                cnf += "0";
                *output << cnf << endl;
                (*clauses_count)++;
        }
        print("constraint (1) finished");
}
// constraint (2)
// each vertex has at most one successor
void condition2(stringstream *output, vector<vector<int> > & edge_matrix, int vertices_count, int *clauses_count){
        for (int i = 0; i < vertices_count; i++) {
                for (int j = 0; j < vertices_count; j++) {
                        if(i != j) {
                                for (int k = j; k < vertices_count; k++) {
                                        if(i != k && j != k) {
                                                string cnf = "-" + encode(i + 1, j + 1, vertices_count, "s") + " " + "-" + encode(i + 1, k + 1, vertices_count, "s") + " 0";
                                                *output << cnf << endl;
                                                (*clauses_count)++;
                                        }
                                }
                        }

                }
        }
        print("constraint (2) finished");
}

// constraint (3)
// each vertex except the first is the successor of at least one vertex
void condition3(stringstream *output,  vector<vector<int> > & edge_matrix, int *clauses_count, int vertices_count){
        for (int i = 1; i < vertices_count; i++) {
                string cnf = "";
                for (int j = 0; j < vertices_count; j++) {
                        if (edge_matrix[j][i] == 1 && i != j) {
                                cnf += to_string(xy_to_satsolve(j+1, i+1, vertices_count, "s")) + " ";
                        }
                }
                cnf += "0";
                *output << cnf << endl;
                (*clauses_count)++;
        }
        print("constraint (3) finished");
}

// constraint (4)
// no vertex is successor to more than one node
void condition4(stringstream *output,  vector<vector<int> > & edge_matrix, int *clauses_count, int vertices_count){
        for (int i = 0; i < vertices_count; i++) {
                for (int j = 0; j < vertices_count; j++) {
                        if (i != j && edge_matrix[i][j] == 1) {
                                for (int k = j; k < vertices_count; k++) {
                                        if (i != k && j != k && edge_matrix[i][k] == 1) {
                                                string cnf = "-" + to_string(xy_to_satsolve(j+1, i+1, vertices_count, "s")) + " " + "-" + to_string(xy_to_satsolve(k+1, i+1, vertices_count, "s"));
                                                cnf += " 0";
                                                *output << cnf << endl;
                                                (*clauses_count)++;
                                        }
                                }
                        }
                }
        }
        print("constraint (4) finished");
}

// constraint (5)
// transitivity replaced
void condition5(stringstream *output, int *clauses_count, int vertices_count){
        for (int i = 1; i <= vertices_count; i++) {
                for (int j = i + 1; j <= vertices_count; j++) {
                        for (int k = j + 1; k <= vertices_count; k++) {
                                string cnf = "-" + to_string(xy_to_satsolve(i, j, vertices_count, "o")) + " " + "-" + to_string(xy_to_satsolve(j, k, vertices_count, "o")) + " " + to_string(xy_to_satsolve(i, k, vertices_count, "o"));
                                cnf += " 0";
                                *output << cnf << endl;
                                string newcnf =  to_string(xy_to_satsolve(i, j, vertices_count, "o")) + " " + to_string(xy_to_satsolve(j, k, vertices_count, "o")) + " " + "-" + to_string(xy_to_satsolve(i, k, vertices_count, "o"));
                                newcnf += " 0";
                                *output << newcnf << endl;
                                *clauses_count += 2;
                        }
                }
        }
        print("constraint (5) finished");
}
// constraint (7)
// irreflexivity
// TODO TODO TODO can be removed if all functions correctly ignore s(i,i)
void condition7(stringstream *output, int *clauses_count, int vertices_count){
        for (int i = 1; i <= vertices_count; i++) {
                string cnf = "-" + to_string(xy_to_satsolve(i, i, vertices_count, "o"));
                cnf += " 0";
                *output << cnf << endl;
                (*clauses_count)++;
        }
        print("constraint (7) finished");
}
// constraint (8)
// the ordering relation must apply to all pairs of vertices
void condition8(stringstream *output, int *clauses_count, int vertices_count){
        for (int i = 1; i <= vertices_count; i++) {
                for (int j = i + 1; j <= vertices_count; j++) {

                        string cnf = to_string(xy_to_satsolve(i, j, vertices_count, "o")) + " " + to_string(xy_to_satsolve(j, i, vertices_count, "o"));
                        cnf += " 0";
                        *output << cnf << endl;
                        (*clauses_count)++;
                }
        }
        print("constraint (8) finished");
}

// constraint (9)
// the first vertex precedes all others
// ATTENTION: s1,1 not allowed
void condition9(stringstream *output, int *clauses_count, int vertices_count){
        for (int i = 2; i <= vertices_count; i++) {
                string cnf = to_string(xy_to_satsolve(1, i, vertices_count, "o"));
                cnf += " 0";
                *output << cnf << endl;
                (*clauses_count)++;
        }
        print("constraint (9) finished");
}
// condition x
// the last vertex succeeds all others
// ATTENTION: s5,5 not allowed
void condition10(stringstream *output, int *clauses_count, int vertices_count){

        for (int i = 1; i < vertices_count; i++) {
                string cnf = to_string(xy_to_satsolve(i, vertices_count, vertices_count, "o"));
                cnf += " 0";
                *output << cnf << endl;
                (*clauses_count)++;
        }
        print("constraint (10) finished");
}
// constraint (11)
// Finally, the relationship between the successor and ordering relations is simply
void condition11(stringstream *output, int *clauses_count, int vertices_count){
        for (int i = 1; i <= vertices_count; i++) {
                for (int j = 1; j <= vertices_count; j++) {

                        if (i > j) {
                                string cnf = "-" + to_string(xy_to_satsolve(i, j, vertices_count, "s")) + " " + "-" +  to_string(xy_to_satsolve(j, i, vertices_count, "o"));
                                cnf += " 0";
                                *output << cnf << endl;
                        } else {
                                string cnf = "-" + to_string(xy_to_satsolve(i, j, vertices_count, "s")) + " " + to_string(xy_to_satsolve(i, j, vertices_count, "o"));
                                cnf += " 0";
                                *output << cnf << endl;
                        }
                        (*clauses_count)++;
                }
        }
        print("constraint (11) finished");
}


int main( int argc, char* argv[] )
{
        if ( argc != 4 ) {
                // argc should be 3 for correct execution
                print("Error, you need to provide a path to the graph and a command how to use the solver");
                print("Usage: ./helga.out <graph> <tmpfile> <solver_command>");
                print("Terminating...");
                exit(1);
        }
        string graph_path = argv[1]; // path to graph
	string tmpfile = argv[2];
        const char * solver_command = argv[3]; // command to solver
        // open graph under given path
        ifstream graph_raw;
        graph_raw.open(graph_path);
        // create adjency matrix
        vector<vector<int> > edge_matrix;
        create_adjency_matrix(&edge_matrix, &vertices_count, &graph_raw);

        //uncomment if you wanna see the matrix
        //print_matrix(edge_matrix, vertices_count);

        // output for SAT-solvers (contains generated clauses)
        int clauses_count = 0;
        fstream output;
        output.open(tmpfile, fstream::in | fstream::out | fstream::trunc);
        // add first line as a placeholder for the header (p cnf V E)
        output << "p cnf                                                       " << endl; // we love c++ and OS's in general this Is fucking AWESOME

        std::stringstream out1("");
        std::stringstream out2("");
        std::stringstream out3("");
        std::stringstream out4("");
        std::stringstream out5("");
        std::stringstream out6("");
        std::stringstream out7("");
        std::stringstream out8("");
        std::stringstream out9("");
        std::stringstream out10("");
        std::stringstream out11("");

        int c1 = 0;
        int c2 = 0;
        int c3 = 0;
        int c4 = 0;
        int c5 = 0;
        int c6 = 0;
        int c7 = 0;
        int c8 = 0;
        int c9 = 0;
        int c10 = 0;
        int c11 = 0;

        //const vector<vector<int> > tmp = edge_matrix;

        thread t1(condition1, &out1, ref(edge_matrix), &c1, vertices_count);
        thread t2(condition2, &out2, ref(edge_matrix), vertices_count, &c2);
        thread t3(condition3, &out3, ref(edge_matrix), &c3, vertices_count);
        thread t4(condition4, &out4, ref(edge_matrix), &c4, vertices_count);
        thread t5(condition5, &out5, &c5, vertices_count);
        //thread t6(condition6, &out6, &c6, vertices_count);
        thread t7(condition7, &out7, &c7, vertices_count);
        thread t8(condition8, &out8, &c8, vertices_count);
        thread t9(condition9, &out9, &c9, vertices_count);
        thread t10(condition10, &out10, &c10, vertices_count);
        thread t11(condition11, &out11, &c11, vertices_count);

        t1.join();
        t2.join();
        t3.join();
        t4.join();
        t5.join();
        //t6.join();
        t7.join();
        t8.join();
        t9.join();
        t10.join();
        t11.join();

        clauses_count = c1 + c2 + c3 + c4 + c5 + c7 + c8 + c9 + c10 + c11;

        print("writing clauses to file...");
        output << out1.str() << out2.str() << out3.str() << out4.str() << out5.str() << out7.str() << out8.str() << out9.str() << out10.str() << out11.str() << endl;
        print("success");

        // write header
        output.seekg(0, std::ios::beg);
        output << "p cnf " << std::to_string(2 * vertices_count * vertices_count) << " " << std::to_string(clauses_count) << endl;
        output.seekg(0, std::ios::beg);
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
                        solve = line;
                        break;
                }
        }

        if (solve == "s UNSATISFIABLE") {
                cout << "s UNSATISFIABLE" << endl;
                exit(20);
        } else if (solve == "s SATISFIABLE") {
                string result = "";
                result = generate_result(&solved, vertices_count);
                cout << "s SATISFIABLE" << endl;
                cout << result << endl;
                exit (10);
        }
        cout << "ERROR" << endl;
        exit(1);
}
