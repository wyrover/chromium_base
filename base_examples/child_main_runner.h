#ifndef CHILD_MAIN_RUNNER_H_
#define CHILD_MAIN_RUNNER_H_

class ChildMainRunner {
public:
  ~ChildMainRunner() {}

  static ChildMainRunner* Create();

  virtual int Initialize() = 0;

  virtual int Run() = 0;

  virtual void Shutdown() = 0;
};

#endif