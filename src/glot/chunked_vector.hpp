#pragma once

#include <array>
#include <cstddef>
#include <vector>
#include <deque>
#include <memory>
#include <spdlog/spdlog.h>

template <typename T, unsigned int ChunkSize> class ChunkedVector
{
    typedef std::array<T, ChunkSize> Chunk;

  public:
    ChunkedVector() : _size(0), _chunks(0)
    {
    }

    void push_back(const T &value)
    {
        push(value);
    }

    void push(const T &value)
    {
        ++_size;
        if (_size > ChunkSize * _chunks)
        {
            ++_chunks;
            auto chunk = std::make_shared<Chunk>();
            // spdlog::info("Allocation {}", _chunks);
            _map.push_back(chunk);
        }

        // const auto chunk_index = (_size - 1) / ChunkSize;
        const auto offset = (_size - 1) % ChunkSize;
        auto &chunk = *(_map.back());
        chunk[offset] = value;
    }

    std::size_t size() const
    {
        return _size;
    }

    const T &at(std::size_t index) const
    {
        if (index > size())
        {
            throw std::out_of_range("Out of range");
        }

        return (*this)[index];
    }

    const T &operator[](std::size_t index) const
    {
        const auto chunk_index = index / ChunkSize;
        const auto offset = index % ChunkSize;
        const auto &chunk = *(_map[chunk_index]);
        return chunk[offset];
    }

    const T &back() const
    {
        return (*this)[_size - 1];
    }

    const T &front() const
    {
        return (*this)[0];
    }

    bool empty() const
    {
        return _size == 0;
    }

  private:
    std::vector<std::shared_ptr<Chunk>> _map;
    std::size_t _size;
    std::size_t _chunks;
};