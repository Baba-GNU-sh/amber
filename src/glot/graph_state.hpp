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

    GraphState()
    {
        view.update(glm::dmat3(1.0));
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
    std::vector<TimeSeriesState> timeseries;
};
