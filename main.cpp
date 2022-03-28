#include <iostream>

#include "RAM.h"

using namespace std;
using namespace RAM;

int main() {
  Machine m;
  m.set_path("../ram-count-0-1.txt");
  m.set_input({6, 1, 0, 1, 0, 0, 1});
  m.be_verbose(false);
  m.set_ostream(cout);

  auto output = m.run();
  cout << "Output: " << output;

  return 0;
}
