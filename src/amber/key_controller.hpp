#pragma once

class KeyController : public View
{
  public:
    KeyController(Window &window, Graph &graph) : m_window(window), m_graph(graph)
    {
        //
    }

    void on_key(int key, int, int action, int mods) override
    {
        if (action == GLFW_PRESS)
        {
            if (key == GLFW_KEY_F11 && action == GLFW_PRESS)
            {
                m_window.set_fullscreen(!m_window.is_fullscreen());
            }
            if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
            {
                m_window.request_close();
            }
            if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
            {
                m_graph.reveal_newest_sample();
            }
            if (key == GLFW_KEY_A && action == GLFW_PRESS)
            {
                if (mods == GLFW_MOD_CONTROL)
                {
                    m_graph.set_marker_visible(Graph::MarkerType::A, false);
                }
                else
                {
                    const auto cursor_gs = m_graph.cursor_gs();
                    m_graph.set_marker_visible(Graph::MarkerType::A, true);
                    m_graph.set_marker_position(Graph::MarkerType::A, cursor_gs.x);
                }
            }
            else if (key == GLFW_KEY_B && action == GLFW_PRESS)
            {
                if (mods == GLFW_MOD_CONTROL)
                {
                    m_graph.set_marker_visible(Graph::MarkerType::B, false);
                }
                else
                {
                    const auto cursor_gs = m_graph.cursor_gs();
                    m_graph.set_marker_visible(Graph::MarkerType::B, true);
                    m_graph.set_marker_position(Graph::MarkerType::B, cursor_gs.x);
                }
            }
            else if (key == GLFW_KEY_C && action == GLFW_PRESS)
            {
                m_graph.set_marker_visible(Graph::MarkerType::A, false);
                m_graph.set_marker_visible(Graph::MarkerType::B, false);
            }
        }
    }

  private:
    Window &m_window;
    Graph &m_graph;
};