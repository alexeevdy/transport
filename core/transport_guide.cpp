#include "transport_guide.h"


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

TransportGuide::TransportGuide(Descriptions::Data data) {
    Descriptions::DictStop stop_descriptions;
    Descriptions::DictBus bus_descriptions;

    for (auto &description: data) {
        if (std::holds_alternative<Descriptions::Stop>(description)) {
            auto &stop = std::get<Descriptions::Stop>(description);
            stop_descriptions.emplace(stop.name, std::move(stop));
        } else if (std::holds_alternative<Descriptions::Bus>(description)) {
            auto &bus = std::get<Descriptions::Bus>(description);
            bus_descriptions.emplace(bus.name, std::move(bus));
        } else {
            throw std::runtime_error("Unknown description variant");
        }
    }

    router_ = std::make_unique<TransportRouter>();

    for (const auto &[name, stop]: stop_descriptions) {
        stops_.emplace(name, Response::Stop{});
        for (const auto &[name_to, distance]: stop.distance_to_stops) {
            router_->UpdateDistance(name, name_to, distance);
        }
    }

    for (const auto &[name, bus]: bus_descriptions) {
        std::unordered_set<std::string> unique_stops;
        for (const auto &stop: bus.stops) {
            unique_stops.insert(stop);
            stops_[stop].busses.insert(name);
        }
        int64_t route_length = CalculateRouteLength(bus.stops);
        double direct_length = CalculateDirectLength(stop_descriptions, bus.stops);
        buses_.emplace(
                name,
                Response::Bus{
                        .stops_on_route = bus.stops.size(),
                        .unique_stops = unique_stops.size(),
                        .route_length = route_length,
                        .curvature = static_cast<double>(route_length) / direct_length
                }
        );
    }

}

std::optional<Response::Stop> TransportGuide::GetStop(const std::string &name) const {
    if (auto response = stops_.find(name); response != stops_.end()) {
        return response->second;
    }
    return std::nullopt;
}

std::optional<Response::Bus> TransportGuide::GetBus(const std::string &name) const {
    if (auto response = buses_.find(name); response != buses_.end()) {
        return response->second;
    }
    return std::nullopt;
}
