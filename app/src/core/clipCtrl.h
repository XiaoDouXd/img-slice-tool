#pragma once

#include <vector>

#include "prstData.hpp"
#include "ui/util/rectTest.hpp"
#include "xdBase.h"

#include "glm.hpp"
#include "imgui_internal.h"

namespace CC
{
    /// @brief 一个切片
    struct Clip
    {
    public:
        Clip() : empty(true) {}
        Clip(glm::ivec2 min, glm::ivec2 size) : min(min), size(size), empty(false) {}
        Clip(glm::ivec4 rect) : min(rect.x, rect.y), size(glm::abs(rect.z - rect.x), glm::abs(rect.w - rect.y)), empty(false) {}
        Clip(ImVec2 min, ImVec2 size) : min(min.x, min.y), size(size.x, size.y), empty(false) {}
        Clip(const Clip& o) : min(o.min), size(o.size), empty(o.empty), mergedRects(o.mergedRects) {}
        Clip(Clip&& o) : min(o.min), size(o.size), empty(o.empty), mergedRects(std::move(o.mergedRects)) {}

        const Clip& operator=(const Clip& o)
        {
            min = o.min;
            size = o.size;
            empty = o.empty;
            mergedRects = o.mergedRects;
            return *this;
        }
        const Clip& operator=(Clip&& o)
        {
            min = o.min;
            size = o.size;
            empty = o.empty;
            mergedRects.swap(o.mergedRects);
            return *this;
        }
        operator std::string() const
        {
            if (empty) return "空";
            if (mergedRects.empty())
                return  "(" + std::to_string((int)min.x) + ", " + std::to_string((int)min.y) + ")|" +
                        "(" + std::to_string((int)size.x) + ", " + std::to_string((int)size.y) + ")";
            return "复合节点";
        }

        glm::ivec4 getAABB() const
        {
            glm::ivec2 oMin = {}, oMax = size;
            for (const auto& c : mergedRects)
            {
                const auto& cAABB = c.getAABB();
                oMin = glm::min(oMin, glm::ivec2(cAABB.x, cAABB.y));
                oMax = glm::max(oMax, glm::ivec2(cAABB.z, cAABB.w));
            }
            return {oMin.x + min.x, oMin.y + min.y, oMax.x + min.x, oMax.y + min.y};
        }

        /// @brief 遍历
        /// @param func void(selfAndChildren)
        void traverse(std::function<void(const Clip&)> func) const
        {
            func(*this);
            for (const auto& c : mergedRects) c.traverse(func);
        }

        /// @brief 遍历
        /// @param func void(children, parent)
        /// @param parent 父节点
        void traverse(std::function<void(const Clip&, const Clip*)> func, const Clip* parent = &defaultClip) const
        {
            func(*this, parent);
            for (const auto& c : mergedRects) c.traverse(func, this);
        }

        /// @brief 遍历孩子
        /// @param func void(children)
        void traverseChild(std::function<void(const Clip&)> func) const
        {
            for (const auto& c : mergedRects) c.traverse(func);
        }

        /// @brief 遍历孩子
        /// @param func void(children)
        void traverseChild(std::function<void(const Clip&, const Clip*)> func) const
        {
            for (const auto& c : mergedRects) c.traverse(func, this);
        }

        /// @brief 遍历
        /// @param func void(selfAndChildren)
        void traverseWithOffset(std::function<void(const Clip&, const glm::ivec2&)> func, const glm::ivec2& offset = {}) const
        {
            func(*this, offset);
            for (const auto& c : mergedRects) c.traverseWithOffset(func, offset + min);
        }

        bool test(glm::ivec2 pos) const
        {
            if (pos.x >= min.x && pos.y >= min.y &&
                pos.x < min.x + size.x && pos.y < min.y + size.y)
                return true;
            else for (const auto& i : mergedRects)
                if (i.test(pos - min)) return true;
            return false;
        }

        bool test(glm::ivec2 rMin, glm::ivec2 rSize) const
        {
            if (twoRectTest(min, size, rMin, rSize))
                return true;
            else for (const auto& i : mergedRects)
                if (i.test(rMin - min, rSize)) return true;
            return false;
        }

        bool empty;
        glm::ivec2 min;
        glm::ivec2 size;
        std::list<Clip> mergedRects;

        static const Clip defaultClip;
    };

    class ClipCtrl
    {
    private:
        class ClipPrstCtrl;

        struct ClipPrstData
        {
            friend ClipCtrl::ClipPrstCtrl;
        public:
            ClipPrstData(ClipPrstData&& d) : dObjs(std::move(d.dObjs)) {}
            const ClipPrstData& operator =(ClipPrstData&& d) { dObjs.swap(d.dObjs); return *this; }

            std::list<std::pair<size_t, Clip>> dObjs;
        private:
            ClipPrstData(std::list<std::pair<size_t, Clip>>&& dDatas) : dObjs(dDatas) {}
            ClipPrstData() {}
        };

        struct ClipDoData
        {
        public:
            std::list<std::pair<size_t, Clip>> dObjs;
        };

        class ClipPrstCtrl : public PrstCtrl<ClipPrstData, ClipDoData>
        {
            void undoAction(Op op, const ClipPrstData& data) override
            {
                if (data.dObjs.empty()) return;
                switch (op)
                {
                case Op::PushClip: // 对推进数据的要求, 只能推到最后一位
                    {
                        auto count = data.dObjs.size();
                        while (count)
                        {
                            _inst->clips.pop_back();
                            count--;
                        }
                    }
                    break;
                case Op::ChangeClip: // 链表第一个储存过去的数值, 链表最后一个储存全新的数值
                    {
                        size_t count = data.dObjs.size() >> 1;
                        auto itr = data.dObjs.begin();
                        while(count)
                        {
                            _inst->clips[itr->first] = itr->second;
                            itr++;
                            count--;
                        }
                    }
                    break;
                case Op::SwapClip: // 仅储存交换双方的 id
                    {
                        auto tempVar = _inst->clips[data.dObjs.front().first];
                        _inst->clips[data.dObjs.front().first] = _inst->clips[data.dObjs.back().first];
                        _inst->clips[data.dObjs.back().first] = tempVar;
                    }
                    break;
                case Op::EraseClip: // 对擦除数据的要求: 只能擦除非空的数据, 擦除数据本质上只是把 empty 字段改成 true
                    for (auto& i : data.dObjs)
                        _inst->clips[i.first].empty = false;
                    break;
                case Op::DeleteClip: // 对删除数据的要求: 删除的数据记录的是删之前的原始 idx, 且 idx 在链表中从小到大排列
                    for (auto& i : data.dObjs)
                        _inst->clips.insert(_inst->clips.begin() + i.first, i.second);
                    break;
                case Op::MergeClip: // 对合并数据的要求: 不能合并空数据, 链表中的项仅记录合并前的 idx (第一项为合并后到目标对象的 idx)
                    {
                        auto itr = ++data.dObjs.cbegin();
                        while(itr != data.dObjs.cend())
                        {
                            _inst->clips[itr->first].empty = false;
                            _inst->clips[itr->first].min += _inst->clips[data.dObjs.front().first].min;
                            _inst->clips[data.dObjs.front().first].mergedRects.pop_back();
                            itr++;
                        }
                    }
                    break;
                case Op::DevidClip: // 链表中第一个数据为 id, 最后一个数据为分裂出了多少个文件 -1
                    {
                        auto idx = data.dObjs.front().first;
                        auto childCount = data.dObjs.back().first;
                        while(childCount)
                        {
                            _inst->clips.back().min -= _inst->clips[idx].min;
                            _inst->clips.back().empty = false;
                            _inst->clips[idx].mergedRects.push_back(_inst->clips.back());
                            _inst->clips.pop_back();
                            childCount--;
                        }
                    }
                    break;
                default: break;
                }
            }

            ClipPrstData todoPreT(Op op, ClipDoData&& data) override
            {
                return ClipPrstData(std::move(data.dObjs));
            }

            void redoAction(Op op, const ClipPrstData& data) override
            {
                switch (op)
                {
                case Op::PushClip:
                    for (auto& i : data.dObjs)
                        _inst->clips.emplace_back(i.second);
                    break;
                case Op::ChangeClip:
                    {
                        size_t count = data.dObjs.size() >> 1;
                        auto itr = --data.dObjs.end();
                        while(count)
                        {
                            _inst->clips[itr->first] = itr->second;
                            itr--;
                            count--;
                        }
                    }
                    break;
                case Op::SwapClip:
                    {
                        auto tempVar = _inst->clips[data.dObjs.front().first];
                        _inst->clips[data.dObjs.front().first] = _inst->clips[data.dObjs.back().first];
                        _inst->clips[data.dObjs.back().first] = tempVar;
                    }
                    break;
                case Op::EraseClip:
                    for (auto& i : data.dObjs)
                        _inst->clips[i.first].empty = true;
                    break;
                case Op::DeleteClip:
                    for (auto i = --data.dObjs.end(); true;i--)
                    {
                        _inst->clips.erase(_inst->clips.begin() + i->first);
                        if (i == data.dObjs.cbegin()) break;
                    }
                    break;
                case Op::MergeClip:
                    {
                        auto itr = ++data.dObjs.cbegin();
                        while(itr != data.dObjs.cend())
                        {
                            _inst->clips[itr->first].min -= _inst->clips[data.dObjs.front().first].min;
                            _inst->clips[data.dObjs.front().first].mergedRects
                                .emplace_back(_inst->clips[itr->first]);
                            _inst->clips[itr->first].empty = true;
                            itr++;
                        }
                    }
                    break;
                case Op::DevidClip:
                    {
                        auto idx = data.dObjs.front().first;
                        auto childCount = data.dObjs.back().first;
                        while(childCount)
                        {
                            _inst->clips[idx].mergedRects.back().min += _inst->clips[idx].min;
                            _inst->clips.emplace_back(_inst->clips[idx].mergedRects.back());
                            _inst->clips[idx].mergedRects.pop_back();
                            childCount--;
                        }
                    }
                    break;
                default: break;
                }
            }

            bool opFilter(Op op) { return (int)op < (int)Op::PushClip || (int)op > (int)Op::DevidClip; }
        };

        class ClipCtrlData
        {
        public:
            std::vector<Clip> clips;    // 所有切片(包含空切片)
            ClipPrstCtrl prstInst;      // 持久化控制器
        };
        static std::unique_ptr<ClipCtrlData> _inst;

    public:
        static void init();
        static void push(const std::list<Clip>& clips);
        static void del(const std::list<size_t>& idx);
        static void swap(size_t a, size_t b);
        static void erase(const std::list<size_t>& idx);
        static void merge(const std::list<size_t>& idx);
        static void set(const std::list<std::pair<size_t, Clip>>& clips);
        static void devid(size_t idx);
        static void undo();
        static void redo();
        static bool isCanUndo();
        static bool isCanRedo();
        static const std::vector<Clip>& getCurClips();
    };
}