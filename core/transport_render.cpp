#include "transport_render.h"

namespace Render {

    Svg::Point ReadPoint(const Json::Node &point_node) {
        return Svg::Point{
                point_node.AsArray().at(0).AsDouble(),
                point_node.AsArray().at(1).AsDouble()
        };
    }

    Svg::Color ReadColor(const Json::Node &color_node) {
        if (std::holds_alternative<std::string>(color_node)) {
            return color_node.AsString();
        } else if (std::holds_alternative<std::vector<Json::Node>>(color_node)) {
            const auto &array = color_node.AsArray();
            Svg::Rgb rgb = Svg::Rgb(
                    static_cast<int>(array.at(0).AsDouble()),
                    static_cast<int>(array.at(1).AsDouble()),
                    static_cast<int>(array.at(2).AsDouble())
            );
            if (array.size() == 4) {
                return Svg::Rgba{rgb, array.at(3).AsDouble()};
            }
            return rgb;
        } else {
            throw std::runtime_error("unknown color variant");
        }
    }

    std::vector<Svg::Color> ReadPalette(const Json::Node &array_node) {
        std::vector<Svg::Color> result_palette;
        result_palette.reserve(array_node.AsArray().size());
        for (const auto &color_node: array_node.AsArray()) {
            result_palette.emplace_back(ReadColor(color_node));
        }
        return result_palette;
    }

    std::vector<std::string> ReadLayers(const Json::Node &array_node) {
        std::vector<std::string> result_layers;
        result_layers.reserve(array_node.AsArray().size());
        for (const auto &layer_node: array_node.AsArray()) {
            result_layers.emplace_back(layer_node.AsString());
        }
        return result_layers;
    }

    SettingsPtr ReadJson(const Json::Node &base_node) {
        const auto &base_map = base_node.AsMap();
        return std::make_shared<Settings>(Settings{
                .width = base_map.at("width").AsDouble(),
                .height = base_map.at("height").AsDouble(),
                .padding = base_map.at("padding").AsDouble(),
                .stop_radius = base_map.at("stop_radius").AsDouble(),
                .line_width = base_map.at("line_width").AsDouble(),
                .stop_label_font_size = static_cast<uint32_t>(
                        base_map.at("stop_label_font_size").AsDouble()),
                .stop_label_offset = ReadPoint(base_map.at("stop_label_offset")),
                .underlayer_color = ReadColor(base_map.at("underlayer_color")),
                .underlayer_width = base_map.at("underlayer_width").AsDouble(),
                .color_palette = ReadPalette(base_map.at("color_palette")),
                .bus_label_font_size = static_cast<uint32_t>(
                        base_map.at("bus_label_font_size").AsDouble()),
                .bus_label_offset = ReadPoint(base_map.at("bus_label_offset")),
                .layers = ReadLayers(base_map.at("layers"))
        });
    }

    RenderFactors GetRenderFactors(const Descriptions::DictStop &stop_descriptions,
                                   SettingsPtr render_settings) {
        const double INF = 1e9;
        double min_lat, min_lon, max_lat, max_lon, zoom_coef = INF;
        min_lat = min_lon = INF;
        max_lat = max_lon = -INF;
        for (const auto &[stop_name, stop]: stop_descriptions) {
            max_lat = std::max(max_lat, stop.coordinates.latitude);
            max_lon = std::max(max_lon, stop.coordinates.longitude);
            min_lat = std::min(min_lat, stop.coordinates.latitude);
            min_lon = std::min(min_lon, stop.coordinates.longitude);
        }
        if (std::abs(min_lon - max_lon) > 1e-6) {
            zoom_coef = (render_settings->width - 2 * render_settings->padding) / (max_lon - min_lon);
        }
        if (std::abs(min_lat - max_lat) > 1e-6) {
            zoom_coef = std::min(
                    zoom_coef,
                    (render_settings->height - 2 * render_settings->padding) / (max_lat - min_lat)
            );
        }
        if (zoom_coef + 1 > INF) {
            zoom_coef = 0;
        }
        return {min_lat, min_lon, max_lat, max_lon, zoom_coef};
    }

    Svg::Point RendererHelper::GetPosition(Coordinates::Point point) const {
        return {
                (point.longitude - render_factors_.min_lon) * render_factors_.zoom_coef
                + render_data_.render_settings->padding,
                (render_factors_.max_lat - point.latitude) * render_factors_.zoom_coef
                + render_data_.render_settings->padding
        };
    }

    void RendererHelper::RenderBusLines() {
        size_t bus_order_id = 0;
        for (const auto &[bus_name, bus]: render_data_.bus_descriptions) {
            Svg::Polyline polyline;
            polyline.SetStrokeColor(render_data_.render_settings->color_palette[bus_order_id])
                    .SetStrokeWidth(render_data_.render_settings->line_width)
                    .SetStrokeLineCap("round")
                    .SetStrokeLineJoin("round");
            for (const auto &stop_name: bus.stops) {
                polyline.AddPoint(GetPosition(render_data_.stop_descriptions.at(stop_name).coordinates));
            }
            (++bus_order_id) %= render_data_.render_settings->color_palette.size();
            svg_->Add(std::move(polyline));
        }
    }

    void RendererHelper::RenderStopPoints() {
        for (const auto &[stop_name, stop]: render_data_.stop_descriptions) {
            svg_->Add(Svg::Circle{}
                              .SetCenter(GetPosition(stop.coordinates))
                              .SetRadius(render_data_.render_settings->stop_radius)
                              .SetFillColor("white"));
        }
    }

    void RendererHelper::RenderStopLabels() {
        for (const auto &[stop_name, stop]: render_data_.stop_descriptions) {
            svg_->Add(Svg::Text{}
                              .SetPoint(GetPosition(stop.coordinates))
                              .SetOffset(render_data_.render_settings->stop_label_offset)
                              .SetFontSize(render_data_.render_settings->stop_label_font_size)
                              .SetFontFamily("Verdana")
                              .SetData(stop_name)
                              .SetFillColor(render_data_.render_settings->underlayer_color)
                              .SetStrokeColor(render_data_.render_settings->underlayer_color)
                              .SetStrokeWidth(render_data_.render_settings->underlayer_width)
                              .SetStrokeLineCap("round")
                              .SetStrokeLineJoin("round"));
            svg_->Add(Svg::Text{}
                              .SetPoint(GetPosition(stop.coordinates))
                              .SetOffset(render_data_.render_settings->stop_label_offset)
                              .SetFontSize(render_data_.render_settings->stop_label_font_size)
                              .SetFontFamily("Verdana")
                              .SetData(stop_name)
                              .SetFillColor("black"));
        }
    }

    void RendererHelper::RenderSingleBusLabel(
            const std::string &bus_name, Svg::Point position, Svg::Color color) {
        svg_->Add(Svg::Text{}
                          .SetPoint(position)
                          .SetOffset(render_data_.render_settings->bus_label_offset)
                          .SetFontSize(render_data_.render_settings->bus_label_font_size)
                          .SetFontFamily("Verdana")
                          .SetFontWeight("bold")
                          .SetData(bus_name)
                          .SetFillColor(render_data_.render_settings->underlayer_color)
                          .SetStrokeColor(render_data_.render_settings->underlayer_color)
                          .SetStrokeWidth(render_data_.render_settings->underlayer_width)
                          .SetStrokeLineCap("round")
                          .SetStrokeLineJoin("round"));
        svg_->Add(Svg::Text{}
                          .SetPoint(position)
                          .SetOffset(render_data_.render_settings->bus_label_offset)
                          .SetFontSize(render_data_.render_settings->bus_label_font_size)
                          .SetFontFamily("Verdana")
                          .SetFontWeight("bold")
                          .SetData(bus_name)
                          .SetFillColor(color));
    }

    void RendererHelper::RenderBusLabels() {
        size_t bus_order_id = 0;
        for (const auto &[bus_name, bus]: render_data_.bus_descriptions) {
            auto color = render_data_.render_settings->color_palette[bus_order_id];
            auto position = GetPosition(
                    render_data_.stop_descriptions.at(bus.stops.front()).coordinates);
            RenderSingleBusLabel(bus_name, position, color);
            if (!bus.is_roundtrip && bus.stops.front() != bus.stops[bus.stops.size() / 2]) {
                position = GetPosition(
                        render_data_.stop_descriptions.at(bus.stops[bus.stops.size() / 2]).coordinates);
                RenderSingleBusLabel(bus_name, position, color);
            }
            (++bus_order_id) %= render_data_.render_settings->color_palette.size();
        }
    }

    const std::unordered_map<std::string, void (RendererHelper::*)()> RendererHelper::kCallLayer = {
            {"bus_lines", &RendererHelper::RenderBusLines},
            {"bus_labels", &RendererHelper::RenderBusLabels},
            {"stop_points", &RendererHelper::RenderStopPoints},
            {"stop_labels", &RendererHelper::RenderStopLabels}
    };

    void RendererHelper::RenderMap() {
        for (const auto& layer_name: render_data_.render_settings->layers) {
            (this->*kCallLayer.at(layer_name))();
        }
    }

Response::Map Renderer::RenderMap(const Descriptions::DictStop &stop_descriptions,
                                      const Descriptions::DictBus &bus_descriptions) {
        Svg::DocumentPtr svg = std::make_shared<Svg::Document>();
        RendererHelper render_helper(
                RenderData{stop_descriptions, bus_descriptions, render_settings_}, svg);
        render_helper.RenderMap();
        std::ostringstream os;
        svg->Render(os);
        return {os.str()};
    }

}
