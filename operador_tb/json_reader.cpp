#include "json_reader.h"

void JsonReader::skipWS() {
    while (pos < s.size() && (s[pos] == ' ' || s[pos] == '\t' || s[pos] == '\n' || s[pos] == '\r'))
        pos++;
}

char JsonReader::peek() {
    return (pos < s.size()) ? s[pos] : '\0';
}

char JsonReader::next() {
    return (pos < s.size()) ? s[pos++] : '\0';
}

JsonValue JsonReader::parse() {
    skipWS();
    return parseValue();
}

JsonValue JsonReader::parseValue() {
    skipWS();
    char c = peek();
    switch (c) {
        case '"': return parseString();
        case '{': return parseObject();
        case '[': return parseArray();
        case '-': case '0': case '1': case '2': case '3':
        case '4': case '5': case '6': case '7': case '8': case '9':
            return parseNumber();
        case 't': {
            std::string lit = "true";
            for (char lc : lit) if (next() != lc) return {};
            JsonValue v; v.type = JsonValue::BOOL; v.bool_val = true; return v;
        }
        case 'f': {
            std::string lit = "false";
            for (char lc : lit) if (next() != lc) return {};
            JsonValue v; v.type = JsonValue::BOOL; v.bool_val = false; return v;
        }
        case 'n': {
            std::string lit = "null";
            for (char lc : lit) if (next() != lc) return {};
            return {};
        }
        default: return {};
    }
}

JsonValue JsonReader::parseString() {
    JsonValue v;
    v.type = JsonValue::STRING;
    next(); // opening "
    while (pos < s.size()) {
        char c = next();
        if (c == '"') break;
        if (c == '\\') {
            char esc = next();
            switch (esc) {
                case '"': v.str_val += '"'; break;
                case '\\': v.str_val += '\\'; break;
                case '/': v.str_val += '/'; break;
                case 'b': v.str_val += '\b'; break;
                case 'f': v.str_val += '\f'; break;
                case 'n': v.str_val += '\n'; break;
                case 'r': v.str_val += '\r'; break;
                case 't': v.str_val += '\t'; break;
                case 'u': {
                    // Simple: just pass through as hex digits for now
                    char hex[5] = {};
                    for (int i = 0; i < 4; i++) hex[i] = next();
                    v.str_val += "\\u";
                    v.str_val += hex;
                    break;
                }
                default: v.str_val += esc;
            }
        } else {
            v.str_val += c;
        }
    }
    return v;
}

JsonValue JsonReader::parseNumber() {
    JsonValue v;
    v.type = JsonValue::NUMBER;
    size_t start = pos;
    if (pos < s.size() && s[pos] == '-') pos++;
    while (pos < s.size() && (s[pos] >= '0' && s[pos] <= '9')) pos++;
    if (pos < s.size() && s[pos] == '.') {
        pos++;
        while (pos < s.size() && (s[pos] >= '0' && s[pos] <= '9')) pos++;
    }
    if (pos < s.size() && (s[pos] == 'e' || s[pos] == 'E')) {
        pos++;
        if (pos < s.size() && (s[pos] == '+' || s[pos] == '-')) pos++;
        while (pos < s.size() && (s[pos] >= '0' && s[pos] <= '9')) pos++;
    }
    v.num_val = std::atof(s.substr(start, pos - start).c_str());
    return v;
}

JsonValue JsonReader::parseArray() {
    JsonValue v;
    v.type = JsonValue::ARRAY;
    next(); // [
    skipWS();
    if (peek() == ']') { next(); return v; }
    while (true) {
        v.arr_val.push_back(parseValue());
        skipWS();
        char c = next();
        if (c == ']') break;
        // c should be ',' — if not, treat as separator anyway
        skipWS();
        if (peek() == ']') { next(); break; }
    }
    return v;
}

JsonValue JsonReader::parseObject() {
    JsonValue v;
    v.type = JsonValue::OBJECT;
    next(); // {
    skipWS();
    if (peek() == '}') { next(); return v; }
    while (true) {
        skipWS();
        auto key = parseString();
        skipWS();
        next(); // :
        skipWS();
        v.obj_val.push_back({key.str_val, parseValue()});
        skipWS();
        char c = next();
        if (c == '}') break;
        // c should be ','
        skipWS();
        if (peek() == '}') { next(); break; }
    }
    return v;
}
