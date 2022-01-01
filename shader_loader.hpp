#pragma once

#include <string>
#include <iostream>
#include <fstream>

struct ShaderLoader
{
    ShaderLoader(const std::string &filename)
        : m_filename(filename)
    {
    }

    std::string load()
    {
        std::string content;
        std::ifstream ifs(m_filename, std::ios::in);
        if(!ifs) {
            throw std::runtime_error("Help = missing shader!");
        }
        std::string line = "";
        while (!ifs.eof())
        {
            std::getline(ifs, line);
            content.append(line + "\n");
        }

        return content;
    }

    const std::string m_filename;
};
