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

    /**
     * @brief Align the newest sample with the left side of the view.
     */
    void goto_newest_sample()
    {
        // Zero out the x translation element of the view transform - centers the view on time=0
        auto view_matrix_copy = view.matrix();
        view_matrix_copy[2][0] = 0;
        view.update(view_matrix_copy);

        // Center the view on the latest sample
        view.translate(glm::dvec2(-latest_visibile_sample_time(), 0));

        // Translate the view to the left by half a screen, to align the latest sample with the right edge of the view
        const auto center_offset = view.apply_inverse_relative(glm::dvec2(1.0, 0.0));
        view.translate(center_offset);
    }

    /**
     * @brief Evalulate the time latest (newest) visible sample.
     */
    double latest_visibile_sample_time() const
    {
        double latest_sample_time = 0.0;
        for (std::size_t i = 0; i < timeseries.size(); ++i)
        {
            const auto &ts = timeseries[i];
            if (ts.visible)
            {
                const auto span = ts.ts->get_span();
                latest_sample_time = std::max(latest_sample_time, span.second);
            }
        }
        return latest_sample_time;
    }

    /**
     * @brief Fit the view into a rectangle defined by two points on the graph.
     * 
     * @param tl Top-left corner of the rectangle.
     * @param end Bottom-right corner of the rectangle.
     */
    void fit_graph(const glm::dvec2 &tl, const glm::dvec2 &br)
    {
        view.update(glm::dmat3(1.0));

        const auto delta = glm::abs(br - tl);
        const auto scaling_factor = 2.0 / delta;
        view.scale(scaling_factor);

        const auto translation = (tl + br) / 2.0;
        view.translate(-translation);
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
