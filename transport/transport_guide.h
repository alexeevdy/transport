#pragma once

#include "transport_render.h"
#include "transport_router.h"

#include <utility>
#include <memory>
#include <optional>
#include <unordered_set>

class TransportGuide {
public:
    explicit TransportGuide(Descriptions::Data data, Transport::RoutingSettings routing_settings,
                            Render::SettingsPtr render_settings);

    std::optional<Response::Stop> GetStop(const std::string &name) const;

    std::optional<Response::Bus> GetBus(const std::string &name) const;

    std::optional<Response::Route> GetRoute(const std::string &from, const std::string &to) const;

    Response::Map GetMap() const;

private:
    double CalculateDirectLength(const Descriptions::DictStop &stop_descriptions,
                                 const std::vector<std::string> &route_stops);

    int64_t CalculateRouteLength(const std::vector<std::string> &route_stops) const;

private:
    Data::DataPtr database_;
    std::unordered_map<std::string, Response::Stop> stop_responses_;
    std::unordered_map<std::string, Response::Bus> bus_responses_;
    std::unique_ptr<Transport::TransportRouter> router_;
    std::unique_ptr<Render::Renderer> renderer_;
};
