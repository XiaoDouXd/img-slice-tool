#include "fileCtrl.h"
#include "imgCtrl.h"
#include "ui/util/rectTest.hpp"

namespace CC
{
    std::unique_ptr<ImgCtrl::ImgCtrlData> ImgCtrl::_inst = std::make_unique<ImgCtrl::ImgCtrlData>();

    const ImgCtrl::PicData& ImgCtrl::front() { return _inst->imgs.front(); }
    const ImgCtrl::PicData& ImgCtrl::back() { return _inst->imgs.back(); }
    bool ImgCtrl::empty() { return _inst->imgs.empty(); }

    void ImgCtrl::draw(std::function<void(const ImTextureID&, const glm::ivec2&, const glm::ivec2&)> dFunc)
    {
        for (const auto& i : _inst->imgs)
            dFunc(i.getId(), i.pos, i.getSize());
    }

    void ImgCtrl::draw(glm::ivec4 rect, std::function<void(const ImTextureID&, const glm::ivec2&, const glm::ivec2&, const glm::vec2&, const glm::vec2&)> dFunc)
    {
        // --------------------------------------------
        //        |rect |img  |rect  |img  |
        glm::ivec2 min1, min2, size1, size2, size;
        glm::vec2 uvMin, uvMax;
        for (const auto& i : _inst->imgs)
        {
            min1 = {glm::min(rect.x, rect.z), glm::min(rect.y, rect.w)};
            size1 = {abs(rect.z - rect.x), abs(rect.w - rect.y)};
            min2 = i.pos;
            size2 = i.getSize();
            if (twoRectTest(min1, size1, min2, size2))
            {
                uvMin.x = glm::clamp(((float)(min1.x - min2.x) / size2.x), 0.f, 1.f);
                uvMin.y = glm::clamp(((float)(min1.y - min2.y) / size2.y), 0.f, 1.f);
                uvMax.x = glm::clamp(((float)(min1.x + size1.x - min2.x) / size2.x), 0.f, 1.f);
                uvMax.y = glm::clamp(((float)(min1.y + size1.y - min2.y) / size2.y), 0.f, 1.f);
                size = glm::min(glm::min(min2 + size2 - min1, size1), min1 + size1 - min2);
                if (size.x < 1 || size.y < 1) continue;
                dFunc(i.getId(), glm::max(min1, min2) - min1, size, uvMin, uvMax);
            }
        }
    }

    void ImgCtrl::draw(
        const Clip& clip,
        std::function<void(const ImTextureID&, const glm::ivec2&, const glm::ivec2&, const glm::vec2&, const glm::vec2&)> dFunc)
    { draw(clip, dFunc, clip.min); }

    void ImgCtrl::draw(
        const Clip& clip,
        std::function<void(const ImTextureID&, const glm::ivec2&, const glm::ivec2&, const glm::vec2&, const glm::vec2&)> dFunc,
        const glm::ivec2& __pRoot,
        const glm::ivec2& __pLocal)
    {
        if (clip.empty) return;
        // --------------------------------------------
        //        |rect |img  |rect  |img  |
        glm::ivec2 min1, min2, size1, size2, size;
        glm::vec2 uvMin, uvMax;
        for (const auto& i : _inst->imgs)
        {
            min1 = clip.min + __pLocal; size1 = clip.size;
            min2 = i.pos; size2 = i.getSize();
            if (twoRectTest(min1, size1, min2, size2))
            {
                uvMin.x = glm::clamp(((float)(min1.x - min2.x) / size2.x), 0.f, 1.f);
                uvMin.y = glm::clamp(((float)(min1.y - min2.y) / size2.y), 0.f, 1.f);
                uvMax.x = glm::clamp(((float)(min1.x + size1.x - min2.x) / size2.x), 0.f, 1.f);
                uvMax.y = glm::clamp(((float)(min1.y + size1.y - min2.y) / size2.y), 0.f, 1.f);
                size = glm::min(glm::min(min2 + size2 - min1, size1), min1 + size1 - min2);
                if (size.x == 0 || size.y == 0) continue;
                dFunc(i.getId(), glm::max(min1, min2) - __pRoot, size, uvMin, uvMax);
            }
        }
        clip.traverseChild([dFunc, __pRoot, __pLocal](const Clip& c, const Clip* p){ draw(c, dFunc, __pRoot, __pLocal + p->min); });
    }

    void ImgCtrl::popBack(bool recy)
    {
        if (_inst->imgs.empty()) return;
        if (recy) FileCtrl::getInst().fileQueue.push_back(_inst->imgs.back().path);
        _inst->imgs.pop_back();
        calc();
    }

    void ImgCtrl::popFront() { _inst->imgs.pop_front(); calc(); }
    void ImgCtrl::traverse(std::function<void(const PicData&, glm::ivec2&, glm::ivec2&)> func)
    {
        for (auto& i : _inst->imgs) func(i, i.pos, i.scale);
        calc();
    }

    bool ImgCtrl::pushBack()
    {
        try
        {
            if (FileCtrl::getInst().fileQueue.empty()) return false;
            _inst->imgs.emplace_back(FileCtrl::getInst().fileQueue.back());
            FileCtrl::getInst().fileQueue.pop_back();
        }
        catch(const std::exception& e)
        {
            return false;
        }
        calc();
        return true;
    }

    const glm::ivec4& ImgCtrl::getAABB() { return _inst->aabb; }
    const glm::vec2& ImgCtrl::getForce() { return _inst->force; }
    void ImgCtrl::clear()
    {
        auto err = vkDeviceWaitIdle(XD::VulkanMgr::getDev());
        XD::VulkanMgr::checkVkResultCType(err);
        return _inst->imgs.clear();
    }
}