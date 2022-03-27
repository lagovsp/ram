#include "RAM.h"

using namespace std;

namespace RAM {

#define GET_ARG(m, c) \
    RAM::Machine::GROUPS.at((c)->ctype_).at((c)->atype_)((m), (c)->arg_)

#define GET_VALUE(m, c) get<int>(GET_ARG(m, c))
#define GET_LABEL(m, c) get<string>(GET_ARG(m, c))

#define INFO(var) #var << "=" << var

static const string SEPARATOR = "----------------------------------------";

ostream &operator<<(ostream &os, const Arg &a) {
  if (a.index() == 0) { os << get<int>(a); }
  else { os << get<string>(a); }
  return os;
}

ostream &operator<<(ostream &os, const Com &c) {
  os << INFO(c.ctype_) << " " << INFO(c.atype_) << " " << INFO(c.arg_) << " "
	 << INFO(c.label_.value_or("__nv"));
  return os;
}

static const char SP = ' ';
static const char LP = '(';
static const char RP = ')';
static const char EQ = '=';
static const char PTR = '*';
static const char COL = ':';

static const string NUMS = "0123456789";

static const string LABEL_USED_ERROR = "label has already been used";
static const string BAD_COMMAND_ERROR = "bad command";
static const string NEGATIVE_ADDRESS_ERROR = "seen negative address";

static const string HANDLER_NOT_FOUND = "handler not found";

static const set<ArgT> LAB_COMS = {
	JUMP,
	JGTZ,
	JZERO,
	HALT,
};

static const map<char, ArgT> CHARS_TYPES = { // the hell
	{EQ, VALUE},
	{PTR, ADDRESS_AT_ADDRESS},
};

const std::list<Com> &Machine::commands() const {
  return commands_;
}

Arg Machine::value_to_argument(const Machine &m, const Arg &a) {
  return get<int>(a);
}

Arg Machine::address_to_argument(const Machine &m, const Arg &a) {
  return m.memory_.at(get<int>(a));
}

Arg Machine::address_to_address_to_argument(const Machine &m, const Arg &a) {
  return m.memory_.at(m.memory_.at(get<int>(a)));
}

Arg Machine::label_to_argument(const Machine &m, const Arg &a) {
  return get<string>(a);
}

Com Machine::parse_command(string line) {
  auto label = parse_label(line);
  auto ct = parse_command_t(line);
  auto arg = (LAB_COMS.contains(ct)) ?
			 parse_argument_labelic(line) :
			 parse_argument_numeric(line);
  cout << "parsing '" << line << "'";
  auto at = (LAB_COMS.contains(ct)) ? LABEL : parse_argument_t(line);

  return {
	  move(ct),
//	  move(parseArgT(line)),
	  move(at),
	  move(arg),
	  move(label),
  };
}

ComT Machine::parse_command_t(string line) {
  auto it = COM_HAND.find(line.substr(0, line.find(LP)));
  if (it == COM_HAND.cend()) {
	throw runtime_error(BAD_COMMAND_ERROR);
  }
  return it->first;
}

ArgT Machine::parse_argument_t(string line) {
  auto it = CHARS_TYPES.find(line.at(line.find(LP) + 1));
  return (it != CHARS_TYPES.cend()) ? it->second : ADDRESS;
}

Arg Machine::parse_argument_numeric(string line) {
  size_t lp = line.find(LP);
  string arg = line.substr(lp + 1, line.find(RP) - lp - 1);
  if (CHARS_TYPES.contains(arg.front())) {
	arg.erase(0, 1);
  }
  return stoi(arg);
}

Arg Machine::parse_argument_labelic(string line) {
  size_t lp = line.find(LP);
  string arg = line.substr(lp + 1, line.find(RP) - lp - 1);
  return arg;
}

Lab Machine::parse_label(string line) { // seems to be good
  Lab ans;
  string extra = line.substr(line.find(RP) + 1, string::npos);
  if (extra.size() > 1) {
	ans = extra.substr(1, string::npos);
  }
  return ans;
}

Machine::Machine(string path) : file_(move(path)) {
  process_file(file_);
}

list<int> Machine::run() {
  process_file(file_);
  cout << "Commands: " << commands_.size() << endl;
  auto it = commands_.begin();
  while (it != commands_.end()) {
	auto f = COM_HAND.find(it->ctype_);
	if (f != COM_HAND.end()) {
	  it = f->second(*this, it);
	} else {
	  throw runtime_error(HANDLER_NOT_FOUND);
	}
	cout << endl << SEPARATOR << endl;
  }
  return output_;
}

void Machine::set_path(const string &path) {
  file_ = path;
}

void Machine::set_input(std::initializer_list<Val> vals) {
  input_.assign(vals);
}

void Machine::process_file(const string &path) {
  ifstream is(path);
  string buffer;
  while (getline(is, buffer)) {
	if (buffer.empty()) {
	  continue;
	}
	auto inserted = process_command(buffer);
	if (inserted->label_.has_value()) {
	  add_label(inserted);
	  cout << endl << "Added Label '" << inserted->label_.value() << "'";
	}
	cout << endl;
  }
  cout << "FILE PROCESSED" << endl;
}

list<Com>::iterator Machine::process_command(string line) {
  Com c = parse_command(move(line));
  return commands_.insert(commands_.end(), move(c));
}

void Machine::add_label(list<Com>::iterator it) {
  if (labels_.contains(it->label_.value())) {
	throw runtime_error(LABEL_USED_ERROR);
  }
  labels_[it->label_.value()] = it;
}

ComIt Machine::load(Machine &m, ComIt &c) {
  cout << "called LOAD\n";
  auto &mem = m.memory_;
  auto i = GET_VALUE(m, c);
  cout << "i == " << i << endl;
  mem[0] = i;
  cout << "mem.at(0) == " << mem.at(0);
  return ++c;
}

ComIt Machine::store(Machine &m, ComIt &c) {
  cout << "called STORE\n";
  auto &mem = m.memory_;
  auto buf = mem.at(0);
  auto i = GET_VALUE(m, c);
  cout << "i == " << i << endl;
  cout << "gonna store mem.at(0) == " << buf << " to i" << endl;
  mem[GET_VALUE(m, c)] = buf;
  cout << "at mem.at(i) == " << mem.at(i);
  return ++c;
}

ComIt Machine::add(Machine &m, ComIt &c) {
  cout << "called ADD\n";
  auto &mem = m.memory_;
  auto adding = mem.at(GET_VALUE(m, c));
  auto i = GET_VALUE(m, c);
  cout << "mem.at(0) == " << mem.at(0) << endl;
  cout << "i == " << i << endl;
  cout << "mem.at(i) == " << mem.at(i);
  mem[0] += adding;
  cout << "mem.at(0) == " << mem.at(0) << endl;
  return ++c;
}

ComIt Machine::sub(Machine &m, ComIt &c) {
  auto &mem = m.memory_;
  mem[0] -= mem.at(GET_VALUE(m, c));
  return ++c;
}

ComIt Machine::mult(Machine &m, ComIt &c) {
  auto &mem = m.memory_;
  mem[0] *= mem.at(GET_VALUE(m, c));
  return ++c;
}

ComIt Machine::div(Machine &m, ComIt &c) {
  auto &mem = m.memory_;
  mem[0] /= mem.at(GET_VALUE(m, c));
  return ++c;
}

ComIt Machine::read(Machine &m, ComIt &c) {
  auto &mem = m.memory_;
  auto i = GET_VALUE(m, c);
  cout << "i == " << i << endl;
  mem[GET_VALUE(m, c)] = m.input_.front();
  cout << "front is " << m.input_.front() << endl;
  m.input_.pop_front();
  cout << "front popped" << endl;
  cout << "mem.at(i) == " << mem.at(i);
  return ++c;
}

ComIt Machine::write(Machine &m, ComIt &c) {
  m.output_.push_back(GET_VALUE(m, c));
  return ++c;
}

ComIt Machine::jump(Machine &m, ComIt &c) {
  auto &labs = m.labels_;
  cout << "called JUMP\n";
  auto l = GET_LABEL(m, c);
  cout << "to label " << l << endl;
  if (labs.find(l) != labs.end()) {
	cout << "FOUND";
  } else {
	cout << "NOT FOUND";
  }
  return labs.at(l);
}

ComIt Machine::jgtz(Machine &m, ComIt &c) {
  auto &labs = m.labels_;
  return (m.memory_.at(0) > 0) ? labs.at(GET_LABEL(m, c)) : ++c;
}

ComIt Machine::jzero(Machine &m, ComIt &c) {
  auto &labs = m.labels_;
  return (m.memory_.at(0) == 0) ? labs.at(GET_LABEL(m, c)) : ++c;
}

ComIt Machine::halt(Machine &m, ComIt &c) {
  return m.commands_.end();
}

}