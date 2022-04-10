#pragma once

#include <memory>
#include <glm/glm.hpp>
#include "database.hpp"
#include "graph.hpp"
#include "window.hpp"
#include "plugin_manager.hpp"
#include "timeseries.hpp"

class AppContext
{
    struct TimeSeriesContainer
    {
        std::shared_ptr<TimeSeries> ts;
        glm::vec3 colour;
        std::string name;
        bool visible;
        float y_offset;
    };

  public:
    AppContext(Database &database,
               Graph &graph,
               Plot &plot,
               Window &window,
               PluginManager &plugin_manager);

    void draw();

  private:
    void draw_gui();
    void update_multisampling() const;
    void update_vsync() const;
    void update_bgcolour() const;
    glm::dvec2 screen2graph(const glm::ivec2 &value) const;

    static constexpr double ZOOM_MIN_X = 1'000'000.0;
    static constexpr double ZOOM_MIN_Y = 1'000'000.0;

    Database &m_database;
    Graph &m_graph;
    Plot &m_plot;
    Window &m_window;
    PluginManager &m_plugin_manager;
    std::vector<TimeSeriesContainer> m_ts;
    int m_plot_width = 1;
    bool m_enable_vsync = true;
    bool m_enable_multisampling = true;
    glm::vec3 m_bgcolor;
    glm::dmat3 m_view_matrix;
    bool m_show_line_segments = false;
};
