#ifndef LEON_GENERATOR_H_
#define LEON_GENERATOR_H_

namespace LEON {
  struct GenerateParams {
    GenerateParams();
    ~GenerateParams();
  };
  void Generate(GenerateParams* params);
}
#endif