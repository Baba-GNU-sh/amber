#pragma once

class Plugin
{
  public:
    virtual void start() = 0;
    virtual void stop() = 0;
    virtual bool is_running() const = 0;
    virtual void draw_menu() = 0;
};
