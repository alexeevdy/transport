#include "request.h"

namespace Requests {

    Json::Node ProcessStop(const TransportGuide &tg, const Json::Node &base_node) {
        std::map<std::string, Json::Node> map;
        if (auto response = tg.GetStop(base_node.AsMap().at("name").AsString())) {
            std::vector<Json::Node> buses;
            for (const auto &bus_name: response->busses) {
                buses.emplace_back(bus_name);
            }
            map.emplace("buses", std::move(buses));
        } else {
            map["error_message"] = "not found";
        }
        map["request_id"] = Json::Int(
                static_cast<int64_t>(base_node.AsMap().at("id").AsDouble()));
        return Json::Node{std::move(map)};
    }

    Json::Node ProcessBus(const TransportGuide &tg, const Json::Node &base_node) {
        std::map<std::string, Json::Node> map;
        if (auto response = tg.GetBus(base_node.AsMap().at("name").AsString())) {
            map["stop_count"] = Json::Int(
                    static_cast<int>(response->stops_on_route));
            map["unique_stop_count"] = Json::Int(
                    static_cast<int>(response->unique_stops));
            map["route_length"] = Json::Int(response->route_length);
            map["curvature"] = response->curvature;
        } else {
            map["error_message"] = "not found";
        }
        map["request_id"] = Json::Int(
                static_cast<int64_t>(base_node.AsMap().at("id").AsDouble()));
        return Json::Node{std::move(map)};
    }

    Json::Node ProcessAll(const TransportGuide &tg, const Json::Node &base_node) {
        std::vector<Json::Node> responses;
        for (const auto &node: base_node.AsArray()) {
            if (const auto &type = node.AsMap().at("type").AsString(); type == "Stop") {
                responses.emplace_back(ProcessStop(tg, node));
            } else if (type == "Bus") {
                responses.emplace_back(ProcessBus(tg, node));
            } else {
                throw std::runtime_error("Unknown request type: " + std::string(type));
            }
        }
        return Json::Node{std::move(responses)};
    }

}
