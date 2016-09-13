/*
   printing functions for debugging purposes
 */
// prints basic information
bool debug = true;
void print(std::string statement){
        if (!debug) return;
        static std::mutex m;
        std::lock_guard<std::mutex> _(m);
        std::cout << statement << std::endl;
}
// prints clauses in human readable form while generating
bool readable = false;
void print_clause(std::string clause){
        if (!readable) return;
        static std::mutex m;
        std::lock_guard<std::mutex> _(m);
        std::cout << clause << std::endl;
}
// prints adjacency matrix
void print_matrix(std::vector<std::vector<int> > vector, int vertices_count){
        printf("Printing edge_matrix:\n");
        for (int i = 0; i < vertices_count; i++) {

                for (int j = 0; j < vertices_count; j++) {
                        printf(" %d ", vector[i][j]);
                }
                printf("\n");
        }
}
