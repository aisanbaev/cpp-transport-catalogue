#pragma once

#include "domain.h"
#include "svg.h"

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <optional>
#include <vector>
#include <set>
#include <map>

class SphereProjector;

class MapRenderer {
public:
    MapRenderer(const RenderSettings& render_settings, const std::unordered_map<std::string_view, const Bus*>& busname_to_bus);

    const svg::Document& GetDocumentSVG() const;
    std::string GetMapAsString() const;

private:
    svg::Document document_;
    RenderSettings render_settings_;
    const int palette_size_ = render_settings_.color_palette.size();

    const std::unordered_map<std::string_view, const Bus*>& busname_to_bus_;

    std::set<std::string_view> buses_;
    std::map<std::string_view, geo::Coordinates> stopname_to_coord_;
    std::vector<geo::Coordinates> geo_coords_;

    void DrawBusRoutes(const SphereProjector& proj);
    void DrawRouteNames(const SphereProjector& proj);
    void DrawStops(const SphereProjector& proj);
    void DrawStopNames(const SphereProjector& proj);

    void CreateDocumentSVG();
};

inline const double EPSILON = 1e-6;
bool IsZero(double value);

class SphereProjector {
public:
    // points_begin � points_end ������ ������ � ����� ��������� ��������� geo::Coordinates
    template <typename PointInputIt>
    SphereProjector(PointInputIt points_begin, PointInputIt points_end,
                    double max_width, double max_height, double padding)
        : padding_(padding) //
    {
        // ���� ����� ����������� ����� �� ������, ��������� ������
        if (points_begin == points_end) {
            return;
        }

        // ������� ����� � ����������� � ������������ ��������
        const auto [left_it, right_it] = std::minmax_element(
            points_begin, points_end,
            [](auto lhs, auto rhs) { return lhs.lng < rhs.lng; });
        min_lon_ = left_it->lng;
        const double max_lon = right_it->lng;

        // ������� ����� � ����������� � ������������ �������
        const auto [bottom_it, top_it] = std::minmax_element(
            points_begin, points_end,
            [](auto lhs, auto rhs) { return lhs.lat < rhs.lat; });
        const double min_lat = bottom_it->lat;
        max_lat_ = top_it->lat;

        // ��������� ����������� ��������������� ����� ���������� x
        std::optional<double> width_zoom;
        if (!IsZero(max_lon - min_lon_)) {
            width_zoom = (max_width - 2 * padding) / (max_lon - min_lon_);
        }

        // ��������� ����������� ��������������� ����� ���������� y
        std::optional<double> height_zoom;
        if (!IsZero(max_lat_ - min_lat)) {
            height_zoom = (max_height - 2 * padding) / (max_lat_ - min_lat);
        }

        if (width_zoom && height_zoom) {
            // ������������ ��������������� �� ������ � ������ ���������,
            // ���� ����������� �� ���
            zoom_coeff_ = std::min(*width_zoom, *height_zoom);
        } else if (width_zoom) {
            // ����������� ��������������� �� ������ ���������, ���������� ���
            zoom_coeff_ = *width_zoom;
        } else if (height_zoom) {
            // ����������� ��������������� �� ������ ���������, ���������� ���
            zoom_coeff_ = *height_zoom;
        }
    }

    // ���������� ������ � ������� � ���������� ������ SVG-�����������
    svg::Point operator()(geo::Coordinates coords) const {
        return {
            (coords.lng - min_lon_) * zoom_coeff_ + padding_,
            (max_lat_ - coords.lat) * zoom_coeff_ + padding_
        };
    }

private:
    double padding_;
    double min_lon_ = 0;
    double max_lat_ = 0;
    double zoom_coeff_ = 0;
};

