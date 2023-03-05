#include "transport_guide.h"
#include "utils.h"

double TransportGuide::CalculateDirectLength(const Descriptions::DictStop &stop_descriptions,
                                             const std::vector<std::string> &route_stops) {
    if (route_stops.empty()) {
        throw std::runtime_error("Empty route");
    }

    double direct_length = 0;
    Coordinates::Point from, to;

    bool first = true;
    for (const auto &stop: route_stops) {
        if (first) {
            from = stop_descriptions.at(stop).coordinates;
            first = false;
            continue;
        }
        to = stop_descriptions.at(stop).coordinates;
        direct_length += Coordinates::DistanceBetween(from, to);
        from = to;
    }

    return direct_length;
}

int64_t TransportGuide::CalculateRouteLength(const std::vector<std::string> &route_stops) const {
    if (route_stops.empty()) {
        throw std::runtime_error("Empty route");
    }

    int64_t route_length = 0;
    const std::string *from, *to;

    bool first = true;
    for (const auto &stop: route_stops) {
        if (first) {
            from = &stop;
            first = false;
            continue;
        }
        to = &stop;
        route_length += router_->GetDistance(*from, *to);
        from = to;
    }

    return route_length;
}

// todo: split this function
TransportGuide::TransportGuide(Descriptions::Data data, Transport::RoutingSettings routing_settings,
                               Render::SettingsPtr render_settings) {
    database_ = std::make_shared<Data::Database>();

    for (auto &description: data) {
        if (std::holds_alternative<Descriptions::Stop>(description)) {
            auto &stop = std::get<Descriptions::Stop>(description);
            database_->stop_descriptions.emplace(stop.name, std::move(stop));
        } else if (std::holds_alternative<Descriptions::Bus>(description)) {
            auto &bus = std::get<Descriptions::Bus>(description);
            database_->bus_descriptions.emplace(bus.name, std::move(bus));
        } else {
            throw std::runtime_error("Unknown description variant");
        }
    }

    router_ = std::make_unique<Transport::TransportRouter>(database_);

    for (const auto &[name, stop]: database_->stop_descriptions) {
        stop_responses_.emplace(name, Response::Stop{});
        for (const auto &[name_to, distance]: stop.distance_to_stops) {
            router_->UpdateDistance(name, name_to, distance);
        }
    }

    for (const auto &[bus_name, bus]: database_->bus_descriptions) {
        std::unordered_set<std::string> unique_stops;
        bool first = true;
        std::string_view prev_stop_name, cur_stop_name;
        for (const auto &stop_name: bus.stops) {
            unique_stops.insert(stop_name);
            stop_responses_[stop_name].busses.insert(bus_name);
            // fill database`s . adjacent stops
            if (first) {
                first = false;
                prev_stop_name = database_->stop_descriptions.find(stop_name)->second.name;
                continue;
            }
            cur_stop_name = database_->stop_descriptions.find(stop_name)->second.name;
            database_->adjacent_stops[cur_stop_name].push_back(
                    Descriptions::VectorEntry{prev_stop_name, bus_name, true});
            database_->adjacent_stops[prev_stop_name].push_back(
                    Descriptions::VectorEntry{cur_stop_name, bus_name, false});
            prev_stop_name = cur_stop_name;
        }
        int64_t route_length = CalculateRouteLength(bus.stops);
        double direct_length = CalculateDirectLength(database_->stop_descriptions, bus.stops);
        bus_responses_.emplace(
                bus_name,
                Response::Bus{
                        .stops_on_route = bus.stops.size(),
                        .unique_stops = unique_stops.size(),
                        .route_length = route_length,
                        .curvature = static_cast<double>(route_length) / direct_length
                }
        );
    }

    router_->BuildMap(database_, routing_settings);

    renderer_ = std::make_unique<Render::Renderer>(Render::RenderData{database_, render_settings});
}

std::optional<Response::Stop> TransportGuide::GetStop(const std::string &name) const {
    if (auto response = stop_responses_.find(name); response != stop_responses_.end()) {
        return response->second;
    }
    return std::nullopt;
}

std::optional<Response::Bus> TransportGuide::GetBus(const std::string &name) const {
    if (auto response = bus_responses_.find(name); response != bus_responses_.end()) {
        return response->second;
    }
    return std::nullopt;
}

std::optional<Response::Route> TransportGuide::GetRoute(const std::string &from, const std::string &to) const {
    if (auto response = router_->GetRoute(from, to)) {
        response->map = renderer_->RenderRoute(response->items);
        return response;
    }
    return std::nullopt;
}

Response::Map TransportGuide::GetMap() const {
    return renderer_->RenderMap();
}
