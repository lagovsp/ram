// Copyright 2022 Sergey Lagov lagovsp@gmail.com

#include "ram.h"

using namespace std;

namespace RAM {

static const char LP = '(';
static const char RP = ')';
static const char EQ = '=';
static const char PTR = '*';

static const string LABEL_USED_ERROR = "label already used";
static const string LABEL_NOT_FOUND_ERROR = "label not found";
static const string BAD_COMMAND_ERROR = "bad command";
static const string PATH_NOT_SET_ERROR = "no file path";
static const string FILE_NOT_FOUND_ERROR = "file not found";

static const string FILE_PROCESSED_MSG = "File processed";
static const string COMMANDS_RECOGNIZED_MSG = "Commands recognized";
static const string COMMANDS_EXECUTED_MSG = "Commands executed";
static const string RUNNING_MSG = "Running...";
static const string CALLED_MSG = "Called";
static const string MEMORY_BEFORE_MSG = "Memory before";
static const string MEMORY_AFTER_MSG = "Memory after";
static const string OUT_TAPE_MSG = "Out tape";
static const string IN_TAPE_MSG = "In tape";
static const string FINISHED_MSG = "Program finished";

#define GET_ARG(m, c)                                                        \
    RAM::Machine::GROUPS.at((c)->ctype_).at((c)->atype_)((m), (c)->arg_)

#define GET_VALUE(m, c) get<int>(GET_ARG((m), (c)))
#define GET_LABEL(m, c) get<string>(GET_ARG((m), (c)))

#define TELL_MEMORY(m, status_msg)                                          \
    *(m).out_ << (status_msg) << "\t " << (m).memory_ << endl

#define VERBOSE_INFO(m, c)                                                  \
    *(m).out_ << CALLED_MSG << " " << (c)->ctype_                           \
    << " " << (c)->arg_                                                     \
    << " " << (c)->atype_                                                   \
    << " " << (c)->label_.value_or("")                                      \
    << endl;                                                                \
    TELL_MEMORY((m), MEMORY_BEFORE_MSG)

ostream &operator<<(ostream &os, const Arg &a) {
  if (a.index() == 0) { os << get<int>(a); }
  else { os << get<string>(a); }
  return os;
}

ostream &operator<<(ostream &os, const Tape &c) {
  os << "{ ";
  bool first = true;
  for (const auto &pos: c) {
	if (!first) { os << ", "; }
	first = false;
	os << pos;
  }
  os << " }";
  return os;
}

ostream &operator<<(ostream &os, const Memory &m) {
  os << "{ ";
  bool first = true;
  for (const auto &[key, value]: m) {
	if (!first) { os << ", "; }
	first = false;
	os << "(" << key << ")->" << value;
  }
  os << " }";
  return os;
}

static const set<ArgT> LABEL_COMS = {
	Machine::JUMP,
	Machine::JGTZ,
	Machine::JZERO,
	Machine::HALT,
};

static const map<char, ArgT> ARG_TYPES = {
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
  return address_to_argument(m, address_to_argument(m, a));
}

Arg Machine::label_to_argument(const Machine &m, const Arg &a) {
  return get<string>(a);
}

Com Machine::parse_command(const string &line) {
  auto ct = parse_command_t(line);
  auto at = (LABEL_COMS.contains(ct)) ? LABEL : parse_argument_t(line);
  auto l = parse_label(line);
  auto a = (LABEL_COMS.contains(ct)) ?
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
  auto it = ARG_TYPES.find(line.at(line.find(LP) + 1));
  return (it != ARG_TYPES.cend()) ? it->second : ADDRESS;
}

Arg Machine::parse_argument_numeric(const string &line) {
  size_t lp = line.find(LP);
  string arg = line.substr(lp + 1, line.find(RP) - lp - 1);
  if (ARG_TYPES.contains(arg.front())) {
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

Tape Machine::run() {
  process_file();
  auto it = commands_.begin();
  uint64_t executed = 0;
  *out_ << IN_TAPE_MSG << " " << input_ << endl;
  if (verbose_) {
	*out_ << RUNNING_MSG << endl;
  }
  while (it != commands_.cend()) {
	auto f = COM_HAND.at(it->ctype_);
	if (verbose_) {
	  VERBOSE_INFO(*this, it);
	}
	++executed;
	it = f(*this, it);
	if (verbose_) {
	  TELL_MEMORY(*this, MEMORY_AFTER_MSG);
	}
  }
  if (verbose_) {
	*out_ << FINISHED_MSG << endl;
	*out_ << COMMANDS_EXECUTED_MSG << " " << executed << endl;
  }
  *out_ << OUT_TAPE_MSG << " " << output_ << endl;
  return output_;
}

void Machine::set_path(const string &path) {
  file_ = path;
}

void Machine::set_input(initializer_list<Val> vals) {
  input_.assign(vals);
}

void Machine::be_verbose(bool flag) {
  verbose_ = flag;
}

void Machine::set_ostream(ostream &os) {
  out_ = &os;
}

void Machine::process_file() {
  if (!file_) {
	throw runtime_error(PATH_NOT_SET_ERROR);
  }
  ifstream is(file_.value());
  if (!is) {
	throw runtime_error(FILE_NOT_FOUND_ERROR);
  }
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
  *out_ << FILE_PROCESSED_MSG << " " << file_.value() << endl;
  if (verbose_) {
	*out_ << COMMANDS_RECOGNIZED_MSG << " " << commands_.size() << endl;
  }
}

ComIt Machine::process_command(const string &line) {
  Com c = parse_command(line);
  return commands_.insert(commands_.end(), move(c));
}

void Machine::add_label(ComIt it) {
  if (labels_.contains(it->label_.value())) {
	throw runtime_error(LABEL_USED_ERROR);
  }
  labels_[it->label_.value()] = it;
}

ComIt Machine::load(Machine &m, ComIt &c) {
  m.memory_[0] = GET_VALUE(m, c);
  return ++c;
}

ComIt Machine::store(Machine &m, ComIt &c) {
  auto buf = m.memory_.at(0);
  m.memory_[GET_VALUE(m, c)] = buf;
  return ++c;
}

ComIt Machine::add(Machine &m, ComIt &c) {
  auto buf = GET_VALUE(m, c);
  m.memory_[0] += buf;
  return ++c;
}

ComIt Machine::sub(Machine &m, ComIt &c) {
  auto buf = GET_VALUE(m, c);
  m.memory_[0] -= buf;
  return ++c;
}

ComIt Machine::mult(Machine &m, ComIt &c) {
  auto buf = GET_VALUE(m, c);
  m.memory_[0] *= buf;
  return ++c;
}

ComIt Machine::div(Machine &m, ComIt &c) {
  auto buf = GET_VALUE(m, c);
  m.memory_[0] /= buf;
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
  auto &labs = m.labels_;
  auto l = GET_LABEL(m, c);
  auto found = labs.find(l);
  if (found == labs.end()) {
	throw runtime_error(LABEL_NOT_FOUND_ERROR);
  }
  return found->second;
}

ComIt Machine::jgtz(Machine &m, ComIt &c) {
  if (m.memory_.at(0) == 0) {
	return ++c;
  }
  auto &labs = m.labels_;
  auto l = GET_LABEL(m, c);
  auto found = labs.find(l);
  if (found == labs.end()) {
	throw runtime_error(LABEL_NOT_FOUND_ERROR);
  }
  return found->second;
}

ComIt Machine::jzero(Machine &m, ComIt &c) {
  if (m.memory_.at(0) != 0) {
	return ++c;
  }
  auto &labs = m.labels_;
  auto l = GET_LABEL(m, c);
  auto found = labs.find(l);
  if (found == labs.end()) {
	throw runtime_error(LABEL_NOT_FOUND_ERROR);
  }
  return found->second;
}

ComIt Machine::halt(Machine &m, ComIt &c) {
  return m.commands_.end();
}

}