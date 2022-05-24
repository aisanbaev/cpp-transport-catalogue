#include "svg.h"

namespace svg {

using namespace std::literals;

void OstreamColorPrinter::operator()(std::monostate) const {
    out << "none"sv;
}

void OstreamColorPrinter::operator()(std::string color) const {
    out << color;
}

void OstreamColorPrinter::operator()(svg::Rgb rgb) const {
    out << "rgb("sv << unsigned(rgb.red) << ","sv
        << unsigned(rgb.green) << ","sv
        << unsigned(rgb.blue) << ")"sv;
}

void OstreamColorPrinter::operator()(svg::Rgba rgba) const {
    out << "rgba("sv << unsigned(rgba.red) << ","sv
        << unsigned(rgba.green) << ","sv
        << unsigned(rgba.blue) << ","sv
        << rgba.opacity << ")"sv;
}

std::ostream& operator<<(std::ostream& out, const Color& color) {
    std::visit(svg::OstreamColorPrinter{out}, color);
    return out;
}

std::ostream& operator<<(std::ostream& out, const StrokeLineCap& cap) {
    switch (cap) {
        case StrokeLineCap::BUTT :
            out << "butt"sv;
            break;

        case StrokeLineCap::ROUND :
            out << "round"sv;
            break;

        case StrokeLineCap::SQUARE :
            out << "square"sv;
            break;
    }

    return out;
}

std::ostream& operator<<(std::ostream& out, const StrokeLineJoin& join) {
    switch (join) {
        case StrokeLineJoin::ARCS :
            out << "arcs"sv;
            break;

        case StrokeLineJoin::BEVEL :
            out << "bevel"sv;
            break;

        case StrokeLineJoin::MITER :
            out << "miter"sv;
            break;

        case StrokeLineJoin::MITER_CLIP :
            out << "miter-clip"sv;
            break;

        case StrokeLineJoin::ROUND :
            out << "round"sv;
            break;
    }

    return out;
}

void Object::Render(const RenderContext& context) const {
    context.RenderIndent();

    // ���������� ����� ���� ����� ����������
    RenderObject(context);

    context.out << std::endl;
}

// ---------- Circle ------------------

Circle& Circle::SetCenter(Point center) {
    center_ = center;
    return *this;
}

Circle& Circle::SetRadius(double radius) {
    radius_ = radius;
    return *this;
}

void Circle::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<circle cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv;
    //out << "r=\""sv << radius_ << "\" "sv;
    out << "r=\""sv << radius_ << "\""sv;
    RenderAttrs(context.out);
    out << "/>"sv;
}

// ---------- Polyline ------------------

Polyline& Polyline::AddPoint(Point point) {
    points_.push_back(point);
    return *this;
}

void Polyline::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    bool is_first = false;

    out << "<polyline points=\""sv;
    for (const auto& p : points_) {
        if (is_first) {
            out << " "sv;
        } else {
            is_first = true;
        }
        out << p.x << ","s << p.y;
    }
    out << "\""sv;
    RenderAttrs(context.out);
    out << "/>"sv;
}

// ---------- Text ------------------

Text& Text::SetPosition(Point pos) {
    pos_ = pos;
    return *this;
}

Text& Text::SetOffset(Point offset) {
    offset_ = offset;
    return *this;
}

Text& Text::SetFontSize(uint32_t size) {
    size_ = size;
    return *this;
}

Text& Text::SetFontFamily(std::string font_family) {
    font_family_ = font_family;
    return *this;
}

Text& Text::SetFontWeight(std::string font_weight) {
    font_weight_ = font_weight;
    return *this;
}

Text& Text::SetData(std::string data) {
    data_ = data;
    return *this;
}

void Text::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<text";
    RenderAttrs(context.out);
    out << " x=\""sv << pos_.x << "\" y=\""sv << pos_.y;
    out << "\" dx=\""sv << offset_.x << "\" dy=\""sv << offset_.y;
    out << "\" font-size=\""sv << size_;
    if (font_family_) {
        out << "\" font-family=\""sv << *font_family_;
    }
    if (font_weight_) {
        out << "\" font-weight=\""sv << *font_weight_;
    }
    out << "\">"sv << data_;
    out << "</text>"sv;
}

// ---------- Document ------------------

void Document::AddPtr(std::unique_ptr<Object>&& obj) {
    objects_.emplace_back(std::move(obj));
}

void Document::Render(std::ostream& out) const {
    out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"sv << std::endl;
    out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"sv << std::endl;

    RenderContext ctx(out, 2, 2);
    for (const auto& obj : objects_) {
        obj->Render(ctx);
    }

    out << "</svg>"sv;
}

}  // namespace svg
