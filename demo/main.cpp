#include <iostream>
#include <ram.h>

using namespace std;
using namespace RAM;

static const vector<string> tasks = {
	"3**n+2**n",
	"count-0-1",
	"n!+n**2",
	"n**n",
};

int main() {
  Machine m;
  m.be_verbose(true);

  for (const auto &t: tasks) {
	ofstream ls("../tasks/" + t + "/log.txt");
	ifstream ss("../tasks/" + t + "/source.txt");
	ifstream is("../tasks/" + t + "/input.txt");

	m.set_log_stream(ls);
	m.set_code(ss);
	m.set_input(is);

	auto output = m.run();
  }

  return 0;
}