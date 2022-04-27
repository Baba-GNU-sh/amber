#pragma once

#include <vector>
#include <memory>
#include <glm/glm.hpp>
#include <glm/gtx/matrix_transform_2d.hpp>
#include "timeseries.hpp"

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

        const auto view_matrix_new = glm::translate(view_matrix_copy, glm::dvec2(-latest_sample_time, 0)); 
        update_view_matrix(view_matrix_new);
    }

    GraphState()
    {
        update_view_matrix(glm::dmat3(1.0));
    }
};
