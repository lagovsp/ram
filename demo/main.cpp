#include <iostream>
#include <ram.h>

using namespace std;
using namespace RAM;

static const vector<Name> demo_tasks = {
	"3**n+2**n",
	"count-0-1",
	"n!+n**2",
	"n**n",
};

static const vector<Name> test_tasks = {
	"test-code",
};

int main() {
  Machine m;
  m.be_verbose(true);

  for (const auto &task: demo_tasks) {
	ofstream ls("../tasks/" + task + "/log.txt");
	ifstream ss("../tasks/" + task + "/source.txt");
	ifstream is("../tasks/" + task + "/input.txt");

	m.set_log_stream(ls);
	m.set_code(ss);
	m.set_input(is);
	m.set_name(task);

	m.run();
  }

  return 0;
}