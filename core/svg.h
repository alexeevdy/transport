#pragma once

#include <iostream>
#include <utility>
#include <vector>
#include <memory>
#include <string>
#include <sstream>
#include <optional>
#include <variant>

namespace Svg {

    struct Point {
        Point() : x(0.), y(0.) {}

        Point(double _x, double _y) : x(_x), y(_y) {}

        double x;
        double y;
    };

    struct Rgb {
        Rgb() : red(0), green(0), blue(0) {}

        Rgb(int r, int g, int b) : red(r), green(g), blue(b) {}

        uint8_t red;
        uint8_t green;
        uint8_t blue;
    };

    struct Rgba : Rgb {
        Rgba() : Rgb(), alpha(1.) {}

        Rgba(int r, int g, int b, double a)
            : Rgb(r, g, b), alpha(a) {}

        Rgba(Rgb _rgb, double a)
            : Rgb(_rgb), alpha(a) {}

        double alpha;
    };

    class Color {
    public:
        Color() : data_("none") {};

        Color(std::string name) : data_(std::move(name)) {}

        Color(const char *name) : Color(std::string(name)) {}

        Color(Rgb rgb) : data_(rgb) {}

        Color(Rgba rgba) : data_(rgba) {}

    private:
        friend std::ostream &operator<<(std::ostream &, const Color &);

        std::variant<std::string, Rgb, Rgba> data_;
    };

    const Color NoneColor = "none";

    struct ObjectBase {
        virtual void Render(std::ostream &) const = 0;
        virtual ~ObjectBase() = default;
    };

    template<class Derived>
    class ObjectProps {
    public:
        ObjectProps() : stroke_width_(1.) {}

        void ListCommon(std::ostream &) const;

        Derived &SetFillColor(const Color &color) {
            fill_color_ = color;
            return cthis();
        }

        Derived &SetStrokeColor(const Color &color) {
            stroke_color_ = color;
            return cthis();
        }

        Derived &SetStrokeWidth(double width) {
            stroke_width_ = width;
            return cthis();
        }

        Derived &SetStrokeLineCap(const std::string &line_cap) {
            stroke_line_cap_ = line_cap;
            return cthis();
        }

        Derived &SetStrokeLineJoin(const std::string &line_join) {
            stroke_line_join_ = line_join;
            return cthis();
        }

        virtual ~ObjectProps() = default;

    private:
        Color fill_color_;
        Color stroke_color_;
        double stroke_width_;
        std::optional<std::string> stroke_line_cap_;
        std::optional<std::string> stroke_line_join_;

        friend Derived;

        Derived &cthis() { return static_cast<Derived &>(*this); }

    };

    class Circle : public ObjectBase, public ObjectProps<Circle> {
    public:
        Circle() : radius_(1.) {};

        void Render(std::ostream &) const override;

        Circle &SetCenter(Point point) {
            center_ = point;
            return *this;
        }

        Circle &SetRadius(double radius) {
            radius_ = radius;
            return *this;
        }

    private:
        Point center_;
        double radius_;
    };

    class Polyline : public ObjectBase, public ObjectProps<Polyline> {
    public:
        Polyline() = default;

        void Render(std::ostream &) const override;

        Polyline &AddPoint(Point point) {
            points_.push_back(point);
            return *this;
        }

    private:
        std::vector<Point> points_;
    };

    class Text : public ObjectBase, public ObjectProps<Text> {
    public:
        Text() : font_size_(1) {}

        void Render(std::ostream &) const override;

        Text &SetPoint(Point point) {
            position_ = point;
            return *this;
        }

        Text &SetOffset(Point point) {
            offset_ = point;
            return *this;
        }

        Text &SetFontSize(uint32_t size) {
            font_size_ = size;
            return *this;
        }

        Text &SetFontFamily(const std::string &font) {
            font_family_ = font;
            return *this;
        }

        Text &SetFontWeight(const std::string &font_weight) {
            font_weight_ = font_weight;
            return *this;
        }

        Text &SetData(const std::string &text) {
            text_ = text;
            return *this;
        }

        ~Text() override = default;

    private:
        Point position_;
        Point offset_;
        uint32_t font_size_;
        std::optional<std::string> font_family_;
        std::optional<std::string> font_weight_;
        std::string text_;
    };

    class Document {
    public:
        Document() = default;

        template<class DerivedObject>
        Document &Add(DerivedObject obj) {
            objects_.push_back(std::make_shared<DerivedObject>(std::move(obj)));
            return *this;
        }

        void Render(std::ostream &) const;

        void Clear() {
            objects_.clear();
        }

    private:
        std::vector<std::shared_ptr<ObjectBase>> objects_;
    };

    using DocumentPtr = std::shared_ptr<Document>;

}