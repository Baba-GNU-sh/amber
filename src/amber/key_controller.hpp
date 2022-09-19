#pragma once

#include "view.hpp"
class KeyController : public View
{
  public:
    KeyController(Window &window, Graph &graph) : m_window(window), m_graph(graph)
    {
        //
    }

    void on_key(Key key, int scancode, Action action, Modifiers mods) override
    {
        (void)scancode;

        if (action == Action::Press)
        {
            if (key == Key::F11)
            {
                m_window.set_fullscreen(!m_window.is_fullscreen());
            }
            if (key == Key::ESCAPE)
            {
                m_window.request_close();
            }
            if (key == Key::SPACE)
            {
                m_graph.reveal_newest_sample();
            }
            if (key == Key::A)
            {
                if (mods & Modifiers::Control)
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
            else if (key == Key::B)
            {
                if (mods & Modifiers::Control)
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
            else if (key == Key::C)
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
