#pragma once

#include <set>
#include <string>
#include <vector>
#include <optional>

namespace Response {

    struct Stop {
        std::set<std::string> busses;
    };

    struct Bus {
        size_t stops_on_route;
        size_t unique_stops;
        int64_t route_length;
        double curvature;
    };

    struct Map {
        std::string data;
    };

    struct Route {

        struct Wait {
            std::string stop_name;
            int64_t time;
        };

        struct Bus {
            std::string bus;
            int64_t span_count;
            double time;

            using StopIt = std::vector<std::string>::const_iterator;
            StopIt from, to;
        };

        using RouteItems = std::vector<std::variant<Wait, Bus>>;
        RouteItems items;
        double total_time;
        Map map;
    };
}