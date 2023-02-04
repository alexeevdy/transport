#pragma once

#include "descriptions.h"
#include "database.h"
#include "response.h"
#include "json.h"
#include "svg.h"

#include <utility>
#include <unordered_map>

namespace Render {

    struct Settings {
        double width;
        double height;
        double padding;
        double stop_radius;
        double line_width;
        uint32_t stop_label_font_size;
        Svg::Point stop_label_offset;
        Svg::Color underlayer_color;
        double underlayer_width;
        std::vector<Svg::Color> color_palette;
        uint32_t bus_label_font_size;
        Svg::Point bus_label_offset;
        std::vector<std::string> layers;
        double outer_margin;
    };

    using SettingsPtr = std::shared_ptr<Settings>;

    struct RenderData {
        Data::DataPtr database;
        SettingsPtr render_settings;
    };

    inline bool IsFinishStop(Data::DataPtr database, std::string_view stop_name, std::string_view bus_name) {
        const auto &bus = database->bus_descriptions.at(std::string(bus_name));
        return stop_name == bus.stops.front()
            || (!bus.is_roundtrip
                && stop_name == bus.stops[bus.stops.size() / 2]);
    }

    class RenderDataOptimizer {
    public:
        explicit RenderDataOptimizer(RenderData render_data)
                : render_data_(RenderData{
                .database = std::make_shared<Data::Database>(*render_data.database),
                .render_settings = render_data.render_settings
        }) {}

        RenderData Optimize();

    private:
        RenderData render_data_;

        bool IsMainStop(std::string_view stop_name, std::string_view bus_name);

        void UniformStops();

        void CompressCoordinates();

        std::vector<int> EnumerateStops(
                const std::vector<std::vector<Descriptions::Stop *>> &grouped, size_t size) const;

    };

    SettingsPtr ReadJson(const Json::Node &);

    struct RouteByStops {
        std::string stop_name;
        std::string bus_name;
        bool is_interchange;

        RouteByStops(std::string stop, std::string bus, bool flag)
                : stop_name(std::move(stop)), bus_name(std::move(bus)), is_interchange(flag) {}
    };

    class Renderer {
    public:
        Renderer() = default;

        explicit Renderer(RenderData render_data);

        Response::Map RenderMap();

        Response::Map RenderRoute(const Response::Route::RouteItems &items) const;

    private:
        RenderData render_data_;
        Svg::Document svg_;

    private:

        static Svg::Point GetPosition(Coordinates::Point);

        void RenderStopPoints();

        static void RenderSingleStopLabel(
                Svg::Document *svg, SettingsPtr render_settings,
                const std::string &stop_name, Svg::Point position);

        void RenderStopLabels();

        void RenderBusLines();

        static void RenderSingleBusLabel(
                Svg::Document *svg, SettingsPtr render_settings,
                const std::string &bus_name, Svg::Point position, Svg::Color color);

        void RenderBusLabels();

        static const std::unordered_map<std::string, void (Renderer::*)()> kCallLayer;

    private:
        class RouteHelper {
        public:
            RouteHelper(RenderData render_data,
                        const Svg::Document &base_svg,
                        const std::vector<RouteByStops> &route_scheme);

            const Svg::Document &GetSvg() const {
                return route_svg_;
            }

        private:
            RenderData render_data_;
            Svg::Document route_svg_;
            const std::vector<RouteByStops> &route_scheme_;

            void RenderRouteStopPoints();

            void RenderRouteStopLabels();

            void RenderRouteBusLines();

            void RenderRouteBusLabels();

            static const std::unordered_map<std::string,
                    void (Renderer::RouteHelper::*)()> kCallRouteLayer;

        };


    };

}