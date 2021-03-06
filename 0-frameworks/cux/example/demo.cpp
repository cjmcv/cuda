#include "executor.h"

////////////
// Plugins.
extern cux::KernelInterface *DotProductGPUPlugin();
extern cux::KernelInterface *GemmGPUPlugin();

void DotProductTest() {
  cux::Executor *executor = new cux::Executor();
  executor->Initialize(0);
  executor->SelectOp("dot", "");

  // Add a user-defined kernel to the selected op.
  executor->AddPlugin(DotProductGPUPlugin(), cux::OpRunMode::ON_DEVICE);

  // Data preparation.
  // Too large a value may cause overflow.
  const int data_len = 4096000; // data_len % threads_per_block == 0.
  cux::Array4D *in_a = new cux::Array4D(1, 1, 1, data_len);
  cux::Array4D *in_b = new cux::Array4D(1, 1, 1, data_len);
  cux::Array4D *out = new cux::Array4D(1, 1, 1, 1);

  std::vector<cux::Array4D*> inputs;
  inputs.push_back(in_a);
  inputs.push_back(in_b);
  std::vector<cux::Array4D*> outputs;
  outputs.push_back(out);

  // Bind And Fill.
  executor->BindAndFill(inputs, outputs, -2, 2, 0);

  // Run.
  executor->Run(cux::OpRunMode::ON_HOST);
  executor->Run(cux::OpRunMode::ON_DEVICE);

  delete in_a;
  delete in_b;
  delete out;

  executor->Clear();
  delete executor;
}

void Nrm2Test() {
  cux::Executor *executor = new cux::Executor();
  executor->Initialize(0);
  executor->SelectOp("nrm2", "");

  // Data preparation.
  // Too large a value may cause overflow.
  const int data_len = 4096000; // data_len % threads_per_block == 0.
  cux::Array4D *in = new cux::Array4D(1, 1, 1, data_len);
  cux::Array4D *out = new cux::Array4D(1, 1, 1, 1);

  std::vector<cux::Array4D*> inputs;
  inputs.push_back(in);
  std::vector<cux::Array4D*> outputs;
  outputs.push_back(out);

  // Bind And Fill.
  executor->BindAndFill(inputs, outputs, -1, 1, 0);

  // Run.
  executor->Run(cux::OpRunMode::ON_HOST);
  executor->Run(cux::OpRunMode::ON_DEVICE);

  delete in;
  delete out;

  executor->Clear();
  delete executor;
}

void GEMMTest() {
  cux::Executor *executor = new cux::Executor();
  executor->Initialize(0);
  executor->SelectOp("gemm", "alpha: 1.0, beta: 3.0");

  // Add a user-defined kernel to the selected op.
  executor->AddPlugin(GemmGPUPlugin(), cux::OpRunMode::ON_DEVICE);

  // Data preparation.
  int block_size = 32;
  cux::Array4D *in_a = new cux::Array4D(1, 1, block_size * 16, block_size * 15);
  cux::Array4D *in_b = new cux::Array4D(1, 1, block_size * 15, block_size * 18);
  std::vector<int> shape_a = in_a->shape();
  std::vector<int> shape_b = in_b->shape();
  cux::Array4D *out_c = new cux::Array4D(1, 1, shape_a[cux::HEIGHT], shape_b[cux::WIDTH]);

  std::vector<cux::Array4D*> inputs;
  inputs.push_back(in_a);
  inputs.push_back(in_b);
  std::vector<cux::Array4D*> outputs;
  outputs.push_back(out_c);

  // Bind And Fill.
  executor->BindAndFill(inputs, outputs, -2, 5, 0);
  
  // Run.
  executor->Run(cux::OpRunMode::ON_HOST);
  out_c->Restore(cux::TypeFlag::FLOAT32, cux::ON_HOST); // For beta in gemm.
  executor->Run(cux::OpRunMode::ON_DEVICE);

  delete in_a;
  delete in_b;
  delete out_c;

  executor->Clear();
  delete executor;
}

int main() {
  cux::InitEnvironment();
  //cux::QueryDevices();

  ////////
  printf("DotProductTest.\n");
  DotProductTest();

  printf("\n\n");

  printf("Norm2Test.\n");
  Nrm2Test();

  printf("\n\n");

  printf("GEMMTest.\n");
  GEMMTest();
  ////////

  cux::CleanUpEnvironment();
  return 0;
}