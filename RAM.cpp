#include "RAM.h"

using namespace std;

//ostream &operator<<(ostream &os, const list<int> &l) {
//  os << "{";
//  bool first = true;
//  for (const auto &pos: l) {
//	if (!first) {
//	  os << ", ";
//	}
//	first = false;
//	os << pos;
//  }
//  return os << "}";
//}

namespace RAM {

#define GET_VALUE(m, c) ARG_HANDLERS.at((c)->atype_)((m), (c)->arg_)

//template<typename T, typename... Ts>
//std::ostream &operator<<(std::ostream &os, const std::variant<T, Ts...> &v) {
//  std::visit([&os](auto &&arg) {
//	os << arg;
//  }, v);
//  return os;
//}

//template<typename T>
//auto getV(variant<int, string> v)-> decltype(v.) {
//
//}

ostream &operator<<(ostream &os, const Com &c) {
  os << c.ctype_ << " " << c.atype_ << " " << c.arg_.index() << " " << c.label_
	  .value();
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
static const string BAD_COMMAND_ERROR = "bad command name";
static const string NEGATIVE_ADDRESS_ERROR = "seen negative address";

static const set<ArgType> LAB_COMS = {
	JUMP,
	JGTZ,
	JZERO,
	HALT,
};

static const map<char, ArgType> CHARS_TYPES = {
	{EQ, VALUE},
	{PTR, ADDRESS_AT_ADDRESS},
};

const std::list<Com> &Machine::commands() const {
  return commands_;
}

int Machine::fromValToArg(const Machine &m, const Arg &a) {
  return get<int>(a);
}

int Machine::fromAddToArg(const Machine &m, const Arg &a) {
  if (get<int>(a) < 0) {
	throw runtime_error(NEGATIVE_ADDRESS_ERROR);
  }
  return m.memory_.at(get<int>(a)); // for STORE this does not make sense
}

int Machine::fromAddAtAddToArg(const Machine &m, const Arg &a) {
  if (get<int>(a) < 0) {
	throw runtime_error(NEGATIVE_ADDRESS_ERROR);
  }
  return m.memory_.at(m.memory_.at(get<int>(a)));
}

Com Machine::parseCom(string line) {
  auto label = parseLab(line);
  auto ct = parseComType(line);
  auto arg = (LAB_COMS.contains(ct)) ?
			 parseArgLab(line) :
			 parseArgNum(line);
  cout << "parsing com '" << line << "'" << ct
	   << "' for arg : ";

  if (holds_alternative<int>(arg)) {
	cout << "'" << get<int>(arg) << "'";
  } else {
	cout << "'" << get<string>(arg) << "'";
  }
  cout << endl;

  return {
	  move(ct),
	  move(parseArgType(line)),
	  move(arg),
	  move(label),
  };
}

ComType Machine::parseComType(string line) {
  auto it = COM_HANDLERS.find(line.substr(0, line.find(LP)));
  if (it == COM_HANDLERS.cend()) {
	throw runtime_error(BAD_COMMAND_ERROR);
  }
  return it->first;
}

ArgType Machine::parseArgType(string line) {
  auto it = CHARS_TYPES.find(line.at(line.find(LP) + 1));
  return (it != CHARS_TYPES.cend()) ? it->second : ADDRESS;
}

Arg Machine::parseArgNum(string line) {
  size_t lp = line.find(LP);
  string arg = line.substr(lp + 1, line.find(RP) - lp - 1);
  if (CHARS_TYPES.contains(arg.front())) {
	arg.erase(0, 1);
  }
//  cout << arg << endl;
  return stoi(arg);
}

Arg Machine::parseArgLab(string line) {
  size_t lp = line.find(LP);
  string arg = line.substr(lp + 1, line.find(RP) - lp - 1);
//  cout << arg << endl;
  return arg;
}

Lab Machine::parseLab(string line) { // seems to be good
  Lab ans = {};
  string extra = line.substr(line.find(RP) + 1, string::npos);
  if (extra.size() > 1) {
	ans = extra.substr(1, string::npos);
  }
//  cout << "seen label is '" << ans.value_or("___nv") << "'\n";
  return ans;
}

Machine::Machine(string path) : file_(move(path)) {
  processFile(file_);
}

list<int> Machine::run() {
  processFile(file_);
//  cout << "Commands seen " << commands_.size() << endl;
  auto it = commands_.begin();
  while (it != commands_.end()) {
	cout << it->atype_ << endl;
	cout << it->ctype_ << endl;
	cout << it->arg_.index() << endl;
	cout << it->label_.value_or("__nv") << endl;
	auto f = COM_HANDLERS.find(it->ctype_);
	if (f != COM_HANDLERS.end()) {
	  cout << "HANDLER FOUND\n";
	  it = f->second(*this, it);
	} else {
	  cout << "HANDLER NOT FOUND\n";
	}
  }
  return output_;
}

void Machine::setPath(const string &path) {
  file_ = path;
}

void Machine::setInput(const string &input) { // this is insane, need reformat
  string s;
  for (auto &c: input) {
	s.push_back(c);
	input_.push_back(stoi(s));
	s.clear();
  }
}

void Machine::processFile(const string &path) {
  ifstream is(path);
  string buffer;
  while (getline(is, buffer)) {
	if (buffer.empty()) {
	  continue;
	}
	cout << "now process " << buffer << endl;
	processCom(buffer);
  }
  cout << "file has been processed" << endl;
}

void Machine::processCom(string line) {
  Com c = parseCom(move(line));
  if (c.label_.has_value()) { // какое нахер хэс вэлью перепиши инвалид
	addLabel(c.label_);
  }
  commands_.push_back(move(c));
}

void Machine::addLabel(Lab lab) {
  if (labels_.contains(lab.value())) {
	throw runtime_error(LABEL_USED_ERROR);
  }
  labels_[lab.value()] = commands_.end();
}

ComIt Machine::load(Machine &m, ComIt &c) {
  cout << "called LOAD\n";
  m.memory_[0] = GET_VALUE(m, c);
  cout << m.memory_[0] << endl << "=============" << endl;
  return ++c;
}

ComIt Machine::store(Machine &m, ComIt &c) {
  cout << "called STORE\n";
  auto buf = m.memory_[0];
  cout << buf << endl;
  cout << "this arg will be put: ";
  cout.flush();
  auto ff = ARG_HANDLERS.find(c->atype_);
  int j = -5;
  if (ff != ARG_HANDLERS.cend()) {
	auto uu = ARG_HANDLERS.at(c->atype_);
	cout << "FF FOUND" << endl;
	cout << "path to arg handler " << &j << endl;
	cout.flush();
	j = uu(m, c->arg_);
	cout.flush();
	cout << j << endl;
  }
  cout.flush();
  m.memory_[j] = buf;
  cout.flush();
  cout << endl << "=================" << endl;
  return ++c;
}

ComIt Machine::add(Machine &m, ComIt &c) {
  m.memory_[0] += m.memory_.at(GET_VALUE(m, c));
  return ++c;
}

ComIt Machine::sub(Machine &m, ComIt &c) {
  m.memory_[0] -= m.memory_.at(GET_VALUE(m, c));
  return ++c;
}

ComIt Machine::mult(Machine &m, ComIt &c) {
  m.memory_[0] *= m.memory_.at(GET_VALUE(m, c));
  return ++c;
}

ComIt Machine::div(Machine &m, ComIt &c) {
  m.memory_[0] /= m.memory_.at(GET_VALUE(m, c));
  return ++c;
}

ComIt Machine::read(Machine &m, ComIt &c) {
  m.memory_[GET_VALUE(m, c)] = m.input_.front();
  m.input_.pop_front();
  return ++c;
}

ComIt Machine::write(Machine &m, ComIt &c) {
  m.output_.push_back(GET_VALUE(m, c));
  return ++c;
}

ComIt Machine::jump(Machine &m, ComIt &c) {
  return m.labels_.at(get<string>(c->arg_));
}

ComIt Machine::jgtz(Machine &m, ComIt &c) {
  return (m.memory_.at(0) > 0) ? m.labels_[get<string>(c->arg_)] : ++c;
}

ComIt Machine::jzero(Machine &m, ComIt &c) {
  return (m.memory_.at(0) == 0) ? m.labels_[get<string>(c->arg_)] : ++c;
}

ComIt Machine::halt(Machine &m, ComIt &c) {
  return m.commands_.end();
}

}