#pragma once

#include "transport_router.h"
#include "descriptions.h"

#include <set>
#include <utility>
#include <memory>
#include <optional>
#include <unordered_set>

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

}

class TransportGuide {
public:
    explicit TransportGuide(Descriptions::Data data);

    std::optional<Response::Stop> GetStop(const std::string &name) const;

    std::optional<Response::Bus> GetBus(const std::string &name) const;

private:
    double CalculateDirectLength(const Descriptions::DictStop &stop_descriptions,
                                 const std::vector<std::string> &route_stops);

    int64_t CalculateRouteLength(const std::vector<std::string> &route_stops) const;

private:
    std::unordered_map<std::string, Response::Stop> stops_;
    std::unordered_map<std::string, Response::Bus> buses_;
    std::unique_ptr<TransportRouter> router_;
};
