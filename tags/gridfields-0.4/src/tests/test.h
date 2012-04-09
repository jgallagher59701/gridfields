#include <vector>

class Big {
  public:
    int name;
    
};

class Agg {
  public:
    Agg(Big *b);
    std::vector<Big *> bigs;
};


