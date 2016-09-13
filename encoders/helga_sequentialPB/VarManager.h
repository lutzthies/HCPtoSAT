#ifndef VARMANAGER_H
#define VARMANAGER_H

#include <stdlib.h>
#include <unordered_map>
#include "../../pblib/pb2cnf.h" // encoding interface


struct variable {
        char type;
        int x;
        int y;
        bool operator==(const variable &other) const
 { return (type == other.type
           && x == other.x
           && y == other.y);
 }
};

namespace std {

  template <>
  struct hash<variable>
  {
    std::size_t operator()(const variable& k) const
    {
      using std::size_t;
      using std::hash;

      // Compute individual hash values for first,
      // second and third and combine them using XOR
      // and bit shifting:

      return ((hash<char>()(k.type)
               ^ (hash<int>()(k.x) << 1)) >> 1)
               ^ (hash<int>()(k.y) << 1);
    }
  };
}

class VarManager {
private:
  AuxVarManager* auxvarx;
  std::unordered_map<variable, int32_t> var_to_lit;
  std::unordered_map<int, variable> lit_to_var;

public:
 VarManager(AuxVarManager *auxvarstest);
 int size(void);
 variable decode(int32_t val);
 int32_t encode(char type, int i, int j);
};
#endif
