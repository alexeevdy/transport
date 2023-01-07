#include "descriptions.h"

namespace Descriptions {

    Stop Stop::ParseFrom(const Json::Node &base_node) {
        const auto &base_map = base_node.AsMap();
        auto name = base_map.at("name").AsString();
        auto coordinates = Coordinates::Point{
                .latitude = base_map.at("latitude").AsDouble(),
                .longitude = base_map.at("longitude").AsDouble()
        };
        std::vector<std::pair<std::string, int>> distance_to_stops;
        const auto &distances_map = base_map.at("road_distances").AsMap();
        for (const auto &[stop_name, node]: distances_map) {
            distance_to_stops.emplace_back(stop_name, static_cast<int>(node.AsDouble()));
        }
        return {
                .name = std::move(name),
                .coordinates = coordinates,
                .distance_to_stops = std::move(distance_to_stops)
        };
    }

    Bus Bus::ParseFrom(const Json::Node &base_node) {
        const auto &base_map = base_node.AsMap();
        auto number = base_map.at("name").AsString();
        std::vector<std::string> stops;

        const auto &stops_array = base_map.at("stops").AsArray();
        for (const auto &node: stops_array) {
            stops.emplace_back(node.AsString());
        }

        if (!base_map.at("is_roundtrip").AsBool()) {
            int n_stops = stops.size();
            stops.resize(2 * n_stops - 1);
            auto lhs = stops.begin() + n_stops - 2;
            auto rhs = stops.begin() + n_stops;
            for (; rhs != stops.end(); ++rhs, --lhs) {
                *rhs = *lhs;
            }
        }

        return Bus{
                .name = std::move(number),
                .stops = std::move(stops),
        };
    }

    Data ReadJson(const Json::Node &base_node) {
        Data result;
        for (const auto &node: base_node.AsArray()) {
            const auto &map = node.AsMap();
            if (const auto &type = map.at("type").AsString(); type == "Stop") {
                result.emplace_back(Stop::ParseFrom(node));
            } else if (type == "Bus") {
                result.emplace_back(Bus::ParseFrom(node));
            } else {
                throw std::runtime_error("Unknown description type: " + std::string(type));
            }
        }
        return result;
    }

}
