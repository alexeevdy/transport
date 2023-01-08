#include "transport_router.h"

namespace Transport {

    void TransportRouter::ResizeToFit(size_t column, size_t row) {
        if (column >= distance_table_.size()) {
            distance_table_.resize(column + 1);
        }
        if (row >= distance_table_[column].size()) {
            distance_table_[column].resize(row + 1);
        }
    }

    void TransportRouter::UpdateDistance(const std::string &from, const std::string &to, int distance) {
        size_t from_id = stops_.GetId(from);
        size_t to_id = stops_.GetId(to);
        ResizeToFit(from_id, to_id);
        distance_table_[from_id][to_id] = distance;
        ResizeToFit(to_id, from_id);
        if (!distance_table_[to_id][from_id]) {
            distance_table_[to_id][from_id] = distance;
        }
    }

    int TransportRouter::GetDistance(const std::string &from, const std::string &to) const {
        size_t from_id = stops_.GetId(from);
        size_t to_id = stops_.GetId(to);
        return distance_table_[from_id][to_id].value();
    }

    RoutingSettings ReadFrom(const Json::Node &base_node) {
        return RoutingSettings{
                .bus_wait_time = static_cast<int64_t>(
                        base_node.AsMap().at("bus_wait_time").AsDouble()),
                .bus_velocity = base_node.AsMap().at("bus_velocity").AsDouble()
        };
    }

    void TransportRouter::BuildMap(const Descriptions::DictStop &stop_descriptions,
                                   const Descriptions::DictBus &bus_descriptions,
                                   RoutingSettings settings) {
        settings_ = settings;
        graph_ = Graph::DirectedWeightedGraph<double>(stop_descriptions.size());
        double bus_velocity_mpm = settings.bus_velocity * 100 / 6;
        for (const auto &[bus_name, bus]: bus_descriptions) {
            size_t bus_id = buses_.GetId(bus_name);
            for (auto from = bus.stops.begin(); from != bus.stops.end(); ++from) {
                double wait_time = settings.bus_wait_time;
                size_t span_count = 1;
                for (auto to = next(from), prev = from; to != bus.stops.end(); ++to, ++prev, ++span_count) {
                    wait_time += GetDistance(*prev, *to) / bus_velocity_mpm;
                    graph_.AddEdge(Graph::Edge<double>{
                            .from = stops_.GetId(*from),
                            .to = stops_.GetId(*to),
                            .weight = wait_time,
                    });
                    edge_info_.emplace_back(EdgeInfo{
                            .bus_id = bus_id,
                            .span_count = span_count,
                    });
                }
            }
        }
        graph_router_ = std::make_unique<Graph::Router<double>>(graph_);
    }

    std::optional<Response::Route> TransportRouter::GetRoute(const std::string &from, const std::string &to) const {
        if (auto route_info = graph_router_->BuildRoute(stops_.GetId(from), stops_.GetId(to))) {
            std::vector<std::variant<Response::Route::Wait, Response::Route::Bus>> route_items;

            for (size_t route_edge_idx = 0; route_edge_idx < route_info->edge_count; ++route_edge_idx) {
                auto graph_edge_idx = graph_router_->GetRouteEdge(route_info->id, route_edge_idx);
                auto graph_edge = graph_.GetEdge(graph_edge_idx);
                route_items.emplace_back(Response::Route::Wait{
                        .stop_name = stops_.GetName(graph_edge.from),
                        .time = settings_.bus_wait_time
                });
                route_items.emplace_back(Response::Route::Bus{
                        .bus = buses_.GetName(edge_info_[graph_edge_idx].bus_id),
                        .span_count = static_cast<int64_t>(
                                edge_info_[graph_edge_idx].span_count),
                        .time = graph_edge.weight - static_cast<double>(settings_.bus_wait_time)
                });
            }

            return Response::Route{
                    .total_time = route_info->weight,
                    .items = std::move(route_items)
            };
        }
        return std::nullopt;
    }

    TransportRouter::TransportRouter(const Descriptions::DictStop &stop_descriptions,
                                     const Descriptions::DictBus &bus_descriptions) {
        for (const auto &[stop_name, stop]: stop_descriptions) {
            stops_.SetId(stop_name);
        }
        for (const auto &[bus_name, bus]: bus_descriptions) {
            buses_.SetId(bus_name);
        }
    }

}