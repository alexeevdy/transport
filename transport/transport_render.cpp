#include "transport_render.h"
#include "utils.h"

#include <algorithm>
#include <stack>
#include <utility>

namespace Render {

    RenderData RenderDataOptimizer::Optimize() {
        UniformStops();
        CompressCoordinates();
        return render_data_;
    }

    bool RenderDataOptimizer::IsMainStop(std::string_view stop_name, std::string_view bus_name) {
        if (IsFinishStop(render_data_.database, stop_name, bus_name)) {
            return true;
        }
        int cur_bus_cnt = 0;
        int unique_bus_cnt = 1;
        for (auto &[adj_stop_name, adj_bus_name, is_cur_next_on_route]
                : render_data_.database->adjacent_stops[stop_name]) {
            if (!is_cur_next_on_route) {
                continue;
            }
            cur_bus_cnt += (bus_name == adj_bus_name);
            unique_bus_cnt += (bus_name != adj_bus_name);
            if (cur_bus_cnt == 3 || unique_bus_cnt == 2) {
                return true;
            }
        }
        return false;
    }

    void RenderDataOptimizer::UniformStops() {
        for (auto &[bus_name, bus]: render_data_.database->bus_descriptions) {
            std::string prev_main_stop;
            std::stack<std::string> secondary_stops;
            bool first = true;
            for (auto &stop_name: bus.stops) {
                if (first) {
                    first = false;
                    prev_main_stop = stop_name;
                    continue;
                }
                if (IsMainStop(stop_name, bus_name)) {
                    const int cnt = static_cast<int>(secondary_stops.size());
                    const auto from = render_data_.database->stop_descriptions[stop_name].coordinates;
                    const auto to = render_data_.database->stop_descriptions[prev_main_stop].coordinates;
                    const double lon_step = (to.longitude - from.longitude) / (cnt + 1);
                    const double lat_step = (to.latitude - from.latitude) / (cnt + 1);
                    int id = 0;
                    while (!secondary_stops.empty()) {
                        auto *secondary_stop =
                                &render_data_.database->stop_descriptions[secondary_stops.top()];
                        secondary_stops.pop();
                        secondary_stop->coordinates.longitude = from.longitude + lon_step * (id + 1);
                        secondary_stop->coordinates.latitude = from.latitude + lat_step * (id + 1);
                        ++id;
                    }
                    prev_main_stop = stop_name;
                } else {
                    secondary_stops.push(stop_name);
                }
            }
        }
    }

    void RenderDataOptimizer::CompressCoordinates() {
        std::vector<Descriptions::Stop *> stops;
        for (auto &[stop_name, stop]: render_data_.database->stop_descriptions) {
            stops.push_back(&stop);
        }
        {
            std::sort(stops.begin(), stops.end(),
                      [](Descriptions::Stop *lhs, Descriptions::Stop *rhs) {
                          return lhs->coordinates.longitude < rhs->coordinates.longitude;
                      });
            std::vector<std::vector<Descriptions::Stop *>> grouped;
            grouped.emplace_back();
            int top = 0;
            std::optional<double> prev_lon;
            for (auto &stop_ptr: stops) {
                if (prev_lon && stop_ptr->coordinates.longitude != *prev_lon) {
                    grouped.emplace_back();
                    ++top;
                }
                grouped[top].push_back(stop_ptr);
                prev_lon = stop_ptr->coordinates.longitude;
            }
            auto ids = EnumerateStops(grouped, stops.size());
            int unique_id_cnt = *std::max_element(ids.begin(), ids.end());
            double x_step = (unique_id_cnt > 0
                             ? (render_data_.render_settings->width
                                - 2 * render_data_.render_settings->padding) / unique_id_cnt
                             : 0
            );
            int id = 0;
            for (auto &stop: stops) {
                stop->coordinates.longitude = render_data_.render_settings->padding + x_step * ids[id];
                ++id;
            }
        }
        {
            std::sort(stops.begin(), stops.end(),
                      [](Descriptions::Stop *lhs, Descriptions::Stop *rhs) {
                          return lhs->coordinates.latitude < rhs->coordinates.latitude;
                      });
            std::vector<std::vector<Descriptions::Stop *>> grouped;
            grouped.emplace_back();
            int top = 0;
            std::optional<double> prev_lat;
            for (auto &stop_ptr: stops) {
                if (prev_lat && stop_ptr->coordinates.latitude != *prev_lat) {
                    grouped.emplace_back();
                    ++top;
                }
                grouped[top].push_back(stop_ptr);
                prev_lat = stop_ptr->coordinates.longitude;
            }
            auto ids = EnumerateStops(grouped, stops.size());
            int unique_id_cnt = *std::max_element(ids.begin(), ids.end());
            double y_step = (unique_id_cnt > 0
                             ? (render_data_.render_settings->height
                                - 2 * render_data_.render_settings->padding) / unique_id_cnt
                             : 0
            );
            int id = 0;
            for (auto &stop: stops) {
                stop->coordinates.latitude = render_data_.render_settings->height
                                             - render_data_.render_settings->padding - y_step * ids[id];
                ++id;
            }
        }
    }

    std::vector<int>
    RenderDataOptimizer::EnumerateStops(
            const std::vector<std::vector<Descriptions::Stop *>> &grouped, size_t size) const {
        std::vector<int> ids(size, -1);
        std::unordered_map<std::string_view, int> name_to_pos;
        int top = 0;
        for (auto &group: grouped) {
            for (auto &stop_ptr: group) {
                name_to_pos[stop_ptr->name] = top;
                ++top;
            }
        }
        top = 0;
        for (int i = 0; i < static_cast<int>(grouped[0].size()); ++i) {
            ids[top] = 0;
            ++top;
        }
        auto find_group_id = [&](const std::vector<Descriptions::Stop *> &group) {
            int max_id = -1;
            for (auto &stop_ptr: group) {
                for (auto &[adj_stop_name, adj_bus_name, is_cur_next_on_route]
                        : render_data_.database->adjacent_stops[stop_ptr->name]) {
                    max_id = std::max(max_id, ids[name_to_pos[adj_stop_name]]);
                }
            }
            return max_id + 1;
        };
        for (int i = 1; i < static_cast<int>(grouped.size()); ++i) {
            auto &group = grouped[i];
            int group_id = find_group_id(group);
            for (int j = 0; j < static_cast<int>(group.size()); ++j) {
                ids[top] = group_id;
                ++top;
            }
        }
        return ids;
    }

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
                .layers = ReadLayers(base_map.at("layers")),
                .outer_margin = base_map.at("outer_margin").AsDouble()
        });
    }

    Svg::Point Renderer::GetPosition(Coordinates::Point point) {
        return {
                point.longitude,
                point.latitude
        };
    }

    void Renderer::RenderBusLines() {
        size_t bus_order_id = 0;
        for (const auto &[bus_name, bus]: render_data_.database->bus_descriptions) {
            Svg::Polyline polyline;
            render_data_.database->bus_colors[bus_name] = render_data_.render_settings->color_palette[bus_order_id];
            polyline.SetStrokeColor(render_data_.database->bus_colors[bus_name])
                    .SetStrokeWidth(render_data_.render_settings->line_width)
                    .SetStrokeLineCap("round")
                    .SetStrokeLineJoin("round");
            for (const auto &stop_name: bus.stops) {
                polyline.AddPoint(
                        GetPosition(render_data_.database->stop_descriptions.at(stop_name).coordinates));
            }
            (++bus_order_id) %= render_data_.render_settings->color_palette.size();
            svg_.Add(std::move(polyline));
        }
    }

    void Renderer::RenderStopPoints() {
        for (const auto &[stop_name, stop]: render_data_.database->stop_descriptions) {
            svg_.Add(Svg::Circle{}
                             .SetCenter(GetPosition(stop.coordinates))
                             .SetRadius(render_data_.render_settings->stop_radius)
                             .SetFillColor("white"));
        }
    }

    void Renderer::RenderSingleStopLabel(
            Svg::Document *svg, SettingsPtr render_settings,
            const std::string &stop_name, Svg::Point position) {
        svg->Add(Svg::Text{}
                         .SetPoint(position)
                         .SetOffset(render_settings->stop_label_offset)
                         .SetFontSize(render_settings->stop_label_font_size)
                         .SetFontFamily("Verdana")
                         .SetData(stop_name)
                         .SetFillColor(render_settings->underlayer_color)
                         .SetStrokeColor(render_settings->underlayer_color)
                         .SetStrokeWidth(render_settings->underlayer_width)
                         .SetStrokeLineCap("round")
                         .SetStrokeLineJoin("round"));
        svg->Add(Svg::Text{}
                         .SetPoint(position)
                         .SetOffset(render_settings->stop_label_offset)
                         .SetFontSize(render_settings->stop_label_font_size)
                         .SetFontFamily("Verdana")
                         .SetData(stop_name)
                         .SetFillColor("black"));
    }

    void Renderer::RenderStopLabels() {
        for (const auto &[stop_name, stop]: render_data_.database->stop_descriptions) {
            RenderSingleStopLabel(
                    &svg_, render_data_.render_settings, stop_name, GetPosition(stop.coordinates));
        }
    }

    void Renderer::RenderSingleBusLabel(
            Svg::Document *svg, SettingsPtr render_settings,
            const std::string &bus_name, Svg::Point position, Svg::Color color) {
        svg->Add(Svg::Text{}
                         .SetPoint(position)
                         .SetOffset(render_settings->bus_label_offset)
                         .SetFontSize(render_settings->bus_label_font_size)
                         .SetFontFamily("Verdana")
                         .SetFontWeight("bold")
                         .SetData(bus_name)
                         .SetFillColor(render_settings->underlayer_color)
                         .SetStrokeColor(render_settings->underlayer_color)
                         .SetStrokeWidth(render_settings->underlayer_width)
                         .SetStrokeLineCap("round")
                         .SetStrokeLineJoin("round"));
        svg->Add(Svg::Text{}
                         .SetPoint(position)
                         .SetOffset(render_settings->bus_label_offset)
                         .SetFontSize(render_settings->bus_label_font_size)
                         .SetFontFamily("Verdana")
                         .SetFontWeight("bold")
                         .SetData(bus_name)
                         .SetFillColor(color));
    }

    void Renderer::RenderBusLabels() {
        size_t bus_order_id = 0;
        for (const auto &[bus_name, bus]: render_data_.database->bus_descriptions) {
            auto color = render_data_.render_settings->color_palette[bus_order_id];
            auto position = GetPosition(
                    render_data_.database->stop_descriptions.at(bus.stops.front()).coordinates);
            RenderSingleBusLabel(
                    &svg_, render_data_.render_settings, bus_name, position, color);
            if (!bus.is_roundtrip && bus.stops.front() != bus.stops[bus.stops.size() / 2]) {
                position = GetPosition(
                        render_data_.database->stop_descriptions.at(bus.stops[bus.stops.size() / 2]).coordinates);
                RenderSingleBusLabel(
                        &svg_, render_data_.render_settings, bus_name, position, color);
            }
            (++bus_order_id) %= render_data_.render_settings->color_palette.size();
        }
    }

    const std::unordered_map<std::string, void (Renderer::*)()> Renderer::kCallLayer = {
            {"bus_lines",   &Renderer::RenderBusLines},
            {"bus_labels",  &Renderer::RenderBusLabels},
            {"stop_points", &Renderer::RenderStopPoints},
            {"stop_labels", &Renderer::RenderStopLabels}
    };

    Renderer::Renderer(RenderData render_data)
            : render_data_(RenderDataOptimizer(std::move(render_data)).Optimize()) {
        for (const auto &layer_name: render_data_.render_settings->layers) {
            (this->*kCallLayer.at(layer_name))();
        }
    }

    Response::Map Renderer::RenderMap() {
        std::ostringstream os;
        svg_.Render(os);
        return {os.str()};
    }

    std::vector<RouteByStops> TraverseRoute(const Response::Route::RouteItems &items) {
        std::vector<RouteByStops> route_scheme;
        for (const auto &item: items) {
            if (std::holds_alternative<Response::Route::Bus>(item)) {
                const auto &bus_segment = std::get<Response::Route::Bus>(item);
                for (auto it = bus_segment.from; it != bus_segment.to; ++it) {
                    route_scheme.emplace_back(*it, bus_segment.bus, false);
                }
                route_scheme.emplace_back(*bus_segment.to, bus_segment.bus, true);
            }
        }
        return route_scheme;
    }

    const std::unordered_map<std::string,
            void (Renderer::RouteHelper::*)()> Renderer::RouteHelper::kCallRouteLayer = {
            {"bus_lines",   &Renderer::RouteHelper::RenderRouteBusLines},
            {"bus_labels",  &Renderer::RouteHelper::RenderRouteBusLabels},
            {"stop_points", &Renderer::RouteHelper::RenderRouteStopPoints},
            {"stop_labels", &Renderer::RouteHelper::RenderRouteStopLabels}
    };

    Response::Map Renderer::RenderRoute(const Response::Route::RouteItems &items) const {
        auto route_scheme = TraverseRoute(items);
        RouteHelper render_helper(render_data_, svg_, route_scheme);
        const auto &route_svg = render_helper.GetSvg();
        std::ostringstream os;
        route_svg.Render(os);
        return {os.str()};
    }

    Renderer::RouteHelper::RouteHelper(RenderData render_data,
                                       const Svg::Document &base_svg,
                                       const std::vector<RouteByStops> &route_scheme)
            : render_data_(std::move(render_data)), route_svg_(base_svg), route_scheme_(route_scheme) {
        const double outer_margin = render_data_.render_settings->outer_margin;
        route_svg_.Add(Svg::Rectangle(-outer_margin, -outer_margin,
                                      render_data_.render_settings->width + 2 * outer_margin,
                                      render_data_.render_settings->height + 2 * outer_margin)
                               .SetFillColor(render_data_.render_settings->underlayer_color));
        for (const auto &layer_name: render_data_.render_settings->layers) {
            (this->*kCallRouteLayer.at(layer_name))();
        }
    }

    void Renderer::RouteHelper::RenderRouteStopPoints() {
        for (const auto &[stop_name, _, is_interchange]: route_scheme_) {
            route_svg_.Add(Svg::Circle{}
                                   .SetCenter(GetPosition(
                                           render_data_.database->stop_descriptions[stop_name].coordinates))
                                   .SetRadius(render_data_.render_settings->stop_radius)
                                   .SetFillColor("white"));
        }
    }

    void Renderer::RouteHelper::RenderRouteStopLabels() {
        bool first = true;
        for (const auto &[stop_name, _, is_interchange]: route_scheme_) {
            if (first || is_interchange) {
                first = false;
                RenderSingleStopLabel(
                        &route_svg_,
                        render_data_.render_settings,
                        stop_name,
                        GetPosition(
                                render_data_.database->stop_descriptions[stop_name].coordinates));
            }
        }
    }

    void Renderer::RouteHelper::RenderRouteBusLines() {
        for (auto it = route_scheme_.begin(); it != route_scheme_.end(); ++it) {
            Svg::Polyline polyline;
            polyline.SetStrokeColor(render_data_.database->bus_colors[it->bus_name])
                    .SetStrokeWidth(render_data_.render_settings->line_width)
                    .SetStrokeLineCap("round")
                    .SetStrokeLineJoin("round");
            for (; it != route_scheme_.end(); ++it) {
                polyline.AddPoint(GetPosition(
                        render_data_.database->stop_descriptions.at(it->stop_name).coordinates));
                if (it->is_interchange) {
                    break;
                }
            }
            route_svg_.Add(std::move(polyline));
        }
    }

    void Renderer::RouteHelper::RenderRouteBusLabels() {
        for (const auto &[stop_name, bus_name, _]: route_scheme_) {
            if (IsFinishStop(render_data_.database, stop_name, bus_name)) {
                RenderSingleBusLabel(
                        &route_svg_,
                        render_data_.render_settings,
                        bus_name,
                        GetPosition(
                                render_data_.database->stop_descriptions[stop_name].coordinates),
                        render_data_.database->bus_colors[bus_name]);
            }
        }
    }

}
