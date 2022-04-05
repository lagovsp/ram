#include <iostream>
#include <ram.h>

using namespace std;
using namespace RAM;

int main() {
  Machine m;

  ofstream ls("../logs/log-count-0-1.txt");
  m.set_log_stream(ls);
  m.be_verbose(true);

  ifstream cs("../codes/src-count-0-1.txt");
  m.set_code(cs);

  ifstream is("../input/in-count-0-1.txt");
  m.set_input({0});
  m.set_input(is);

  auto output = m.run();

  return 0;
}