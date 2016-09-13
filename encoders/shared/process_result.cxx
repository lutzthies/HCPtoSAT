#include "./VarManager.h"

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

std::string generate_result(VarManager* vars, std::stringstream* input, int vertex_count){
        std::string result = "v ",
                    tmp = "",
                    line = "";
        int length;
        std::string path = "\nPfad: ";
        std::vector<variable> successorconstraints;
        std::vector<variable> orderingconstraints;

        while (getline(*input,line)) {
                if(line[0] == 'v') {
                        int z = 1;
                        length = line.length();
                        while ((z + 1) < length) {
                                if ((line[z] == ' ') && (line[z + 1] != '-')) {
                                        while ((line[z + 1] != ' ') && ((z + 1) < length)) {
                                                tmp = tmp + line[z + 1];
                                                z++;
                                        }
                                        variable var = vars->decode(stoi(tmp));

                                        if (var.type == 's') {
                                                //cout << (to_string(var.type) + "("  + to_string(var.x) + "," + to_string(var.y) + ") ") << endl;
                                                successorconstraints.push_back(var);
                                        }
                                        tmp = "";
                                }
                                z++;
                        }
                }
        }
        int index = 1;
        for(int i = 0; i < vertex_count - 1; i++) {
                variable var = searchvec(index, successorconstraints);
                result = result + std::to_string(var.x) + " ";
                index = var.y;
        }
        return result;
}
