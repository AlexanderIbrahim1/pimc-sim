#pragma once

#include <filesystem>

#include <torch/script.h>

namespace test_utils
{

auto read_file_to_tensor_f32(const std::filesystem::path& abs_filepath, torch::IntArrayRef shape) -> torch::Tensor;
auto almost_equal_relative(const torch::Tensor& left, const torch::Tensor& right, double rtol = 1.0e-5) -> bool;

}  // namespace test_utils
