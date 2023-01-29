#pragma once

#include "coordinates.h"
#include "json.h"

#include <variant>
#include <vector>
#include <iostream>
#include <map>
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

    Data ReadJson(const Json::Node &);

}
