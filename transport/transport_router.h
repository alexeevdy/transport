#include "router.h"
#include "transport_render.h"


#include <set>
#include <optional>
#include <memory>

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
            return names_.at(id);
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
        explicit TransportRouter(Data::DataPtr);

        void UpdateDistance(const std::string &from, const std::string &to, int distance);

        int GetDistance(const std::string &from, const std::string &to) const;

        void BuildMap(Data::DataPtr,
                      RoutingSettings settings);

        std::optional<Response::Route> GetRoute(const std::string &from, const std::string &to) const;

    private:
        void ResizeToFit(size_t column, size_t row);

        struct EdgeInfo {
            size_t bus_id;
            size_t span_count;
            Response::Route::Bus::StopIt from, to;
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