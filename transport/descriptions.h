#pragma once

#include "coordinates.h"
#include "json.h"

#include <variant>
#include <vector>
#include <iostream>
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <utility>

namespace Descriptions {

    struct Stop {
        std::string name;
        Coordinates::Point coordinates;
        std::vector<std::pair<std::string, int>> distance_to_stops;

        static Stop ParseFrom(const Json::Node &);
    };

    struct Bus {
        std::string name;
        std::vector<std::string> stops;
        bool is_roundtrip;

        static Bus ParseFrom(const Json::Node &);
    };

    using Data = std::vector<std::variant<Stop, Bus>>;

    Data ReadFrom(std::istream &in);

    using DictStop = std::map<std::string, Stop>;
    using DictBus = std::map<std::string, Bus>;

    struct VectorEntry {
        std::string_view stop;
        std::string_view bus;
        bool is_key_next_on_route;
    };

//    struct MyHash {
//        size_t operator ()(VectorEntry obj) const {
//            std::hash<std::string_view> hash_sv; // todo: outside
//            return hash_sv(obj.stop) + hash_sv(obj.bus) + obj.is_key_next_on_route;
//        }
//    };

    using AdjacentStops = std::unordered_map<std::string_view, std::vector<VectorEntry>>;

    Data ReadJson(const Json::Node &);

}
