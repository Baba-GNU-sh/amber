#pragma once

#include <memory>
#include <optional>
#include <glm/glm.hpp>
#include "database.hpp"
#include "graph_renderer_opengl.hpp"
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
    AppContext(Database &database, Graph &graph, Window &window, PluginManager &plugin_manager);

    void draw();
    void spin();

  private:
    void draw_gui();
    void update_multisampling() const;
    void update_vsync() const;
    void update_bgcolour() const;
    glm::dvec2 screen2graph(const glm::ivec2 &value) const;
    static std::pair<double, const char *> human_readable(std::size_t size,
                                                          double divisor = 1000,
                                                          std::vector<const char *> suffixes = {
                                                              "K", "M", "B", "T"});
    

    Database &m_database;
    Graph &m_graph;
    Window &m_window;
    PluginManager &m_plugin_manager;
    std::vector<TimeSeriesContainer> m_ts;
    int m_plot_width = 2;
    bool m_enable_vsync = true;
    bool m_enable_multisampling = true;
    glm::vec3 m_bgcolor;
    glm::dmat3 m_view_matrix;
    bool m_show_line_segments = false;
    bool m_call_glfinish = false;
    std::pair<std::optional<double>, std::optional<double>> m_markers;
};
