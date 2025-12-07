#include "couttast/tinytast.hpp"
#include "wwjson.hpp"

#include <charconv>

DEF_TOOL(to_chars, "test std::to_chars")
{
    std::string json;
    std::array<char, 32> buffer;
    auto [ptr, ec] = std::to_chars(buffer.data(), buffer.data() + buffer.size(), 100);
    json.append(buffer.data(), ptr);
    COUT(json);

    // compile error for double?
    {
        std::string json;
        std::array<char, 32> buffer;
        auto [ptr, ec] = std::to_chars(buffer.data(), buffer.data() + buffer.size(),500);
        json.append(buffer.data(), ptr);
        COUT(json);
    }

    {
        uint64_t maxInt = -1;
        std::string strMaxInt = std::to_string(maxInt);
        COUT(strMaxInt);
        COUT(strMaxInt.size());
    }

    COUT(wwjson::has_float_to_chars_v<float>);
    COUT(wwjson::has_float_to_chars_v<double>);
}

