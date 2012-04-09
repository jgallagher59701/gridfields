#include "assignments.h"

int Assign::pnpoly(int npol, float *xp, float *yp, float x, float y) {
  int i, j, c = 0;
  for (i = 0, j = npol-1; i < npol; j = i++) {
    if ((((yp[i] <= y) && (y < yp[j])) ||
       ((yp[j] <= y) && (y < yp[i]))) &&
       (x < (xp[j] - xp[i]) * (y - yp[i]) / (yp[j] - yp[i]) + xp[i]))
       c = !c;
  }
  return c;
};

bool Assign::TestCallback(long unsigned id, void *arg) {
  set<CellId> *out = (set<CellId> *) arg;
  out->insert(id);
  //std::cout << "hit: " << id << std::endl;
  return true;
}

bool Assign::equal(Type t, UnTypedPtr p, UnTypedPtr q) {
      switch (t) {
        case FLOAT:
          return *(float *)p == *(float *)q;
          break;
        case INT:
          return *(int *)p == *(int *)q;
          break;
        case OBJ:
          return p == q;
      }
}

