#include <cstdint>
#include <filesystem>
#include <fstream>
#include <numeric>
#include <sstream>
#include <string>

#include <torch/script.h>

#include "test_tensor_utils.hpp"

// Function to read the file and create a tensor

namespace test_utils
{

auto read_file_to_tensor_f32(const std::filesystem::path& abs_filepath, torch::IntArrayRef shape) -> torch::Tensor
{
    auto instream = std::ifstream {abs_filepath, std::ios::in};
    if (!instream.is_open()) {
        auto err_msg = std::stringstream {};
        err_msg << "Failed to open file: " << abs_filepath.string() << '\n';
        throw std::ios_base::failure {err_msg.str()};
    }

    const auto n_elements =
        std::accumulate(shape.begin(), shape.end(), std::int64_t {1}, [](auto x, auto y) { return x * y; });

    auto output = torch::empty({n_elements}, torch::dtype(torch::kFloat32));

    auto value = float {};

    // Read the file line by line
    for (std::int64_t i {0}; i < n_elements; ++i) {
        instream >> value;
        output[i] = value;
    }

    auto output_reshaped = output.reshape(shape);

    return output_reshaped;
}

auto almost_equal_relative(const torch::Tensor& left, const torch::Tensor& right, double rtol) -> bool
{
    if (left.sizes() != right.sizes()) {
        return false;
    }

    // Compute the absolute difference
    torch::Tensor abs_diff = torch::abs(left - right);

    // Compute the relative tolerance
    torch::Tensor relative_diff = abs_diff / torch::clamp(torch::abs(right), 1e-8);

    // Check if all elements are within the tolerance
    return torch::all(relative_diff <= rtol).item<bool>();
}

}  // namespace test_utils
