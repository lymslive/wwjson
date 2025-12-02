#include "test_util.h"
#include "xyjson.h"

namespace test
{

bool IsJsonValid(const std::string& json)
{
    yyjson::Document doc(json);
    return doc.isValid();
}

} /* test */ 
