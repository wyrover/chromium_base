#ifndef LEON_GENERATOR_H_
#define LEON_GENERATOR_H_

#include <Windows.h>

namespace LEON {
  struct NewLeonParams {
    NewLeonParams();
    ~NewLeonParams();

    bool create_process_;
  };
  struct DelLeonParams {
    DelLeonParams();
    ~DelLeonParams();

    int routing_id_;
  };
  void NewLeon(const NewLeonParams& params);

  void DelLeon(const DelLeonParams& params);

  void OnPaint(HWND hWnd);
}
#endif