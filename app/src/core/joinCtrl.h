#pragma once

#include <glm.hpp>
#include <map>

#include "clipCtrl.h"
#include "ui/util/polyTest.hpp"
#include "ui/util/rectTest.hpp"
#include "xdBase.h"

namespace CC
{
    /// @brief 一块瓦片
    struct Tile
    {
    public:
        Tile() {}
        Tile(const size_t& clip) : _clipId(clip) {}
        Tile(const glm::ivec2& pos) : _pos(pos) {}
        Tile(const size_t& clip, const glm::ivec2& pos) : _clipId(clip), _pos(pos) {}
        Tile(const Tile& o) :
            _scale(o._scale),
            _pos(o._pos),
            _clipId(o._clipId),
            _rota(o._rota),
            _children(o._children),
            _dirty(true) {}
        Tile(Tile&& o) :
            _scale(o._scale),
            _pos(o._pos),
            _clipId(o._clipId),
            _rota(o._rota),
            _children(std::move(o._children)),
            _dirty(true) {}

        const Tile& operator=(const Tile& o)
        {
            _scale = o._scale;
            _pos = o._pos;
            _clipId = o._clipId;
            _rota = o._rota;
            _children = o._children;
            _dirty = true;
            return *this;
        }
        const Tile& operator=(Tile&& o)
        {
            _scale = o._scale;
            _pos = o._pos;
            _clipId = o._clipId;
            _rota = o._rota;
            _children.swap(o._children);
            _dirty = true;
            return *this;
        }

        struct TileVData { glm::ivec2 pLU; glm::vec2 pLD; glm::vec2 pRD; glm::vec2 pRU;};

        TileVData getVDataLocal() const
        {
            if (_dirty) updateTileVData();
            return _vDataCacheLocal;
        }

        bool test(const glm::ivec2& min, const glm::ivec2& size) const
        {
            if (_dirty) updateTileVData();
            if (!twoRectTestFloat(min, size, _aabbMin, _aabbSize)) return false;
            auto v1 = (_vDataCacheLocal.pRD - _vDataCacheLocal.pLD) * 0.5f,
                v2 = (glm::vec2(_vDataCacheLocal.pLU) - _vDataCacheLocal.pLD) * 0.5f,
                a1 = glm::vec2(min + glm::ivec2(size.x, 0)) * 0.5f,
                a2 = glm::vec2(min + glm::ivec2(0, size.y)) * 0.5f,
                c1 = a1 + a2,
                c2 = (v1 + v2) * 0.5f;

            if (glm::dot(a1, v1) / (float(size.x) * 0.5f) +
                glm::dot(a2, v1) / (float(size.y) * 0.5f) >
                glm::dot(c1 - c2, v1) / (glm::length(c1 - c2) * 0.5f) ||
                glm::dot(a1, v2) / (float(size.x) * 0.5f) +
                glm::dot(a2, v2) / (float(size.y) * 0.5f) >
                glm::dot(c1 - c2, v2) / (glm::length(c1 - c2) * 0.5f))
                return true;
            return false;
        }

        bool test(const glm::ivec2& pos) const
        {
            if (_dirty) updateTileVData();
            if (triangleTest(_vDataCacheLocal.pLU, _vDataCacheLocal.pLD, _vDataCacheLocal.pRU, pos) ||
                triangleTest(_vDataCacheLocal.pLD, _vDataCacheLocal.pRD, _vDataCacheLocal.pRU, pos))
                return true;
            return false;
        }

        void setClip(size_t clip = SIZE_MAX) noexcept { _clipId = clip; _dirty = true; }
        void setPos(glm::ivec2 p) noexcept { _pos = p; _dirty = true; }
        void setScale(glm::vec2 s) noexcept { _scale = s; _dirty = true; }
        void setRota(float r) noexcept { _rota = r; _dirty = true; }

        /// @brief 遍历 (包括根节点)
        /// @param func void(tile)
        void traverse(std::function<void(const Tile&)> func) const
        {
            func(*this);
            for (auto& t : _children) t.traverse(func);
        }

        /// @brief 遍历 (包括根节点)
        /// @param func void(curTile, parentTile*)
        /// @param parent nullptr
        void traverse(std::function<void(const Tile&, const Tile*)> func, const Tile* parent = nullptr) const
        {
            func(*this, parent);
            for (auto& t : _children) t.traverse(func, this);
        }

        /// @brief 遍历子节点
        /// @param func void(tile)
        void traverseChild(std::function<void(const Tile&)> func) const
        {
            for (auto& t : _children) t.traverse(func);
        }

        /// @brief 遍历子节点
        /// @param func void(curTile, parentTile)
        void traverseChild(std::function<void(const Tile&, const Tile*)> func) const
        {
            for (auto& t : _children) t.traverse(func, this);
        }

        /// @brief 遍历并计算偏移量 (包括根节点)
        /// @param func void(tile)
        void traverseWithOffset(std::function<void(const Tile&, const glm::ivec2&)> func, const glm::ivec2& offset = {}) const
        {
            func(*this, offset);
            for (auto& t : _children) t.traverseWithOffset(func, _pos + offset);
        }

    private:
        void updateTileVData() const
        {
            if (_clipId >= ClipCtrl::getCurClips().size() || ClipCtrl::getCurClips()[_clipId].empty)
            {
                _aabbMin = _pos;
                _vDataCacheLocal = {_aabbMin, _aabbMin, _aabbMin, _aabbMin};
                _aabbSize = glm::vec2(0, 0);
                _dirty = false;
                return;
            }
            _vDataCacheLocal.pLU = _pos;
            _size = glm::vec2(ClipCtrl::getCurClips()[_clipId].size) * _scale;
            _vDataCacheLocal.pLD = glm::vec2(0, _size.y);
            _vDataCacheLocal.pRD = _size;
            _vDataCacheLocal.pRU = glm::vec2(_size.x, 0);

            auto mR = glm::mat2x2(
                glm::cos(_rota), glm::sin(_rota),
                -glm::sin(_rota), glm::cos(_rota));
            _vDataCacheLocal.pLD = mR * _vDataCacheLocal.pLD;
            _vDataCacheLocal.pRD = mR * _vDataCacheLocal.pRD;
            _vDataCacheLocal.pRU = mR * _vDataCacheLocal.pRU;

            _vDataCacheLocal.pLD += _pos;
            _vDataCacheLocal.pRD += _pos;
            _vDataCacheLocal.pRU += _pos;

            _aabbMin = glm::min(
                _vDataCacheLocal.pLD, glm::min(
                _vDataCacheLocal.pRD, glm::min(
                _vDataCacheLocal.pRU,
                glm::vec2(_vDataCacheLocal.pLU))));
            _aabbSize = glm::max(
                _vDataCacheLocal.pLD, glm::max(
                _vDataCacheLocal.pRD, glm::max(
                _vDataCacheLocal.pRU,
                glm::vec2(_vDataCacheLocal.pLU))));
            _aabbSize -= _aabbMin;

            _dirty = false;
        }

        // 缓存的计算数据
        mutable TileVData _vDataCacheLocal;
        mutable glm::vec2 _size             = glm::vec2();
        mutable glm::vec2 _aabbMin          = glm::vec2();
        mutable glm::vec2 _aabbSize         = glm::vec2();
        mutable bool _dirty                 = true;

        // tile 数据
        glm::vec2 _scale            = glm::vec2(1.f, 1.f);
        glm::ivec2 _pos             = glm::ivec2();
        size_t _clipId              = SIZE_MAX;
        float _rota                 = 0;
        std::vector<Tile> _children;
    };
}