#pragma once

#include "json.h"
#include "svg.h"
#include "response.h"
#include "descriptions.h"

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
    };

    using SettingsPtr = std::shared_ptr<Settings>;

    SettingsPtr ReadJson(const Json::Node &);

    struct RenderFactors {
        double min_lat;
        double min_lon;
        double max_lat;
        double max_lon;
        double zoom_coef;
    };

    RenderFactors GetRenderFactors(const Descriptions::DictStop &stop_descriptions,
                                   SettingsPtr render_settings);

    struct RenderData {
        const Descriptions::DictStop &stop_descriptions;
        const Descriptions::DictBus &bus_descriptions;
        SettingsPtr render_settings;
    };

    class RendererHelper {
    public:
        RendererHelper(RenderData render_data,
                       Svg::DocumentPtr svg)
                : render_data_(render_data),
                  render_factors_(GetRenderFactors(
                          render_data_.stop_descriptions, render_data_.render_settings)),
                  svg_(svg) {}

        void RenderMap();

    private:
        RenderData render_data_;
        RenderFactors render_factors_;
        Svg::DocumentPtr svg_;

        Svg::Point GetPosition(Coordinates::Point) const;

        void RenderStopPoints();

        void RenderStopLabels();

        void RenderBusLines();

        void RenderSingleBusLabel(const std::string& bus_name, Svg::Point position, Svg::Color color);

        void RenderBusLabels();

        static const std::unordered_map<std::string, void (RendererHelper::*)()> kCallLayer;
    };

    class Renderer {
    public:
        explicit Renderer(SettingsPtr settings) :
                render_settings_(std::move(settings)) {}

        Response::Map RenderMap(const Descriptions::DictStop &,
                                const Descriptions::DictBus &);

    private:
        SettingsPtr render_settings_;
    };

}