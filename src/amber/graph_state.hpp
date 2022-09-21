#pragma once

#include <vector>
#include <memory>
#include <string>
#include <glm/glm.hpp>
#include <glm/gtx/matrix_transform_2d.hpp>
#include <database/timeseries.hpp>
#include "utils/transform.hpp"

namespace amber
{
struct GraphState
{
    struct TimeSeriesState
    {
        std::shared_ptr<database::TimeSeries> ts;
        glm::vec3 colour;
        std::string name;
        bool visible = false;
        float y_offset = 0.0;
    };

    int plot_width = 2;
    bool show_line_segments = false;
    std::vector<TimeSeriesState> timeseries;
};
} // namespace amber
