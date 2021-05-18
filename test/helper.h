#pragma once
#include <gtest/gtest.h>
#include <string>
#include <vector>

std::vector<short> read_sample_file(const std::string& filename);
bool file_exists(const std::string& name);
std::string detect_project_root();