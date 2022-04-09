#pragma once

#include <memory>
#include <glm/glm.hpp>
#include "database.hpp"
#include "graph.hpp"
#include "window_container.hpp"
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
               WindowContainer &window,
               PluginManager &plugin_manager);

    void draw();

  private:
    void draw_gui();

    void _update_multisampling();
    void _update_vsync() const;
    void _update_bgcolour() const;

    Database &m_database;
    Graph &m_graph;
    WindowContainer &m_window;
    PluginManager &m_plugin_manager;
    std::vector<TimeSeriesContainer> m_ts;
    int m_plot_width = 1;
    bool m_enable_vsync = true;
    bool m_enable_multisampling = true;
    glm::vec3 m_bgcolor;
};
