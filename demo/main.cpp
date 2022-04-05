#include <iostream>
#include <ram.h>

using namespace std;
using namespace RAM;

int main() {
  Machine m;

  string task = "3**n+2**n";

  ofstream ls("../tasks/" + task + "/log.txt");
  ifstream ss("../tasks/" + task + "/source.txt");
  ifstream is("../tasks/" + task + "/input.txt");

  m.set_log_stream(ls);
  m.be_verbose(true);
  m.set_code(ss);
  m.set_input(is);

  auto output = m.run();

  return 0;
}