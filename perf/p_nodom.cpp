#include "couttast/tinytast.hpp"

#include "argv.h"
#include "relative_perf.h"

#include "wwjson.hpp"
#include "jbuilder.hpp"

#include <string>
#include <vector>
#include <sstream>
#include <cstdio>
#include <cstring>

namespace test::perf
{

// Reference JSON for verification (compact format)
// Generated from DataItem with 50 fields and RootData with 4 fields
static const char* REFERENCE_JSON = R"({"status":"success","code":"200","message":"OK","data":{"field_1":"value_001","field_2":"value_002","field_3":"value_003","field_4":"value_004","field_5":"value_005","field_6":"value_006","field_7":"value_007","field_8":"value_008","field_9":"value_009","field_10":"value_010","field_11":"value_011","field_12":"value_012","field_13":"value_013","field_14":"value_014","field_15":"value_015","field_16":"value_016","field_17":"value_017","field_18":"value_018","field_19":"value_019","field_20":"value_020","field_21":"value_021","field_22":"value_022","field_23":"value_023","field_24":"value_024","field_25":"value_025","field_26":"value_026","field_27":"value_027","field_28":"value_028","field_29":"value_029","field_30":"value_030","field_31":"value_031","field_32":"value_032","field_33":"value_033","field_34":"value_034","field_35":"value_035","field_36":"value_036","field_37":"value_037","field_38":"value_038","field_39":"value_039","field_40":"value_040","field_41":"value_041","field_42":"value_042","field_43":"value_043","field_44":"value_044","field_45":"value_045","field_46":"value_046","field_47":"value_047","field_48":"value_048","field_49":"value_049","field_50":"value_050"}})";

// Sub-struct with 50 fields (values are longer strings)
struct DataItem
{
    std::string field_1 = "value_001";
    std::string field_2 = "value_002";
    std::string field_3 = "value_003";
    std::string field_4 = "value_004";
    std::string field_5 = "value_005";
    std::string field_6 = "value_006";
    std::string field_7 = "value_007";
    std::string field_8 = "value_008";
    std::string field_9 = "value_009";
    std::string field_10 = "value_010";
    std::string field_11 = "value_011";
    std::string field_12 = "value_012";
    std::string field_13 = "value_013";
    std::string field_14 = "value_014";
    std::string field_15 = "value_015";
    std::string field_16 = "value_016";
    std::string field_17 = "value_017";
    std::string field_18 = "value_018";
    std::string field_19 = "value_019";
    std::string field_20 = "value_020";
    std::string field_21 = "value_021";
    std::string field_22 = "value_022";
    std::string field_23 = "value_023";
    std::string field_24 = "value_024";
    std::string field_25 = "value_025";
    std::string field_26 = "value_026";
    std::string field_27 = "value_027";
    std::string field_28 = "value_028";
    std::string field_29 = "value_029";
    std::string field_30 = "value_030";
    std::string field_31 = "value_031";
    std::string field_32 = "value_032";
    std::string field_33 = "value_033";
    std::string field_34 = "value_034";
    std::string field_35 = "value_035";
    std::string field_36 = "value_036";
    std::string field_37 = "value_037";
    std::string field_38 = "value_038";
    std::string field_39 = "value_039";
    std::string field_40 = "value_040";
    std::string field_41 = "value_041";
    std::string field_42 = "value_042";
    std::string field_43 = "value_043";
    std::string field_44 = "value_044";
    std::string field_45 = "value_045";
    std::string field_46 = "value_046";
    std::string field_47 = "value_047";
    std::string field_48 = "value_048";
    std::string field_49 = "value_049";
    std::string field_50 = "value_050";
};

// Root struct with 4 fields + data object
struct RootData
{
    std::string status = "success";
    std::string code = "200";
    std::string message = "OK";
    DataItem data;
};

// ============================================================================
// Builder Method Template
// ============================================================================
template <typename BuilderT>
class BuilderMethodT
{
public:
    void Build(RootData& data, std::string& out)
    {
        BuilderT builder(4096);

        builder.BeginObject();

        builder.AddMember("status", data.status);
        builder.AddMember("code", data.code);
        builder.AddMember("message", data.message);

        builder.BeginObject("data");

        auto& item = data.data;
        builder.AddMember("field_1", item.field_1);
        builder.AddMember("field_2", item.field_2);
        builder.AddMember("field_3", item.field_3);
        builder.AddMember("field_4", item.field_4);
        builder.AddMember("field_5", item.field_5);
        builder.AddMember("field_6", item.field_6);
        builder.AddMember("field_7", item.field_7);
        builder.AddMember("field_8", item.field_8);
        builder.AddMember("field_9", item.field_9);
        builder.AddMember("field_10", item.field_10);
        builder.AddMember("field_11", item.field_11);
        builder.AddMember("field_12", item.field_12);
        builder.AddMember("field_13", item.field_13);
        builder.AddMember("field_14", item.field_14);
        builder.AddMember("field_15", item.field_15);
        builder.AddMember("field_16", item.field_16);
        builder.AddMember("field_17", item.field_17);
        builder.AddMember("field_18", item.field_18);
        builder.AddMember("field_19", item.field_19);
        builder.AddMember("field_20", item.field_20);
        builder.AddMember("field_21", item.field_21);
        builder.AddMember("field_22", item.field_22);
        builder.AddMember("field_23", item.field_23);
        builder.AddMember("field_24", item.field_24);
        builder.AddMember("field_25", item.field_25);
        builder.AddMember("field_26", item.field_26);
        builder.AddMember("field_27", item.field_27);
        builder.AddMember("field_28", item.field_28);
        builder.AddMember("field_29", item.field_29);
        builder.AddMember("field_30", item.field_30);
        builder.AddMember("field_31", item.field_31);
        builder.AddMember("field_32", item.field_32);
        builder.AddMember("field_33", item.field_33);
        builder.AddMember("field_34", item.field_34);
        builder.AddMember("field_35", item.field_35);
        builder.AddMember("field_36", item.field_36);
        builder.AddMember("field_37", item.field_37);
        builder.AddMember("field_38", item.field_38);
        builder.AddMember("field_39", item.field_39);
        builder.AddMember("field_40", item.field_40);
        builder.AddMember("field_41", item.field_41);
        builder.AddMember("field_42", item.field_42);
        builder.AddMember("field_43", item.field_43);
        builder.AddMember("field_44", item.field_44);
        builder.AddMember("field_45", item.field_45);
        builder.AddMember("field_46", item.field_46);
        builder.AddMember("field_47", item.field_47);
        builder.AddMember("field_48", item.field_48);
        builder.AddMember("field_49", item.field_49);
        builder.AddMember("field_50", item.field_50);

        builder.EndObject();
        builder.EndObject();

        out = builder.MoveResult();
    }
};

// Type aliases for convenience
using RawBuilderMethod = BuilderMethodT<::wwjson::RawBuilder>;
using BuilderMethod = BuilderMethodT<::wwjson::Builder>;
using FastBuilderMethod = BuilderMethodT<::wwjson::FastBuilder>;

// ============================================================================
// Method B1: snprintf (single format string with all fields)
// ============================================================================
class SnprintfMethod
{
public:
    void Build(RootData& data, std::string& out)
    {
        // Single snprintf call with all field names in format string
        // and all field values as variadic arguments
        char buf[4096];
        snprintf(buf, sizeof(buf),
            R"({"status":"%s","code":"%s","message":"%s","data":{)"
            R"("field_1":"%s","field_2":"%s","field_3":"%s","field_4":"%s","field_5":"%s",)"
            R"("field_6":"%s","field_7":"%s","field_8":"%s","field_9":"%s","field_10":"%s",)"
            R"("field_11":"%s","field_12":"%s","field_13":"%s","field_14":"%s","field_15":"%s",)"
            R"("field_16":"%s","field_17":"%s","field_18":"%s","field_19":"%s","field_20":"%s",)"
            R"("field_21":"%s","field_22":"%s","field_23":"%s","field_24":"%s","field_25":"%s",)"
            R"("field_26":"%s","field_27":"%s","field_28":"%s","field_29":"%s","field_30":"%s",)"
            R"("field_31":"%s","field_32":"%s","field_33":"%s","field_34":"%s","field_35":"%s",)"
            R"("field_36":"%s","field_37":"%s","field_38":"%s","field_39":"%s","field_40":"%s",)"
            R"("field_41":"%s","field_42":"%s","field_43":"%s","field_44":"%s","field_45":"%s",)"
            R"("field_46":"%s","field_47":"%s","field_48":"%s","field_49":"%s","field_50":"%s")"
            R"(}})",
            // Root fields
            data.status.c_str(), data.code.c_str(), data.message.c_str(),
            // Data item fields (50 fields)
            data.data.field_1.c_str(), data.data.field_2.c_str(),
            data.data.field_3.c_str(), data.data.field_4.c_str(),
            data.data.field_5.c_str(), data.data.field_6.c_str(),
            data.data.field_7.c_str(), data.data.field_8.c_str(),
            data.data.field_9.c_str(), data.data.field_10.c_str(),
            data.data.field_11.c_str(), data.data.field_12.c_str(),
            data.data.field_13.c_str(), data.data.field_14.c_str(),
            data.data.field_15.c_str(), data.data.field_16.c_str(),
            data.data.field_17.c_str(), data.data.field_18.c_str(),
            data.data.field_19.c_str(), data.data.field_20.c_str(),
            data.data.field_21.c_str(), data.data.field_22.c_str(),
            data.data.field_23.c_str(), data.data.field_24.c_str(),
            data.data.field_25.c_str(), data.data.field_26.c_str(),
            data.data.field_27.c_str(), data.data.field_28.c_str(),
            data.data.field_29.c_str(), data.data.field_30.c_str(),
            data.data.field_31.c_str(), data.data.field_32.c_str(),
            data.data.field_33.c_str(), data.data.field_34.c_str(),
            data.data.field_35.c_str(), data.data.field_36.c_str(),
            data.data.field_37.c_str(), data.data.field_38.c_str(),
            data.data.field_39.c_str(), data.data.field_40.c_str(),
            data.data.field_41.c_str(), data.data.field_42.c_str(),
            data.data.field_43.c_str(), data.data.field_44.c_str(),
            data.data.field_45.c_str(), data.data.field_46.c_str(),
            data.data.field_47.c_str(), data.data.field_48.c_str(),
            data.data.field_49.c_str(), data.data.field_50.c_str());

        out = buf;
    }
};

// ============================================================================
// Method B2: std::string::append (one field at a time)
// ============================================================================
class StringAppendMethod
{
public:
    void Build(RootData& data, std::string& out)
    {
        out.clear();
        out.reserve(4096);

        // Root object start
        out += "{";

        // Root fields (one at a time)
        out += "\"status\":\"";
        out += data.status;
        out += "\",";

        out += "\"code\":\"";
        out += data.code;
        out += "\",";

        out += "\"message\":\"";
        out += data.message;
        out += "\",";

        // Data object start
        out += "\"data\":{";

        // Data item fields (one at a time)
        auto& item = data.data;

        out += "\"field_1\":\"";
        out += item.field_1;
        out += "\",";

        out += "\"field_2\":\"";
        out += item.field_2;
        out += "\",";

        out += "\"field_3\":\"";
        out += item.field_3;
        out += "\",";

        out += "\"field_4\":\"";
        out += item.field_4;
        out += "\",";

        out += "\"field_5\":\"";
        out += item.field_5;
        out += "\",";

        out += "\"field_6\":\"";
        out += item.field_6;
        out += "\",";

        out += "\"field_7\":\"";
        out += item.field_7;
        out += "\",";

        out += "\"field_8\":\"";
        out += item.field_8;
        out += "\",";

        out += "\"field_9\":\"";
        out += item.field_9;
        out += "\",";

        out += "\"field_10\":\"";
        out += item.field_10;
        out += "\",";

        out += "\"field_11\":\"";
        out += item.field_11;
        out += "\",";

        out += "\"field_12\":\"";
        out += item.field_12;
        out += "\",";

        out += "\"field_13\":\"";
        out += item.field_13;
        out += "\",";

        out += "\"field_14\":\"";
        out += item.field_14;
        out += "\",";

        out += "\"field_15\":\"";
        out += item.field_15;
        out += "\",";

        out += "\"field_16\":\"";
        out += item.field_16;
        out += "\",";

        out += "\"field_17\":\"";
        out += item.field_17;
        out += "\",";

        out += "\"field_18\":\"";
        out += item.field_18;
        out += "\",";

        out += "\"field_19\":\"";
        out += item.field_19;
        out += "\",";

        out += "\"field_20\":\"";
        out += item.field_20;
        out += "\",";

        out += "\"field_21\":\"";
        out += item.field_21;
        out += "\",";

        out += "\"field_22\":\"";
        out += item.field_22;
        out += "\",";

        out += "\"field_23\":\"";
        out += item.field_23;
        out += "\",";

        out += "\"field_24\":\"";
        out += item.field_24;
        out += "\",";

        out += "\"field_25\":\"";
        out += item.field_25;
        out += "\",";

        out += "\"field_26\":\"";
        out += item.field_26;
        out += "\",";

        out += "\"field_27\":\"";
        out += item.field_27;
        out += "\",";

        out += "\"field_28\":\"";
        out += item.field_28;
        out += "\",";

        out += "\"field_29\":\"";
        out += item.field_29;
        out += "\",";

        out += "\"field_30\":\"";
        out += item.field_30;
        out += "\",";

        out += "\"field_31\":\"";
        out += item.field_31;
        out += "\",";

        out += "\"field_32\":\"";
        out += item.field_32;
        out += "\",";

        out += "\"field_33\":\"";
        out += item.field_33;
        out += "\",";

        out += "\"field_34\":\"";
        out += item.field_34;
        out += "\",";

        out += "\"field_35\":\"";
        out += item.field_35;
        out += "\",";

        out += "\"field_36\":\"";
        out += item.field_36;
        out += "\",";

        out += "\"field_37\":\"";
        out += item.field_37;
        out += "\",";

        out += "\"field_38\":\"";
        out += item.field_38;
        out += "\",";

        out += "\"field_39\":\"";
        out += item.field_39;
        out += "\",";

        out += "\"field_40\":\"";
        out += item.field_40;
        out += "\",";

        out += "\"field_41\":\"";
        out += item.field_41;
        out += "\",";

        out += "\"field_42\":\"";
        out += item.field_42;
        out += "\",";

        out += "\"field_43\":\"";
        out += item.field_43;
        out += "\",";

        out += "\"field_44\":\"";
        out += item.field_44;
        out += "\",";

        out += "\"field_45\":\"";
        out += item.field_45;
        out += "\",";

        out += "\"field_46\":\"";
        out += item.field_46;
        out += "\",";

        out += "\"field_47\":\"";
        out += item.field_47;
        out += "\",";

        out += "\"field_48\":\"";
        out += item.field_48;
        out += "\",";

        out += "\"field_49\":\"";
        out += item.field_49;
        out += "\",";

        out += "\"field_50\":\"";
        out += item.field_50;
        out += "\"";

        // Data object end, root object end
        out += "}}";
    }
};

// ============================================================================
// Method B3: stringstream << (one field at a time)
// ============================================================================
class StringStreamMethod
{
public:
    void Build(RootData& data, std::string& out)
    {
        std::ostringstream oss;

        // Root object start
        oss << "{";

        // Root fields (one at a time)
        oss << "\"status\":\"";
        oss << data.status;
        oss << "\",";

        oss << "\"code\":\"";
        oss << data.code;
        oss << "\",";

        oss << "\"message\":\"";
        oss << data.message;
        oss << "\",";

        // Data object start
        oss << "\"data\":{";

        // Data item fields (one at a time)
        auto& item = data.data;

        oss << "\"field_1\":\"";
        oss << item.field_1;
        oss << "\",";

        oss << "\"field_2\":\"";
        oss << item.field_2;
        oss << "\",";

        oss << "\"field_3\":\"";
        oss << item.field_3;
        oss << "\",";

        oss << "\"field_4\":\"";
        oss << item.field_4;
        oss << "\",";

        oss << "\"field_5\":\"";
        oss << item.field_5;
        oss << "\",";

        oss << "\"field_6\":\"";
        oss << item.field_6;
        oss << "\",";

        oss << "\"field_7\":\"";
        oss << item.field_7;
        oss << "\",";

        oss << "\"field_8\":\"";
        oss << item.field_8;
        oss << "\",";

        oss << "\"field_9\":\"";
        oss << item.field_9;
        oss << "\",";

        oss << "\"field_10\":\"";
        oss << item.field_10;
        oss << "\",";

        oss << "\"field_11\":\"";
        oss << item.field_11;
        oss << "\",";

        oss << "\"field_12\":\"";
        oss << item.field_12;
        oss << "\",";

        oss << "\"field_13\":\"";
        oss << item.field_13;
        oss << "\",";

        oss << "\"field_14\":\"";
        oss << item.field_14;
        oss << "\",";

        oss << "\"field_15\":\"";
        oss << item.field_15;
        oss << "\",";

        oss << "\"field_16\":\"";
        oss << item.field_16;
        oss << "\",";

        oss << "\"field_17\":\"";
        oss << item.field_17;
        oss << "\",";

        oss << "\"field_18\":\"";
        oss << item.field_18;
        oss << "\",";

        oss << "\"field_19\":\"";
        oss << item.field_19;
        oss << "\",";

        oss << "\"field_20\":\"";
        oss << item.field_20;
        oss << "\",";

        oss << "\"field_21\":\"";
        oss << item.field_21;
        oss << "\",";

        oss << "\"field_22\":\"";
        oss << item.field_22;
        oss << "\",";

        oss << "\"field_23\":\"";
        oss << item.field_23;
        oss << "\",";

        oss << "\"field_24\":\"";
        oss << item.field_24;
        oss << "\",";

        oss << "\"field_25\":\"";
        oss << item.field_25;
        oss << "\",";

        oss << "\"field_26\":\"";
        oss << item.field_26;
        oss << "\",";

        oss << "\"field_27\":\"";
        oss << item.field_27;
        oss << "\",";

        oss << "\"field_28\":\"";
        oss << item.field_28;
        oss << "\",";

        oss << "\"field_29\":\"";
        oss << item.field_29;
        oss << "\",";

        oss << "\"field_30\":\"";
        oss << item.field_30;
        oss << "\",";

        oss << "\"field_31\":\"";
        oss << item.field_31;
        oss << "\",";

        oss << "\"field_32\":\"";
        oss << item.field_32;
        oss << "\",";

        oss << "\"field_33\":\"";
        oss << item.field_33;
        oss << "\",";

        oss << "\"field_34\":\"";
        oss << item.field_34;
        oss << "\",";

        oss << "\"field_35\":\"";
        oss << item.field_35;
        oss << "\",";

        oss << "\"field_36\":\"";
        oss << item.field_36;
        oss << "\",";

        oss << "\"field_37\":\"";
        oss << item.field_37;
        oss << "\",";

        oss << "\"field_38\":\"";
        oss << item.field_38;
        oss << "\",";

        oss << "\"field_39\":\"";
        oss << item.field_39;
        oss << "\",";

        oss << "\"field_40\":\"";
        oss << item.field_40;
        oss << "\",";

        oss << "\"field_41\":\"";
        oss << item.field_41;
        oss << "\",";

        oss << "\"field_42\":\"";
        oss << item.field_42;
        oss << "\",";

        oss << "\"field_43\":\"";
        oss << item.field_43;
        oss << "\",";

        oss << "\"field_44\":\"";
        oss << item.field_44;
        oss << "\",";

        oss << "\"field_45\":\"";
        oss << item.field_45;
        oss << "\",";

        oss << "\"field_46\":\"";
        oss << item.field_46;
        oss << "\",";

        oss << "\"field_47\":\"";
        oss << item.field_47;
        oss << "\",";

        oss << "\"field_48\":\"";
        oss << item.field_48;
        oss << "\",";

        oss << "\"field_49\":\"";
        oss << item.field_49;
        oss << "\",";

        oss << "\"field_50\":\"";
        oss << item.field_50;
        oss << "\"";

        // Data object end, root object end
        oss << "}}";

        out = oss.str();
    }
};

// ============================================================================
// Test Classes (RelativeTimer derived)
// ============================================================================

// Test: RawBuilder vs snprintf
class RawBuilderVsSnprintf : public RelativeTimer<RawBuilderVsSnprintf>
{
public:
    RootData data;
    std::string resultA;
    std::string resultB;

    RawBuilderVsSnprintf() = default;

    void methodA()
    {
        RawBuilderMethod builder;
        builder.Build(data, resultA);
    }

    void methodB()
    {
        SnprintfMethod builder;
        builder.Build(data, resultB);
    }

    bool methodVerify()
    {
        methodA();
        std::string tempA = resultA;
        methodB();
        std::string tempB = resultB;
        return tempA == tempB && tempA == REFERENCE_JSON;
    }
};

// Test: RawBuilder vs string::append
class RawBuilderVsAppend : public RelativeTimer<RawBuilderVsAppend>
{
public:
    RootData data;
    std::string resultA;
    std::string resultB;

    RawBuilderVsAppend() = default;

    void methodA()
    {
        RawBuilderMethod builder;
        builder.Build(data, resultA);
    }

    void methodB()
    {
        StringAppendMethod builder;
        builder.Build(data, resultB);
    }

    bool methodVerify()
    {
        methodA();
        std::string tempA = resultA;
        methodB();
        std::string tempB = resultB;
        return tempA == tempB && tempA == REFERENCE_JSON;
    }
};

// Test: RawBuilder vs stringstream
class RawBuilderVsStream : public RelativeTimer<RawBuilderVsStream>
{
public:
    RootData data;
    std::string resultA;
    std::string resultB;

    RawBuilderVsStream() = default;

    void methodA()
    {
        RawBuilderMethod builder;
        builder.Build(data, resultA);
    }

    void methodB()
    {
        StringStreamMethod builder;
        builder.Build(data, resultB);
    }

    bool methodVerify()
    {
        methodA();
        std::string tempA = resultA;
        methodB();
        std::string tempB = resultB;
        return tempA == tempB && tempA == REFERENCE_JSON;
    }
};

// Test: Builder vs string::append
class BuilderVsAppend : public RelativeTimer<BuilderVsAppend>
{
public:
    RootData data;
    std::string resultA;
    std::string resultB;

    BuilderVsAppend() = default;

    void methodA()
    {
        BuilderMethod builder;
        builder.Build(data, resultA);
    }

    void methodB()
    {
        StringAppendMethod builder;
        builder.Build(data, resultB);
    }

    bool methodVerify()
    {
        methodA();
        std::string tempA = resultA;
        methodB();
        std::string tempB = resultB;
        return tempA == tempB && tempA == REFERENCE_JSON;
    }
};

// Test: FastBuilder vs string::append
class FastBuilderVsAppend : public RelativeTimer<FastBuilderVsAppend>
{
public:
    RootData data;
    std::string resultA;
    std::string resultB;

    FastBuilderVsAppend() = default;

    void methodA()
    {
        FastBuilderMethod builder;
        builder.Build(data, resultA);
    }

    void methodB()
    {
        StringAppendMethod builder;
        builder.Build(data, resultB);
    }

    bool methodVerify()
    {
        methodA();
        std::string tempA = resultA;
        methodB();
        std::string tempB = resultB;
        return tempA == tempB && tempA == REFERENCE_JSON;
    }
};

} // namespace test::perf

// ============================================================================
// Test Cases
// ============================================================================

DEF_TAST(nodom_raw_vs_snprintf, "wwjson RawBuilder vs snprintf性能对比")
{
    test::CArgv argv;
    DESC("Args: --loop=%d", argv.loop);

    auto tester = test::perf::RawBuilderVsSnprintf();

    double ratio = tester.runAndPrint("RawBuilder vs Snprintf",
                                      "RawBuilder", "snprintf",
                                      argv.loop, 10);
    COUT(ratio < 1.5, true);
}

DEF_TAST(nodom_raw_vs_append, "wwjson RawBuilder vs string::append性能对比")
{
    test::CArgv argv;
    DESC("Args: --loop=%d", argv.loop);

    auto tester = test::perf::RawBuilderVsAppend();

    double ratio = tester.runAndPrint("RawBuilder vs Append",
                                      "RawBuilder", "string::append",
                                      argv.loop, 10);
    COUT(ratio < 1.5, true);
}

DEF_TAST(nodom_raw_vs_stream, "wwjson RawBuilder vs stringstream性能对比")
{
    test::CArgv argv;
    DESC("Args: --loop=%d", argv.loop);

    auto tester = test::perf::RawBuilderVsStream();

    double ratio = tester.runAndPrint("RawBuilder vs Stream",
                                      "RawBuilder", "stringstream",
                                      argv.loop, 10);
    COUT(ratio < 1.0, true);
}

DEF_TAST(nodom_builder_vs_append, "wwjson Builder vs string::append性能对比")
{
    test::CArgv argv;
    DESC("Args: --loop=%d", argv.loop);

    auto tester = test::perf::BuilderVsAppend();

    double ratio = tester.runAndPrint("Builder vs Append",
                                      "Builder", "string::append",
                                      argv.loop, 10);
    COUT(ratio < 1.2, true);
}

DEF_TAST(nodom_fastbuilder_vs_append, "wwjson FastBuilder vs string::append性能对比")
{
    test::CArgv argv;
    DESC("Args: --loop=%d", argv.loop);

    auto tester = test::perf::FastBuilderVsAppend();

    double ratio = tester.runAndPrint("FastBuilder vs Append",
                                      "FastBuilder", "string::append",
                                      argv.loop, 10);
    COUT(ratio < 0.9, true);
}

