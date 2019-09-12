#pragma once
#include <exception>
#include <string>

class Exception : public std::exception {
  public:
    Exception(std::string info);
    Exception(const Exception &other);
    ~Exception();

    Exception &set_info(std::string info);
    Exception &set_line(size_t line);
    Exception &set_column(size_t column);
    Exception &set_hint(std::string hint);

    const char *what() const throw();

  private:
    Exception();
    Exception &operator=(const Exception &other);

    void build_pretty_info();
    template<typename T>
    Exception &set_member(T Exception::* member, T value);

    std::string pretty_info;

    std::string info;
    size_t line = 0;
    size_t column = 0;
    std::string hint;
};
