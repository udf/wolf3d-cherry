#include "exception.hpp"

Exception::Exception(
    const std::string info
) {
    this->info = info;
    this->build_pretty_info();
}

Exception::Exception(const Exception &other) {
    this->pretty_info = other.pretty_info;
    this->info = other.info;
    this->line = other.line;
    this->column = other.column;
    this->hint = other.hint;
}

Exception::~Exception() {
}

template<typename T>
Exception &Exception::set_member(T Exception::* member, T value)
{
    this->*member = value;
    this->build_pretty_info();
    return *this;
}

Exception &Exception::set_info(std::string info) {
    return set_member(&Exception::info, info);
}

Exception &Exception::set_line(size_t line) {
    return set_member(&Exception::line, line);
}

Exception &Exception::set_column(size_t column) {
    return set_member(&Exception::column, column);
}

Exception &Exception::set_hint(std::string hint) {
    return set_member(&Exception::hint, hint);
}

const char *Exception::what() const throw() {
    return this->pretty_info.c_str();
}

// Error [on line {line}[, column {column}]]: info "{hint}"
void Exception::build_pretty_info() {
    this->pretty_info = "Error";
    if (this->line) {
        this->pretty_info += " on line " + std::to_string(this->line);
        if (this->column) {
            this->pretty_info += ", column " + std::to_string(this->column);
        }
    }
    this->pretty_info += ": " + this->info;
    if (!this->hint.empty()) {
        this->pretty_info += " \"" + this->hint + '"';
    }
}