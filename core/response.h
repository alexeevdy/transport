#pragma once

#include <set>
#include <string>
#include <vector>

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

    struct Route {

        struct Wait {
            std::string stop_name;
            int64_t time;
        };

        struct Bus {
            std::string bus;
            int64_t span_count;
            double time;
        };

        double total_time;
        std::vector<std::variant<Wait, Bus>> items;

    };

    struct Map {
        std::string data;
    };
}