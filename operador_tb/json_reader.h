#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <cstdint>
#include <cmath>
#include <cstdlib>

struct JsonValue {
    enum Type { NULL_VAL, BOOL, NUMBER, STRING, ARRAY, OBJECT };
    Type type = NULL_VAL;
    bool bool_val = false;
    double num_val = 0.0;
    std::string str_val;
    std::vector<JsonValue> arr_val;
    std::vector<std::pair<std::string, JsonValue>> obj_val;

    const JsonValue* get(const std::string& key) const {
        if (type != OBJECT) return nullptr;
        for (auto& [k, v] : obj_val)
            if (k == key) return &v;
        return nullptr;
    }

    int asInt() const { return (int)num_val; }
    float asFloat() const { return (float)num_val; }
    bool asBool() const { return bool_val; }
    const std::string& asString() const { return str_val; }

    int arrSize() const { return (int)arr_val.size(); }
    const JsonValue& operator[](int i) const { return arr_val[i]; }
};

class JsonReader {
public:
    JsonReader(const std::string& input) : s(input), pos(0) {}
    JsonValue parse();

private:
    const std::string& s;
    size_t pos;

    void skipWS();
    char peek();
    char next();
    JsonValue parseValue();
    JsonValue parseString();
    JsonValue parseNumber();
    JsonValue parseArray();
    JsonValue parseObject();
};
