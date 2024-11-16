#pragma once

// because max clashes with c++17
#define NOMINMAX
#include <string>
using std::string, std::to_string;

#include <vector>
using std::vector;

#include <set>
using std::set;

#include <exception>
using std::exception;

#include <stdexcept>
using std::runtime_error, std::invalid_argument;

#include <cstdint>

#include <map>

#include <optional>
using std::optional;

#include <limits>    // Necessary for std::numeric_limits
#include <algorithm> // Necessary for std::clamp

#include <mutex>
using std::shared_ptr, std::make_shared, std::unique_ptr, std::make_unique;

#include <boost/core/noncopyable.hpp>

#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>
using vk::Extent2D;

#include <filesystem>
using std::filesystem::path;


using std::pair;