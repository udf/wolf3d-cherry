#pragma once

class View {
  private:
    View(const View &other) = delete;
    View &operator=(const View &other) = delete;

  public:
    View();
    ~View();
};