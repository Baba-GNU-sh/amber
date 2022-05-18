#pragma once

#include <vector>
#include <memory>
#include <string>
#include <glm/glm.hpp>
#include <glm/gtx/matrix_transform_2d.hpp>
#include <database/timeseries.hpp>
#include "utils/transform.hpp"

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

    GraphState()
    {
        view.update(glm::dmat3(1.0));
    }

    void goto_newest_sample()
    {
        auto view_matrix_copy = view.matrix();
        view_matrix_copy[2][0] = 0;

        std::vector<TimeSeriesState> visible_timeseries;
        std::copy_if(timeseries.begin(),
                     timeseries.end(),
                     std::back_inserter(visible_timeseries),
                     [](const auto &ts) { return ts.visible; });

        const auto max_element = std::max_element(
            visible_timeseries.begin(), visible_timeseries.end(), [](const auto &a, const auto &b) {
                return a.ts->get_span().second > b.ts->get_span().second;
            });

        auto latest_sample_time = max_element->ts->get_span().second;

        const auto offset_a = view.apply_inverse(glm::dvec2(0.0, 0.0));
        const auto offset_b = view.apply_inverse(glm::dvec2(1.0, 0.0));
        const auto offset = offset_b - offset_a;
        view_matrix_copy = glm::translate(view_matrix_copy,
                                          glm::dvec2(offset) + glm::dvec2(-latest_sample_time, 0));
        view.update(view_matrix_copy);
    }

    void fit_graph(const glm::dvec2 &start, const glm::dvec2 &end)
    {
        glm::dmat3 view_matrix(1.0);

        const auto delta = glm::abs(end - start);
        const auto scaling_factor = 2.0 / delta;
        view_matrix = glm::scale(view_matrix, scaling_factor);

        const auto translation = (start + end) / 2.0;
        view_matrix = glm::translate(view_matrix, -translation);

        view.update(view_matrix);
    }

    int plot_width = 2;

    /**
     * @brief This transform describes how the graph maps to the viewable area of the graph.
     */
    Transform<double> view;
    bool show_line_segments = false;
    std::pair<MarkerState, MarkerState> markers;
    std::vector<TimeSeriesState> timeseries;
    bool sync_latest_data = false;
};
