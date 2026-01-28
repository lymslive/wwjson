#pragma once

#include <string>

namespace test
{

bool IsJsonValid(const std::string &json);
bool IsJsonEqual(const std::string &left, const std::string& right);

} // namespace test
