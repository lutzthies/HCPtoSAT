#include "./VarManager.h"

std::mutex write1;

VarManager::VarManager(AuxVarManager *auxvarstest, int *vertices_counttest) {
        auxvarx = auxvarstest;
        vertices_count = vertices_counttest;
}
int VarManager::size(void){
        return var_to_litO.size();
}
// encode our variables into coherent natural numbers
int32_t VarManager::encode(char type, int i, int j){
        variable key = {type, i, j};
        if (type == 'o') {
                return var_to_litO.at(key);
        } else {
                if (var_to_litS.count(key) == 1 ) {
                        write1.lock();
                        int32_t temp = var_to_litS.at(key);
                        write1.unlock();
                        return temp;
                } else {
                        int value = (*auxvarx).getVariable();
                        std::pair<variable,int32_t> newkv (key, value);
                        std::pair<int32_t,variable> newvk (value, key);
                        var_to_litS.insert(newkv);
                        lit_to_varS.insert(newvk);
                        write1.unlock();
                        return value;
                }
        }
}

bool VarManager::prepare(void){
        variable key;
        int value;
        for (int i = 1; i <= (*vertices_count); i++) {
                for (int j = 1; j <= (*vertices_count); j++) {
                        if(i < j) {
                                key = {'o', i, j};
                                value = var_to_litO.size() + 1;
                                std::pair<variable,int32_t> newkv (key, value);
                                std::pair<int32_t,variable> newvk (value, key);
                                var_to_litO.insert(newkv);
                                lit_to_varO.insert(newvk);
                        }
                }
        }
        (*auxvarx).resetAuxVarsTo(var_to_litO.size() + 1);
        return true;
}
// decode by reverse lookup (check for existence of key is necessary as pblib may introduce auxilary variables)
variable VarManager::decode(int32_t val){
        if (val <= var_to_litO.size()) {
                if(lit_to_varO.count(val) == 1) {
                        return lit_to_varO.at(val);
                }
        } else {
                write1.lock();
                if(lit_to_varS.count(val) == 1) {
                        variable temp = lit_to_varS.at(val);
                        write1.unlock();
                        return temp;
                } else {
                        write1.unlock();
                }
        }
}
