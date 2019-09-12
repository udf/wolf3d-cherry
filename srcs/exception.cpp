
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
auto Exception::set_member(T Exception::* member, T value)
    -> Exception &
{
    this->*member = value;
    this->build_pretty_info();
    return *this;
}

auto Exception::set_info(std::string info) -> Exception &{
    return set_member(&Exception::info, info);
}

auto Exception::set_line(size_t line) -> Exception &{
    return set_member(&Exception::line, line);
}

auto Exception::set_column(size_t column) -> Exception &{
    return set_member(&Exception::column, column);
}

auto Exception::set_hint(std::string hint) -> Exception &{
    return set_member(&Exception::hint, hint);
}

auto Exception::what() const throw() -> const char *{
    return this->pretty_info.c_str();
}

// Error [on line {line}[, column {column}]]: info "{hint}"
auto Exception::build_pretty_info() -> void{
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