#include "./VarManager.h"

VarManager::VarManager(AuxVarManager *auxvarstest) {
        auxvarx = auxvarstest;
}
int VarManager::size(void){
        return var_to_lit.size();
}
// encode our variables into coherent natural numbers
int32_t VarManager::encode(char type, int i, int j){
        variable key = {type, i, j};
        if (var_to_lit.count(key) == 1) {
                int32_t temp = var_to_lit.at(key);
                return temp;
        } else {
                int value = (*auxvarx).getVariable();
                std::pair<variable,int32_t> newkv (key, value);
                std::pair<int32_t,variable> newvk (value, key);
                var_to_lit.insert(newkv);
                lit_to_var.insert(newvk);
                return value;
        }
}

// decode by reverse lookup (check for existence of key is necessary as pblib may introduce auxilary variables)
variable VarManager::decode(int32_t val){
        if(lit_to_var.count(val) == 1) {
                variable temp = lit_to_var.at(val);
                return temp;
        }
        exit(1);
}
