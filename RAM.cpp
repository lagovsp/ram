#include "RAM.h"

using namespace std;

ostream &operator<<(ostream &os, const list<int> &l) {
  os << "{";
  bool first = true;
  for (const auto &pos: l) {
	if (!first) {
	  os << ", ";
	}
	first = false;
	os << pos;
  }
  return os << "}";
}

namespace RAM {

template<typename T, typename... Ts>
std::ostream &operator<<(std::ostream &os, const std::variant<T, Ts...> &v) {
  std::visit([&os](auto &&arg) {
	os << arg;
  }, v);
  return os;
}

ostream &operator<<(ostream &os, const Command &c) {
  os << c.ctype_ << " " << c.atype_ << " " << c.arg_.index() << " " << c.label_
	  .value();
  return os;
}

static const char SPACE = ' ';
static const char LP = '(';
static const char RP = ')';
static const char EQ = '=';
static const char PTR = '*';
static const char COLON = ':';

static const string NUMERICS = "0123456789";

static const string LABEL_USED_ERROR = "label has already been used";
static const string BAD_COMMAND_ERROR = "bad command name";

static const unordered_set<ArgType> LABELIC_COMMANDS = {
	JUMP,
	JGTZ,
	JZERO,
	HALT,
};

static const unordered_map<char, ArgType> CHARS_TYPES = {
	{EQ, VALUE},
	{PTR, ADDRESS_AT_ADDRESS},
};

const std::list<Command> &Machine::commands() const {
  return commands_;
}

Arg Machine::fromValueToValue(const Machine &m, const Arg &a) {
  return a;
}

Arg Machine::fromAddressToValue(const Machine &m, const Arg &a) {
  if (get<int>(a) < 0) {
	throw runtime_error("bad cell index");
  }
  return m.memory_.at(get<int>(a));
}

Arg Machine::fromAddressAtAddressToValue(const Machine &m, const Arg &a) {
  return m.memory_.at(m.memory_.at(get<int>(a)));
}

Command Machine::parseCommand(std::string line) {
  auto label = parseLabel(line);
  auto ct = parseCommandType(line);
  auto arg = (LABELIC_COMMANDS.contains(ct)) ?
			 parseArgumentLabel(line) :
			 parseArgumentNumeric(line);
  return {
	  move(ct),
	  move(parseArgumentType(line)),
	  move(arg),
	  move(label),
  };
}

CommandType Machine::parseCommandType(string line) {
  auto it = COMMANDS_HANDLERS.find(line.substr(0, line.find(LP)));
  if (it == COMMANDS_HANDLERS.cend()) {
	throw runtime_error(BAD_COMMAND_ERROR);
  }
  return it->first;
}

ArgType Machine::parseArgumentType(string line) {
  auto mod = line.at(line.find(LP) + 1);
  if (auto it = CHARS_TYPES.find(mod); it != CHARS_TYPES.cend()) {
	return it->second;
  }
  return ADDRESS;
}

Arg Machine::parseArgumentNumeric(string line) {
  size_t lp = line.find(LP);
  string arg = line.substr(lp + 1, line.find(RP) - lp - 1);
  if (CHARS_TYPES.contains(arg.front())) {
	arg.erase(0, 1);
  }
//  cout << arg << endl;
  return stoi(arg);
}

Arg Machine::parseArgumentLabel(std::string line) {
  size_t lp = line.find(LP);
  string arg = line.substr(lp + 1, line.find(RP) - lp - 1);
//  cout << arg << endl;
  return arg;
}

optional<string> Machine::parseLabel(string line) {
  string extra = line.substr(line.find(RP) + 1, string::npos);
  optional<string> ans;
  if (!extra.empty()) {
	ans = extra;
  }
  return ans;
}

Machine::Machine(string path) : file_(move(path)) {
  processFile(file_);
}

list<int> Machine::run() {
  processFile(file_);
  cout << commands_.size() << endl;
  for (auto it = commands_.begin(); it != commands_.end(); ++it) {
	cout << it->atype_ << endl;
	auto a = ARGUMENTS_HANDLERS.at(it->atype_)(*this, it->arg_);
//	COMMANDS_HANDLERS.at(it->ctype_)(*this, a, it);
  }
  return output_;
}

void Machine::setPath(std::string path) {
  file_ = move(path);
}

void Machine::setInput(string input) {
  input_ = move(input);
}

void Machine::processFile(const string &path) {
  ifstream is(path);
  string buffer;
  while (getline(is, buffer)) {
	if (buffer.empty()) {
	  continue;
	}
	cout << "now process " << buffer << endl;
	processCommand(buffer);
  }
  cout << "file has been processed" << endl;
}

void Machine::processCommand(string line) {
  Command c = parseCommand(move(line));
  if (c.label_.has_value()) {
	addLabel(c.label_);
  }
  commands_.push_back(move(c));
}

void Machine::addLabel(std::optional<std::string> lab) {
  if (labels_.contains(lab.value())) {
	throw runtime_error(LABEL_USED_ERROR);
  }
  labels_[lab.value()] = commands_.end();
}

void Machine::load(Machine &m, const Arg &arg, CommandIt &nc) {
  m.memory_[0] = get<int>(arg);
}

void Machine::store(Machine &m, const Arg &arg, CommandIt &nc) {
  m.memory_[get<int>(arg)] = m.memory_[0];
}

void Machine::add(Machine &m, const Arg &arg, CommandIt &nc) {
  m.memory_[0] += m.memory_[get<int>(arg)];
}

void Machine::sub(Machine &m, const Arg &arg, CommandIt &nc) {
  m.memory_[0] -= m.memory_[get<int>(arg)];
}

void Machine::mult(Machine &m, const Arg &arg, CommandIt &nc) {
  m.memory_[0] *= m.memory_[get<int>(arg)];
}

void Machine::div(Machine &m, const Arg &arg, CommandIt &nc) {
  m.memory_[0] /= m.memory_[get<int>(arg)];
}

void Machine::read(Machine &m, const Arg &arg, CommandIt &nc) {
  m.memory_[get<int>(arg)] = m.input_.front();
}

void Machine::write(Machine &m, const Arg &arg, CommandIt &nc) {
  m.output_.push_back(get<int>(arg));
}

void Machine::jump(Machine &m, const Arg &arg, CommandIt &nc) {
  nc = m.labels_.at(get<string>(arg));
}

void Machine::jgtz(Machine &m, const Arg &arg, CommandIt &nc) {
  if (m.memory_[0] > 0) {
	nc = m.labels_[get<string>(arg)];
  }
}

void Machine::jzero(Machine &m, const Arg &arg, CommandIt &nc) {
  if (m.memory_[0] == 0) {
	nc = m.labels_[get<string>(arg)];
  }
}

void Machine::halt(Machine &m, const Arg &arg, CommandIt &nc) {
  nc = m.commands_.end();
}

}