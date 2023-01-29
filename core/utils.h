#pragma once

#pragma once

#include <string>
#include <optional>
#include <sstream>

inline std::string_view &StripWhitespaces(std::string_view &str) {
    while (!str.empty() && std::isspace(str.front())) {
        str.remove_prefix(1);
    }
    while (!str.empty() && std::isspace(str.back())) {
        str.remove_suffix(1);
    }
    return str;
}

inline std::pair<std::string_view, std::optional<std::string_view>> SplitTwoStrict(
        std::string_view s, std::string_view delimiter) {
    const size_t pos = s.find(delimiter);
    if (pos == s.npos) {
        return {s, std::nullopt};
    } else {
        return {s.substr(0, pos), s.substr(pos + delimiter.length())};
    }
}

inline std::pair<std::string_view, std::string_view> SplitTwo(std::string_view s,
                                                       std::string_view delimiter) {
    const auto [lhs, rhs_opt] = SplitTwoStrict(s, delimiter);
    return {lhs, rhs_opt.value_or("")};
}

inline std::string_view ReadToken(std::string_view &s, std::string_view delimiter) {
    const auto [lhs, rhs] = SplitTwo(StripWhitespaces(s), delimiter);
    s = rhs;
    return lhs;
}

inline double ConvertToDouble(std::string_view str) {
    size_t pos;
    const double result = stod(std::string(str), &pos);
    if (pos != str.length()) {
        std::stringstream error;
        error << "string " << str << " contains " << (str.length() - pos) << " trailing chars";
        throw std::invalid_argument(error.str());
    }
    return result;
}

inline void ReplaceAll(std::string& str, std::string_view from, std::string_view to) {
    std::string buffer;
    buffer.reserve(str.size());
    size_t prev_pos, pos = 0;
    for(;;) {
        prev_pos = pos;
        pos = str.find(from, pos);
        if (pos == str.npos) {
            break;
        }
        buffer.append(str, prev_pos, pos - prev_pos);
        buffer += to;
        pos += from.size();
    }
    buffer.append(str, prev_pos, pos - prev_pos);
    str.swap(buffer);
}