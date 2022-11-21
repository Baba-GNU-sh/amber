#pragma once

#include <array>
#include <cstddef>
#include <deque>
#include <memory>
#include <vector>
#include <stdexcept>

namespace amber::database
{
template <typename T, unsigned int ChunkSize> class ChunkedVector
{
    typedef std::array<T, ChunkSize> Chunk;

  public:
    ChunkedVector() : _size(0)
    {
        //
    }

    void push_back(const T &value)
    {
        push(value);
    }

    void push(const T &value)
    {
        ++_size;
        if (_size > ChunkSize * _map.size())
        {
            _map.push_back(std::make_unique<Chunk>());
        }

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

    std::size_t capacity() const
    {
        return _map.size() * ChunkSize;
    }

  private:
    std::vector<std::unique_ptr<Chunk>> _map;
    std::size_t _size;
};
} // namespace amber::database
