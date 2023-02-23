#pragma once

#include <glm.hpp>
#include <list>

#include "core/clipCtrl.h"
#include "ui/unit/imgUnit.h"
#include "xdBase.h"

namespace CC
{
    class ImgCtrl
    {
    public:
        class PicData
        {
        public:
            PicData(std::filesystem::path imgPath) : unit(imgPath), path(imgPath) {}
            UI::ImageUnit unit;
            std::filesystem::path path;
            glm::ivec2 scale = glm::ivec2(1, 1);
            glm::ivec2 pos = glm::ivec2(0, 0);
            glm::ivec2 getSize() const { return {(unit.desc().w * scale.x) / scale.y, (unit.desc().h * scale.x) / scale.y}; }
            ImTextureID getId() const { return unit.getId(); }
        };

    private:
        class ImgCtrlData
        {
        public:
            glm::vec2 force;
            glm::ivec4 aabb;
            std::list<PicData> imgs;
        };
        static std::unique_ptr<ImgCtrlData> _inst;
        static void calc()
        {
            _inst->force = {0.f, 0.f};
            _inst->aabb = {};
            if (_inst->imgs.empty()) return;
            glm::ivec2 min = _inst->imgs.front().pos;
            glm::ivec2 max = _inst->imgs.front().pos + _inst->imgs.front().getSize();
            for (auto& i : _inst->imgs)
            {
                _inst->force += (glm::vec2)(i.pos + i.getSize()) / 2.f;
                min = glm::min(min, i.pos);
                max = glm::max(max, i.pos + i.getSize());
            }
            _inst->force /= _inst->imgs.size();
            _inst->aabb = {min.x, min.y, max.x - min.x, max.y - min.y};
        }
        static void draw(
            const Clip& clip,
            std::function<void(const ImTextureID&, const glm::ivec2&, const glm::ivec2&, const glm::vec2&, const glm::vec2&)> dFunc,
            const glm::ivec2& __pRoot,
            const glm::ivec2& __pLocal = glm::ivec2());

    public:
        static const PicData& front();
        static const PicData& back();
        static bool empty();
        static bool pushBack();
        static void draw(std::function<void(const ImTextureID&, const glm::ivec2&, const glm::ivec2&)> dFunc);
        /// @brief
        /// @param rect
        /// @param dFunc void(textureId, min, size, uv_min, uv_max)
        static void draw(glm::ivec4 rect, std::function<void(const ImTextureID&, const glm::ivec2&, const glm::ivec2&, const glm::vec2&, const glm::vec2&)> dFunc);
        /// @brief
        /// @param rect
        /// @param dFunc void(textureId, min, size, uv_min, uv_max)
        static void draw(const Clip& clip, std::function<void(const ImTextureID&, const glm::ivec2&, const glm::ivec2&, const glm::vec2&, const glm::vec2&)> dFunc);
        static void popBack(bool recy = false);
        static void popFront();
        /// @brief
        /// @param func void(const img&, pos&, scale&)
        static void traverse(std::function<void(const PicData&, glm::ivec2&, glm::ivec2&)> func);
        static const glm::ivec4& getAABB();
        static const glm::vec2& getForce();
        static void clear();
    };
}