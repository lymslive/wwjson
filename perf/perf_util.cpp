#include "perf_util.h"
#include "xyjson.h"

#include <string>

namespace test
{

bool IsJsonValid(const std::string &json)
{
    yyjson::Document doc(json);
    return doc.isValid();
}

bool IsJsonEqual(const std::string &left, const std::string& right)
{
    yyjson::Document docLeft(left);
    yyjson::Document docRight(right);
    return docLeft.isValid() && docRight.isValid() && left == right;
}

} // namespace test
