#ifndef MAIN_RUNNER_H_
#define MAIN_RUNNER_H_

#include "../base/basictypes.h"

class MainRunner {
public:
  virtual ~MainRunner() {}

  static MainRunner* Create();

  virtual int Initialize() = 0;

  virtual int Run() = 0;

  virtual void Shutdown() = 0;
};

#endif