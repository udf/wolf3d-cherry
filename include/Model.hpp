#pragma once

class Model {
  private:
    Model(const Model &other) = delete;
    Model &operator=(const Model &other) = delete;

  public:
    Model() {
    }
    ~Model() {
    }
};