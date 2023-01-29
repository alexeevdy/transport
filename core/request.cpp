#include "request.h"

namespace Requests {

    Json::Node ProcessStop(const TransportGuide &tg, const Json::Node &base_node) {
        std::map<std::string, Json::Node> response_map;
        if (auto response = tg.GetStop(base_node.AsMap().at("name").AsString())) {
            std::vector<Json::Node> buses;
            for (const auto &bus_name: response->busses) {
                buses.emplace_back(bus_name);
            }
            response_map.emplace("buses", std::move(buses));
        } else {
            response_map["error_message"] = "not found";
        }
        response_map["request_id"] = Json::Int(
                static_cast<int64_t>(base_node.AsMap().at("id").AsDouble()));
        return Json::Node{std::move(response_map)};
    }

    Json::Node ProcessBus(const TransportGuide &tg, const Json::Node &base_node) {
        std::map<std::string, Json::Node> response_map;
        if (auto response = tg.GetBus(base_node.AsMap().at("name").AsString())) {
            response_map["stop_count"] = Json::Int(
                    static_cast<int64_t>(response->stops_on_route));
            response_map["unique_stop_count"] = Json::Int(
                    static_cast<int64_t>(response->unique_stops));
            response_map["route_length"] = Json::Int(response->route_length);
            response_map["curvature"] = response->curvature;
        } else {
            response_map["error_message"] = "not found";
        }
        response_map["request_id"] = Json::Int(
                static_cast<int64_t>(base_node.AsMap().at("id").AsDouble()));
        return Json::Node{std::move(response_map)};
    }

    Json::Node ProcessRoute(const TransportGuide &tg, const Json::Node &base_node) {
        std::map<std::string, Json::Node> response_map;
        if (auto response = tg.GetRoute(base_node.AsMap().at("from").AsString(),
                                        base_node.AsMap().at("to").AsString())) {
            std::vector<Json::Node> items_array;
            response_map["total_time"] = response->total_time;
            for (const auto &item: response->items) {
                std::map<std::string, Json::Node> item_map;
                if (std::holds_alternative<Response::Route::Wait>(item)) {
                    auto wait_route_element = std::get<Response::Route::Wait>(item);
                    item_map["type"] = "Wait";
                    item_map["stop_name"] = wait_route_element.stop_name;
                    item_map["time"] = Json::Int(wait_route_element.time);
                } else if (std::holds_alternative<Response::Route::Bus>(item)) {
                    auto bus_route_element = std::get<Response::Route::Bus>(item);
                    item_map["type"] = "Bus";
                    item_map["bus"] = bus_route_element.bus;
                    item_map["span_count"] = Json::Int(bus_route_element.span_count);
                    item_map["time"] = bus_route_element.time;
                } else {
                    throw std::runtime_error("Unknown route response");
                }
                items_array.emplace_back(Json::Node{std::move(item_map)});
            }
            response_map["items"] = std::move(items_array);
        } else {
            response_map["error_message"] = "not found";
        }
        response_map["request_id"] = Json::Int(
                static_cast<int64_t>(base_node.AsMap().at("id").AsDouble()));
        return Json::Node{std::move(response_map)};
    }


    Json::Node ProcessMap(const TransportGuide &tg, const Json::Node &base_node) {
        std::map<std::string, Json::Node> response_map;
        response_map["map"] = tg.GetMap().data;
        response_map["request_id"] = Json::Int(
                static_cast<int64_t>(base_node.AsMap().at("id").AsDouble()));
        return Json::Node{std::move(response_map)};
    }

    Json::Node ProcessAll(const TransportGuide &tg, const Json::Node &base_node) {
        std::vector<Json::Node> responses;
        for (const auto &node: base_node.AsArray()) {
            if (const auto &type = node.AsMap().at("type").AsString(); type == "Stop") {
                responses.emplace_back(ProcessStop(tg, node));
            } else if (type == "Bus") {
                responses.emplace_back(ProcessBus(tg, node));
            } else if (type == "Route") {
                responses.emplace_back(ProcessRoute(tg, node));
            } else if (type == "Map") {
                responses.emplace_back(ProcessMap(tg, node));
            } else {
                throw std::runtime_error("Unknown request type: " + std::string(type));
            }
        }
        return Json::Node{std::move(responses)};
    }

}
