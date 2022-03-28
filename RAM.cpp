#include "RAM.h"

using namespace std;

namespace RAM {

static const char LP = '(';
static const char RP = ')';
static const char EQ = '=';
static const char PTR = '*';

static const string LABEL_USED_ERROR = "label already used";
static const string BAD_COMMAND_ERROR = "bad command";
static const string HANDLER_NOT_FOUND = "handler not found";
static const string PATH_NOT_SET_ERROR = "no file path";

static const string FILE_PROCESSED_MSG = "File processed";
static const string COMMANDS_MSG = "Commands";
static const string CALLED_MSG = "Called";
static const string MEMORY_MSG = "Memory";

static const string SEPARATOR = "----------------------------------------";

#define GET_ARG(m, c) \
    RAM::Machine::GROUPS.at((c)->ctype_).at((c)->atype_)((m), (c)->arg_)

#define GET_VALUE(m, c) get<int>(GET_ARG((m), (c)))
#define GET_LABEL(m, c) get<string>(GET_ARG((m), (c)))

#define ON_CALLED_INFO(c) \
    do{                      \
    cout << CALLED_MSG << " " << (c)->ctype_         \
    << " " << (c)->arg_                              \
    << " " << (c)->atype_                            \
    << " " << (c)->label_.value_or("")               \
    << endl; while(false)

#define TELL_MEMORY(m) cout << MEMORY_MSG << (m).memory_

//#define INFO(var) #var << "=" << var

ostream &operator<<(ostream &os, const Arg &a) {
  if (a.index() == 0) { os << get<int>(a); }
  else { os << get<string>(a); }
  return os;
}

//ostream &operator<<(ostream &os, const Com &c) {
//  os << INFO(c.ctype_) << " " << INFO(c.atype_) << " " << INFO(c.arg_) << " "
//	 << INFO(c.label_.value_or("__nv"));
//  return os;
//}

ostream &operator<<(ostream &os, const Tape &c) {
  os << "{";
  bool first = true;
  for (const auto &pos: c) {
	if (!first) { os << ", "; }
	first = false;
	os << pos;
  }
  os << "}";
  return os;
}

ostream &operator<<(ostream &os, const Memory &m) {
  os << "{";
  bool first = true;
  for (const auto &[key, value]: m) {
	if (!first) { os << ", "; }
	first = false;
	os << "[" << key << "] -> " << value;
  }
  os << "}";
  return os;
}

static const set<ArgT> LAB_COMS = {
	Machine::JUMP,
	Machine::JGTZ,
	Machine::JZERO,
	Machine::HALT,
};

static const map<char, ArgT> CHARS_TYPES = {
	{EQ, Machine::VALUE},
	{PTR, Machine::ADDRESS_AT_ADDRESS},
};

Arg Machine::value_to_argument(const Machine &m, const Arg &a) {
  return get<int>(a);
}

Arg Machine::address_to_argument(const Machine &m, const Arg &a) {
  return m.memory_.at(get<int>(a));
}

Arg Machine::address_to_address_to_argument(const Machine &m, const Arg &a) {
//  return m.memory_.at(m.memory_.at(get<int>(a)));
  return address_to_argument(m, address_to_argument(m, a));
}

Arg Machine::label_to_argument(const Machine &m, const Arg &a) {
  return get<string>(a);
}

Com Machine::parse_command(const string &line) {
  auto ct = parse_command_t(line);
  auto at = (LAB_COMS.contains(ct)) ? LABEL : parse_argument_t(line);
  auto l = parse_label(line);
  auto a = (LAB_COMS.contains(ct)) ?
		   parse_argument_label(line) :
		   parse_argument_numeric(line);
  return {move(ct), move(at), move(a), move(l)};
}

ComT Machine::parse_command_t(const string &line) {
  auto it = COM_HAND.find(line.substr(0, line.find(LP)));
  if (it == COM_HAND.cend()) {
	throw runtime_error(BAD_COMMAND_ERROR);
  }
  return it->first;
}

ArgT Machine::parse_argument_t(const string &line) {
  auto it = CHARS_TYPES.find(line.at(line.find(LP) + 1));
  return (it != CHARS_TYPES.cend()) ? it->second : ADDRESS;
}

Arg Machine::parse_argument_numeric(const string &line) {
  size_t lp = line.find(LP);
  string arg = line.substr(lp + 1, line.find(RP) - lp - 1);
  if (CHARS_TYPES.contains(arg.front())) {
	arg.erase(0, 1);
  }
  return stoi(arg);
}

Arg Machine::parse_argument_label(const string &line) {
  size_t lp = line.find(LP);
  return line.substr(lp + 1, line.find(RP) - lp - 1);
}

Lab Machine::parse_label(const string &line) {
  Lab ans;
  string extra = line.substr(line.find(RP) + 1, string::npos);
  if (extra.size() > 1) {
	ans = extra.substr(1, string::npos);
  }
  return ans;
}

Machine::Machine(const string &path) : file_(path) {}

Tape Machine::run() {
  process_file();
  auto it = commands_.begin();
  while (it != commands_.cend()) {
	auto f = COM_HAND.find(it->ctype_);
	if (f != COM_HAND.end()) {
	  it = f->second(*this, it);
	} else {
	  throw runtime_error(HANDLER_NOT_FOUND);
	}
	cout << endl;
  }
  return output_;
}

void Machine::set_path(const string &path) {
  file_ = path;
}

void Machine::set_input(std::initializer_list<Val> vals) {
  input_.assign(vals);
}

void Machine::be_verbose(bool flag) {
  verbose_ = flag;
}

void Machine::set_ostream(std::ostream &os) {
  out_ = &os;
}

void Machine::process_file() {
  if (!file_) {
	throw runtime_error(PATH_NOT_SET_ERROR);
  }
  ifstream is(file_.value());
  string buffer;
  while (getline(is, buffer)) {
	if (buffer.empty()) {
	  continue;
	}
	auto inserted = process_command(buffer);
	if (inserted->label_.has_value()) {
	  add_label(inserted);
	}
  }
  cout << FILE_PROCESSED_MSG << endl;
  cout << COMMANDS_MSG << " " << commands_.size() << endl;

}

ComIt Machine::process_command(const string &line) {
  Com c = parse_command(line);
  return commands_.insert(commands_.end(), move(c));
}

void Machine::add_label(list<Com>::iterator it) {
  if (labels_.contains(it->label_.value())) {
	throw runtime_error(LABEL_USED_ERROR);
  }
  labels_[it->label_.value()] = it;
}

ComIt Machine::load(Machine &m, ComIt &c) {
  if (m.verbose_) {
	ON_CALLED_INFO(c);
	TELL_MEMORY(m);
  }
  auto &mem = m.memory_;
//  cout << *c << endl;
  cout << mem << endl;
  auto i = GET_VALUE(m, c);
//  cout << "i == " << i << endl;
  mem[0] = i;
//  cout << "mem.at(0) == " << mem.at(0);
  cout << mem << endl;
  return ++c;
}

ComIt Machine::store(Machine &m, ComIt &c) {
  ON_CALLED_INFO(c);
  auto &mem = m.memory_;
//  cout << *c << endl;
  cout << mem << endl;
  auto buf = mem.at(0);
  auto i = GET_VALUE(m, c);
//  cout << "i == " << i << endl;
//  cout << "gonna store mem.at(0) == " << buf << " to i" << endl;
  mem[GET_VALUE(m, c)] = buf;
//  cout << "at mem.at(i) == " << mem.at(i);
  cout << mem << endl;
  return ++c;
}

ComIt Machine::add(Machine &m, ComIt &c) {
  ON_CALLED_INFO(c);
  auto &mem = m.memory_;
//  cout << *c << endl;
  cout << mem << endl;
  auto v = GET_VALUE(m, c);
//  cout << "mem.at(0) == " << mem.at(0) << endl;
//  cout << "i == " << i << endl;
//  cout << "mem.at(i) == " << mem.at(i);
  mem[0] += v;
//  cout << "mem.at(0) == " << mem.at(0) << endl;
  cout << mem << endl;
  return ++c;
}

ComIt Machine::sub(Machine &m, ComIt &c) {
  ON_CALLED_INFO(c);
  auto &mem = m.memory_;
//  cout << *c << endl;
  cout << mem << endl;
  auto v = GET_VALUE(m, c);
//  cout << "mem.at(0) == " << mem.at(0) << endl;
//  cout << "i == " << i << endl;
//  cout << "mem.at(i) == " << mem.at(i);
  mem[0] -= v;
//  cout << "mem.at(0) == " << mem.at(0) << endl;
  cout << mem << endl;
  return ++c;
}

ComIt Machine::mult(Machine &m, ComIt &c) {
  ON_CALLED_INFO(c);
  auto &mem = m.memory_;
//  cout << *c << endl;
  cout << mem << endl;
  auto v = GET_VALUE(m, c);
//  cout << "mem.at(0) == " << mem.at(0) << endl;
//  cout << "i == " << i << endl;
//  cout << "mem.at(i) == " << mem.at(i);
  mem[0] *= v;
//  cout << "mem.at(0) == " << mem.at(0) << endl;
  cout << mem << endl;
  return ++c;
}

ComIt Machine::div(Machine &m, ComIt &c) {
  ON_CALLED_INFO(c);
  auto &mem = m.memory_;
//  cout << *c << endl;
  cout << mem << endl;
  auto v = GET_VALUE(m, c);
//  cout << "mem.at(0) == " << mem.at(0) << endl;
//  cout << "i == " << i << endl;
//  cout << "mem.at(i) == " << mem.at(i);
  mem[0] /= v;
//  cout << "mem.at(0) == " << mem.at(0) << endl;
  cout << mem << endl;
  return ++c;
}

ComIt Machine::read(Machine &m, ComIt &c) {
  ON_CALLED_INFO(c);
  auto &mem = m.memory_;
//  cout << *c << endl;
  cout << mem << endl;
  cout << "input: " << m.input_ << endl;
  auto i = GET_VALUE(m, c);
//  cout << "i == " << i << endl;
  mem[GET_VALUE(m, c)] = m.input_.front();
//  cout << "front is " << m.input_.front() << endl;
  m.input_.pop_front();
  cout << "front popped" << endl;
//  cout << "mem.at(i) == " << mem.at(i);
  cout << mem << endl;
  cout << "input: " << m.input_ << endl;
  return ++c;
}

ComIt Machine::write(Machine &m, ComIt &c) {
  ON_CALLED_INFO(c);
  auto &mem = m.memory_;
//  cout << *c << endl;
  cout << mem << endl;
  cout << "output: " << m.output_ << endl;
  auto letter = GET_VALUE(m, c);
  cout << "will write '" << letter << "'" << endl;
  m.output_.push_back(GET_VALUE(m, c));
  cout << "full out tape: " << m.output_ << endl;
  return ++c;
}

ComIt Machine::jump(Machine &m, ComIt &c) {
  ON_CALLED_INFO(c);
  auto &mem = m.memory_;
//  cout << *c << endl;
  cout << mem << endl;
  auto &labs = m.labels_;
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
  ON_CALLED_INFO(c);
  auto &mem = m.memory_;
//  cout << *c << endl;
  cout << mem << endl;
  auto &labs = m.labels_;
  cout << "mem.at(0) == " << m.memory_.at(0) << endl;
  return (m.memory_.at(0) > 0) ? labs.at(GET_LABEL(m, c)) : ++c;
}

ComIt Machine::jzero(Machine &m, ComIt &c) {
  ON_CALLED_INFO(c);
  auto &mem = m.memory_;
//  cout << *c << endl;
  cout << mem << endl;
  auto &labs = m.labels_;
  cout << "mem.at(0) == " << m.memory_.at(0) << endl;
  return (m.memory_.at(0) == 0) ? labs.at(GET_LABEL(m, c)) : ++c;
}

ComIt Machine::halt(Machine &m, ComIt &c) {
  ON_CALLED_INFO(c);
  return m.commands_.end();
}

}