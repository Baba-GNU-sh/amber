#pragma once

#include <array>
#include <cstddef>
#include <deque>
#include <memory>
#include <vector>

#include <iterator>

template <typename T, unsigned int ChunkSize> class ChunkedVector
{
    typedef std::array<T, ChunkSize> Chunk;

  public:
    struct Iterator
    {
        using iterator_category = std::forward_iterator_tag;
        using difference_type = std::ptrdiff_t;
        using value_type = T;
        using pointer = T *;
        using reference = T &;

        Iterator(ChunkedVector<T, ChunkSize> *source, std::size_t offset)
            : m_source(source), m_offset(offset)
        {
            //
        }

        Iterator &operator=(const Iterator &other)
        {
            m_source = other.m_source;
            m_offset = other.m_offset;
            return *this;
        }

        T &operator*()
        {
            return (*m_source)[m_offset];
        }

        pointer operator->()
        {
            return &m_source[m_offset];
        }

        Iterator &operator++()
        {
            m_offset++;
            return *this;
        }

        bool operator==(const Iterator &other) const
        {
            return m_offset == other.m_offset;
        }

        bool operator!=(const Iterator &other) const
        {
            return m_offset != other.m_offset;
        }

        ChunkedVector<T, ChunkSize> *m_source;
        std::size_t m_offset;
    };

    struct ConstIterator
    {
        using iterator_category = std::forward_iterator_tag;
        using difference_type = std::ptrdiff_t;
        using value_type = T;
        using pointer = T *;
        using reference = T &;

        ConstIterator(const ChunkedVector<T, ChunkSize> *source, std::size_t offset)
            : m_source(source), m_offset(offset)
        {
            //
        }

        ConstIterator &operator=(const ConstIterator &other)
        {
            m_source = other.m_source;
            m_offset = other.m_offset;
            return *this;
        }

        const T &operator*() const
        {
            return (*m_source)[m_offset];
        }

        const T *operator->()
        {
            return &((*m_source)[m_offset]);
        }

        ConstIterator &operator++()
        {
            m_offset++;
            return *this;
        }

        bool operator==(const ConstIterator &other) const
        {
            return m_offset == other.m_offset;
        }

        bool operator!=(const ConstIterator &other) const
        {
            return m_offset != other.m_offset;
        }

        const ChunkedVector<T, ChunkSize> *m_source;
        std::size_t m_offset;
    };

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

    T &operator[](std::size_t index)
    {
        const auto chunk_index = index / ChunkSize;
        const auto offset = index % ChunkSize;
        auto &chunk = *(_map[chunk_index]);
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

    Iterator begin()
    {
        return Iterator{this, 0};
    }

    Iterator end()
    {
        return Iterator{this, size() - 1};
    }

    ConstIterator begin() const
    {
        return ConstIterator{this, 0};
    }

    ConstIterator end() const
    {
        return ConstIterator{this, size() - 1};
    }

  private:
    std::vector<std::unique_ptr<Chunk>> _map;
    std::size_t _size;
};
