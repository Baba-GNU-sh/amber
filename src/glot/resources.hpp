#pragma once

#include <filesystem>
#include <spdlog/spdlog.h>
#include <stdexcept>

class Resources
{
  public:
    static std::string find_shader(const char *name)
    {
        auto root = get_root();
        auto path = root / "shaders" / name;
        if (std::filesystem::exists(path))
        {
            spdlog::info("Found shader: {}", path.string());
            return path.string();
        }
        else
        {
            spdlog::error("No shader '{}' found in {}", name, root.string());
            throw std::runtime_error("Cannot find font");
        }
    }

    static std::string find_font(const char *name)
    {
        auto root = get_root() / "assets" / "fonts";
        auto path = root / name;
        if (std::filesystem::exists(path))
        {
            spdlog::info("Found font: {}", path.string());
            return path.string();
        }
        else
        {
            spdlog::error("No font '{}' found in {}", name, root.string());
            throw std::runtime_error("Cannot find font");
        }
    }

    static std::string find_sprite(const std::string &name)
    {
        auto root = get_root() / "assets" / "sprites";
        auto path = root / name;
        if (std::filesystem::exists(path))
        {
            spdlog::info("Found texture: {}", path.string());
            return path.string();
        }
        else
        {
            spdlog::error("No sprite '{}' found in {}", name, root.string());
            throw std::runtime_error("Cannot find sprite");
        }
    }

  private:
    static std::filesystem::path get_root()
    {
        // TODO change this to use some invironment variable or something for
        // locating resource files
        auto *env = std::getenv("GLOT_ROOT");
        if (env)
        {
            return std::filesystem::path(env);
        }
        return std::filesystem::current_path();
    }
};
