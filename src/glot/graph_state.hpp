#pragma once

#include <vector>
#include <memory>
#include <string>
#include <glm/glm.hpp>
#include <glm/gtx/matrix_transform_2d.hpp>
#include <database/timeseries.hpp>

struct GraphState
{
    struct TimeSeriesState
    {
        std::shared_ptr<TimeSeries> ts;
        glm::vec3 colour;
        std::string name;
        bool visible = false;
        float y_offset = 0.0;
    };

    struct MarkerState
    {
        bool visible = false;
        double position = 0.0;
    };

    int plot_width = 2;
    glm::dmat3 view_matrix;
    glm::dmat3 view_matrix_inv;
    bool show_line_segments = false;
    std::pair<MarkerState, MarkerState> markers;
    std::vector<TimeSeriesState> timeseries;
    bool sync_latest_data = false;

    void update_view_matrix(const glm::dmat3 &value)
    {
        view_matrix = value;
        view_matrix_inv = glm::inverse(value);
    }

    void goto_newest_sample()
    {
        auto view_matrix_copy = view_matrix;
        view_matrix_copy[2][0] = 0;

        double latest_sample_time = 0.0;
        for (const auto &ts : timeseries)
        {
            if (ts.visible)
            {
                latest_sample_time = std::max(ts.ts->get_span().second, latest_sample_time);
            }
        }

        const auto offset_a = view_matrix_inv * glm::dvec3(0.0, 0.0, 1.0);
        const auto offset_b = view_matrix_inv * glm::dvec3(1.0, 0.0, 1.0);
        const auto offset = offset_b - offset_a;
        view_matrix_copy = glm::translate(view_matrix_copy,
                                          glm::dvec2(offset) + glm::dvec2(-latest_sample_time, 0));
        update_view_matrix(view_matrix_copy);
    }

    GraphState()
    {
        update_view_matrix(glm::dmat3(1.0));
    }
};
