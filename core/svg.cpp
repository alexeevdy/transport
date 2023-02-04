#include "svg.h"

namespace Svg {

    std::ostream &operator<<(std::ostream &out, const Color &color) {
        if (std::holds_alternative<std::string>(color.data_)) {
            out << std::get<std::string>(color.data_);
        } else if (std::holds_alternative<Rgb>(color.data_)) {
            auto rgb = std::get<Rgb>(color.data_);
            out << "rgb(" << static_cast<int>(rgb.red) << ","
                << static_cast<int>(rgb.green) << ","
                << static_cast<int>(rgb.blue) << ")";
        } else if (std::holds_alternative<Rgba>(color.data_)) {
            auto rgba = std::get<Rgba>(color.data_);
            out << "rgba(" << static_cast<int>(rgba.red) << ","
                << static_cast<int>(rgba.green) << ","
                << static_cast<int>(rgba.blue) << ","
                << rgba.alpha << ")";
        } else {
            throw std::runtime_error("unknown color variant");
        }
        return out;
    }

    template<class Derived>
    void ObjectProps<Derived>::ListCommon(std::ostream &out) const {
        out << "fill=\"" << fill_color_ << "\" stroke=\""
            << stroke_color_ << "\" stroke-width=\""
            << stroke_width_ << "\" ";
        if (stroke_line_cap_) {
            out << "stroke-linecap=\"" << *stroke_line_cap_ << "\" ";
        }
        if (stroke_line_join_) {
            out << "stroke-linejoin=\"" << *stroke_line_join_ << "\" ";
        }
    }

    void Circle::Render(std::ostream &out) const {
        out << "<circle cx=\"" << center_.x << "\" cy=\""
            << center_.y << "\" r=\"" << radius_ << "\" ";
        ListCommon(out);
        out << "/>";
    }

    void Polyline::Render(std::ostream &out) const {
        out.precision(10);
        out << "<polyline ";
        ListCommon(out);
        out << "points=\"";
        bool first = true;
        for (auto point: points_) {
            if (first) {
                first = false;
                out << std::fixed << point.x << "," << point.y;
                continue;
            }
            out << " " << std::fixed << point.x << "," << point.y;
        }
        out << "\" />";
    }

    void Text::Render(std::ostream &out) const {
        out << "<text x=\"" << position_.x << "\" y=\"" << position_.y
            << "\" dx=\"" << offset_.x << "\" dy=\"" << offset_.y
            << "\" font-size=\"" << font_size_ << "\" ";
        if (font_family_) {
            out << "font-family=\"" << *font_family_ << "\" ";
        }
        if (font_weight_) {
            out << "font-weight=\"" << *font_weight_ << "\" ";
        }
        ListCommon(out);
        out << ">" << text_ << "</text>";
    }

    void Rectangle::Render(std::ostream & out) const {
        out << "<rect x=\"" << edge_.x << "\" y=\"" << edge_.y
            << "\" width=\"" << width_ << "\" height=\"" << height_ << "\" ";
        ListCommon(out);
        out << "/>";
    }

}