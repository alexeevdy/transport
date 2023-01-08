#include "descriptions.h"
#include "router.h"

#include <set>
#include <optional>
#include <memory>

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
}

namespace Transport {

    class NameId {
    public:
        size_t SetId(const std::string &name) {
            if (auto it = ids_.find(name); it != ids_.end()) {
                return it->second;
            }
            size_t id = names_.size();
            ids_.emplace(name, id);
            names_.push_back(name);
            return id;
        }

        size_t GetId(const std::string &name) const {
            return ids_.at(name);
        }

        std::string GetName(size_t id) const {
            if (id < names_.size()) {
                return names_.at(id);
            }
            throw std::runtime_error("id out of bounds: " + std::to_string(id));
        }

    private:
        std::unordered_map<std::string, size_t> ids_;
        std::vector<std::string> names_;
    };

    struct RoutingSettings {
        int64_t bus_wait_time;
        double bus_velocity;
    };

    class TransportRouter {
    public:
        TransportRouter(const Descriptions::DictStop &,
                        const Descriptions::DictBus &);

        void UpdateDistance(const std::string &from, const std::string &to, int distance);

        int GetDistance(const std::string &from, const std::string &to) const;

        void BuildMap(const Descriptions::DictStop &,
                      const Descriptions::DictBus &,
                      RoutingSettings settings);

        std::optional<Response::Route> GetRoute(const std::string &from, const std::string &to) const;

    private:
        void ResizeToFit(size_t column, size_t row);

        struct EdgeInfo {
            size_t bus_id;
            size_t span_count;
        };

    private:
        NameId stops_;
        NameId buses_;
        RoutingSettings settings_;
        std::vector<std::vector<std::optional<int>>> distance_table_;
        Graph::DirectedWeightedGraph<double> graph_;
        std::unique_ptr<Graph::Router<double>> graph_router_;
        std::vector<EdgeInfo> edge_info_;
    };

    RoutingSettings ReadFrom(const Json::Node &);

}