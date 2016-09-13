#include <iostream>
#include <string>
#include <vector>

void create_adjency_matrix(std::vector<std::vector<int> > *edge_matrix, int *vertices_count, std::ifstream *graph_raw){
        print("parsing graph from file into adjacency matrix");
        std::string line;
        while (std::getline(*graph_raw, line))
        {
                std::stringstream stream(line);
                std::istream_iterator<std::string> begin(stream),
                end;
                std::vector<std::string> splitted_line(begin, end);
                std::string type = splitted_line[0];
                if (type == "p") {
                        *vertices_count = stoi(splitted_line[2]);
                        // increasing for dummy vertex
                        (*vertices_count)++;
                        // initialize vector with zeros
                        std::vector<int> edge_vector;
                        edge_vector.resize(*vertices_count, 0);
                        // fill matrix
                        for (int i = 0; i < *vertices_count; i++) {
                                (*edge_matrix).push_back(edge_vector);
                        }
                        // i == j edge not not apparend
                        for (int i = 0; i < *vertices_count; i++) {
                                (*edge_matrix)[i][i] = -1;
                        }
                } else if (type == "e") {
                        // add entry for edge
                        (*edge_matrix)[stoi(splitted_line[1]) - 1][stoi(splitted_line[2]) - 1] = 1;
                        // manipulate edges of first node
                        if (splitted_line[2] == "1") {
                                (*edge_matrix)[stoi(splitted_line[1]) - 1][*vertices_count - 1] = 1;
                        }
                } else {
                        print("this line was either a comment or pretty dumb shit");
                }
        }
}
