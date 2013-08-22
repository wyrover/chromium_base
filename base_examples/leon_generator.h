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

    int child_process_id_;
    int routing_id_;
  };
  struct ShutdownParams {
    ShutdownParams();
    ~ShutdownParams();
    int child_process_id_;
  };
  void NewLeon(const NewLeonParams& params);

  void DelLeon(const DelLeonParams& params);

  void Shutdown(const ShutdownParams& params);

  void OnPaint(HWND hWnd);
}
#endif