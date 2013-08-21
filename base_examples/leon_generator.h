#ifndef LEON_GENERATOR_H_
#define LEON_GENERATOR_H_

namespace LEON {
  struct GenerateParams {
    GenerateParams();
    ~GenerateParams();

    bool create_process_;
  };
  void Generate(const GenerateParams& params);

  void DelLeon(int routing_id);
}
#endif