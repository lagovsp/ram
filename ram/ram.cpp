// Copyright 2022 Sergey Lagov lagovsp@gmail.com

#include "ram.h"

using namespace std;

namespace RAM {

static const char SP = ' ';
static const char LP = '(';
static const char RP = ')';
static const char EQ = '=';
static const char PTR = '*';

static const string LABEL_USED_ERROR = "label already used";
static const string LABEL_NOT_FOUND_ERROR = "label not found";
static const string BAD_ARG_TYPE_ERROR = "bad argument type";
static const string BAD_COMMAND_ERROR = "bad command";
static const string BAD_ISTREAM_ERROR = "bad istream";
static const string NO_MORE_INPUT_ERROR = "no more input";

static const string MACHINE_NAME_MSG = "Machine name set ";
static const string INPUT_RECEIVED_MSG = "Input set ";
static const string CODE_SET_MSG = "Code set ";

static const string COMMANDS_RECOGNIZED_MSG = "Commands recognized ";
static const string COMMANDS_EXECUTED_MSG = "Commands executed ";
static const string REACHING_UNINIT_MSG = "Reaching uninitialized cell ";

static const string OUT_TAPE_MSG = "Output: ";

static const string RUNNING_MSG = "RUNNING ";
static const string FINISHED_MSG = "PROGRAM FINISHED ";
static const string WARNING_MSG = "WARNING ";

#define LOG(m, msg)                                                          \
    do { if ((m).out_) { *(m).out_ << msg << endl; } } while(false)

#define LOG_VERBOSE(m, msg)                                                  \
    do { if ((m).verbose_) { LOG((m),msg); } } while(false)

#define STOPPED_WITH(m, msg)                                                 \
    do {                                                                     \
        (m).stop_ = true;                                                    \
        LOG((m), msg);                                                       \
        throw runtime_error((msg));                                          \
    } while(false)

#define CHECK_LABEL_PRESENT(m, lab)                                          \
    do {                                                                     \
        auto f = (m).labels_.find(lab);                                      \
        if (f == (m).labels_.cend()) {                                       \
            (m).stop_ = true;                                                \
            STOPPED_WITH((m), LABEL_NOT_FOUND_ERROR);                        \
        }                                                                    \
    } while(false)

#define COMMAND_INFO(num, c)                                                 \
    setw(2) << (num) << ". " << (c)->ctype_                                  \
    << " " << (c)->arg_                                                      \
    << " " << (c)->atype_                                                    \
    << " " << (c)->label_.value_or("")

#define GET_VALUE(m, c) get<Val>((m).get_arg((c)))
#define GET_LABEL(m, c) get<string>((m).get_arg((c)))

ostream &operator<<(ostream &os, const Arg &a) {
  if (a.index() == 0) { os << get<Val>(a); }
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
  return os << " }";
}

ostream &operator<<(ostream &os, const Memory &m) {
  os << "{ ";
  bool first = true;
  for (const auto &[key, value]: m) {
	if (!first) { os << ", "; }
	first = false;
	os << "(" << key << ")->" << value;
  }
  return os << " }";
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

Arg Machine::get_arg(ComIt c) {
  const auto allowed_types = COM_HAND.at(c->ctype_).first;
  const auto a_handler = allowed_types.find(c->atype_);
  if (a_handler == allowed_types.cend()) {
	STOPPED_WITH(*this, BAD_ARG_TYPE_ERROR);
  }
  return a_handler->second(*this, c->arg_);
}

Arg Machine::value(Machine &m, const Arg &a) {
  return get<Val>(a);
}

Arg Machine::direct_value(Machine &m, const Arg &a) {
  auto f = m.memory_.find(get<Val>(a));
  if (f == m.memory_.cend()) {
	LOG_VERBOSE(m, WARNING_MSG << REACHING_UNINIT_MSG);
	return m.memory_[get<Val>(a)] = static_cast<Val>(Machine::gen_());
  }
  return f->second;
}

Arg Machine::indirect_value(Machine &m, const Arg &a) {
  return direct_value(m, direct_value(m, a));
}

Arg Machine::label(Machine &m, const Arg &a) {
  return get<string>(a);
}

Com Machine::parse_command(const string &line) {
  auto ct = get_com_type(line);
  auto at = (LABEL_COMS.contains(ct)) ? LABEL : get_arg_type(line);
  auto l = get_label(line);
  auto a = (LABEL_COMS.contains(ct)) ?
		   get_arg_label(line) :
		   get_arg_number(line);
  return {move(ct), move(at), move(a), move(l)};
}

ComT Machine::get_com_type(const string &line) {
  auto it = COM_HAND.find(line.substr(0, line.find(LP)));
  if (it == COM_HAND.cend()) {
	throw runtime_error(BAD_COMMAND_ERROR);
  }
  return it->first;
}

ArgT Machine::get_arg_type(const string &line) {
  auto it = ARG_TYPES.find(line.at(line.find(LP) + 1));
  return (it != ARG_TYPES.cend()) ? it->second : ADDRESS;
}

Arg Machine::get_arg_number(const string &line) {
  size_t lp = line.find(LP);
  string arg = line.substr(lp + 1, line.find(RP) - lp - 1);
  if (ARG_TYPES.contains(arg.front())) {
	arg.erase(0, 1);
  }
  return stoi(arg);
}

Arg Machine::get_arg_label(const string &line) {
  size_t lp = line.find(LP);
  return line.substr(lp + 1, line.find(RP) - lp - 1);
}

Lab Machine::get_label(const string &line) {
  Lab ans;
  string extra = line.substr(line.find(RP) + 1, string::npos);
  if (extra.size() > 1) {
	ans = extra.substr(1, string::npos);
  }
  return ans;
}

Tape Machine::run() {
  output_.clear();
  auto it = commands_.begin();
  uint64_t executed = 0;
  LOG_VERBOSE(*this, RUNNING_MSG << name_);

  while (!stop_ && it != commands_.cend()) {
	auto f = COM_HAND.at(it->ctype_);
	auto cmit = it;
	++executed;
	it = f.second(*this, it);
	LOG_VERBOSE(*this, COMMAND_INFO(executed, cmit) << " -> \t\t" << memory_);
  }

  LOG_VERBOSE(*this, FINISHED_MSG << endl);
  LOG_VERBOSE(*this, COMMANDS_EXECUTED_MSG << executed);
  LOG(*this, OUT_TAPE_MSG << output_);
  return output_;
}

const Tape &Machine::input() const {
  return input_;
}

const Tape &Machine::output() const {
  return output_;
}

void Machine::set_name(const Name &name) {
  name_ = name;
  LOG(*this, MACHINE_NAME_MSG << name_ << endl);
}

void Machine::set_code(istream &is) {
  if (!is) {
	STOPPED_WITH(*this, BAD_ISTREAM_ERROR);
  }
  memory_.clear();
  labels_.clear();
  commands_.clear();
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
  LOG(*this, CODE_SET_MSG);
  LOG_VERBOSE(*this, COMMANDS_RECOGNIZED_MSG << commands_.size() << endl);
}

void Machine::set_input(istream &is) {
  if (!is) {
	STOPPED_WITH(*this, BAD_ISTREAM_ERROR);
  }
  input_.clear();
  string s;
  while (getline(is, s, SP)) {
	input_.push_back(stoi(s));
  }
  LOG(*this, INPUT_RECEIVED_MSG << input_ << endl);
}

void Machine::set_log_stream(ostream &os) {
  out_ = &os;
}

void Machine::set_input(initializer_list<Val> vals) {
  input_.clear();
  input_.assign(vals);
  LOG(*this, INPUT_RECEIVED_MSG << input_ << endl);
}

void Machine::be_verbose(bool flag) {
  verbose_ = flag;
}

void Machine::reset() {
  name_.clear();
  input_.clear();
  output_.clear();
  memory_.clear();
  labels_.clear();
  commands_.clear();
  stop_ = false;
  verbose_ = false;
  out_ = &cout;
}

ComIt Machine::process_command(const string &line) {
  Com c = parse_command(line);
  return commands_.insert(commands_.end(), move(c));
}

void Machine::add_label(ComIt it) {
  if (labels_.contains(it->label_.value())) {
	STOPPED_WITH(*this, LABEL_USED_ERROR);
  }
  labels_[it->label_.value()] = it;
}

ComIt Machine::load(Machine &m, ComIt &c) {
  auto v = GET_VALUE(m, c);
  m.memory_[0] = v;
  return ++c;
}

ComIt Machine::store(Machine &m, ComIt &c) {
  auto v = GET_VALUE(m, c);
  auto buf = m.memory_.at(0);
  m.memory_[v] = buf;
  return ++c;
}

ComIt Machine::add(Machine &m, ComIt &c) {
  auto v = GET_VALUE(m, c);
  m.memory_[0] += v;
  return ++c;
}

ComIt Machine::sub(Machine &m, ComIt &c) {
  auto v = GET_VALUE(m, c);
  m.memory_[0] -= v;
  return ++c;
}

ComIt Machine::mult(Machine &m, ComIt &c) {
  auto v = GET_VALUE(m, c);
  m.memory_[0] *= v;
  return ++c;
}

ComIt Machine::div(Machine &m, ComIt &c) {
  auto v = GET_VALUE(m, c);
  m.memory_[0] /= v;
  return ++c;
}

ComIt Machine::read(Machine &m, ComIt &c) {
  if (m.input_.empty()) {
	STOPPED_WITH(m, NO_MORE_INPUT_ERROR);
  }
  auto v = GET_VALUE(m, c);
  m.memory_[v] = m.input_.front();
  m.input_.pop_front();
  return ++c;
}

ComIt Machine::write(Machine &m, ComIt &c) {
  auto v = GET_VALUE(m, c);
  m.output_.push_back(v);
  return ++c;
}

ComIt Machine::jump(Machine &m, ComIt &c) {
  auto v = GET_LABEL(m, c);
  CHECK_LABEL_PRESENT(m, v);
  auto f = m.labels_.find(v);
  return f->second;
}

ComIt Machine::jgtz(Machine &m, ComIt &c) {
  if (m.memory_.at(0) < 1) {
	return ++c;
  }
  auto v = GET_LABEL(m, c);
  CHECK_LABEL_PRESENT(m, v);
  auto f = m.labels_.find(v);
  return f->second;
}

ComIt Machine::jzero(Machine &m, ComIt &c) {
  if (m.memory_.at(0) != 0) {
	return ++c;
  }
  auto v = GET_LABEL(m, c);
  CHECK_LABEL_PRESENT(m, v);
  auto f = m.labels_.find(v);
  return f->second;
}

ComIt Machine::halt(Machine &m, ComIt &c) {
  return m.commands_.end();
}

}