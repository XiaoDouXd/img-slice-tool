#pragma once

#include <cmath>

#include "misc/cpp/imgui_stdlib.h"

#include "core/clipCtrl.h"
#include "core/imgCtrl.h"
#include "util/rectTest.hpp"
#include "xdBase.h"
#include "xdBase/wndBase.hpp"

#include "staticPanelData.h"

namespace CC::UI
{
    const char* gSizeLabel = "- 网格宽高";
    const char* gOffsetLabel = "- 网格偏移";
    const char* gCountLabel = "- 网格行列数量";
    const char* createGrid = "建立网格";
    const char* createGridOK = "确定";
    const ImGuiWindowFlags popFlag =    ImGuiWindowFlags_NoCollapse |
                                        ImGuiWindowFlags_NoDocking |
                                        ImGuiWindowFlags_AlwaysAutoResize |
                                        ImGuiWindowFlags_NoScrollbar |
                                        ImGuiWindowFlags_NoResize;

class CtrlPanel : public XD::App::WndBase<CtrlPanel>
    {
    private:
        // ------------------- 窗口设置
        ImGuiWindowFlags windowFlags    = 0;
        ImVec4 clearColor               = ImVec4(0.22f, 0.22f, 0.22f, 1.00f);

    protected:
        void onShow(XD::App::WndDataBaseHolder*) override
        {
            windowFlags |= ImGuiWindowFlags_NoCollapse;
        };

        void onRefresh() override
        {
            const auto& clips = ClipCtrl::getCurClips();
            if (curSelectedClips.size() != clips.size())
                curSelectedClipsReset = true;
            if (curSelectedClipsReset)
            {
                curSelectedClips.resize(clips.size());
                for (auto& i : curSelectedClips) i = false;
                curSelectedClipsReset = false;
            }

            if (ImGui::BeginTabItem("生成"))
            {
                if (ImGui::Button(createGrid))
                    _gridPop_createGridPopShow = true;

                ImGui::Text("程序刷新率 %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
                ImGui::EndTabItem();
            }
            clipList();
            ImGui::EndTabBar();

            // ------------------------- 一些弹窗
            createGridPop();
            delEmptyPop();
        }

        bool onWndBegin() override
        {
            if (ImGui::Begin("切片", nullptr, windowFlags))
            {
                ImGui::BeginTabBar("MyTabBar", ImGuiTabBarFlags_None);
                return true;
            }
            return false;
        }

        void onWndEnd() override
        {
            ImGui::End();
        }
    private:
        // ------------------- 网格窗口
        int _gridPop_rectOffset[2]               = {0, 0};
        int _gridPop_rectWidth[2]                = {32, 32};
        int _gridPop_rectCount[2]                = {2, 2};
        bool _gridPop_createGridPopShow              = false;
        void createGridPop()
        {
            gridCreatePreviewShown = _gridPop_createGridPopShow;
            if (!_gridPop_createGridPopShow) return;
            ImGui::Begin("创建网格切片", &_gridPop_createGridPopShow, popFlag);

            ImGui::InputInt2(gSizeLabel, _gridPop_rectWidth, ImGuiInputTextFlags_CharsDecimal);
            ImGui::InputInt2(gOffsetLabel, _gridPop_rectOffset);
            ImGui::InputInt2(gCountLabel, _gridPop_rectCount);

            gridCreatePreviewP0 = glm::ivec2(_gridPop_rectOffset[0], _gridPop_rectOffset[1]);
            gridCreatePreviewLineCount = glm::ivec2(_gridPop_rectCount[0], _gridPop_rectCount[1]);
            gridCreatePreviewSize = glm::ivec2(_gridPop_rectWidth[0], _gridPop_rectWidth[1]);

            if (ImGui::Button(createGridOK))
            {
                if (_gridPop_rectCount[0]  <= 0 || _gridPop_rectCount[1] <= 0)
                {
                    _gridPop_rectCount[0] = 0;
                    _gridPop_rectCount[1] = 0;
                }
                _gridPop_createGridPopShow = false;
                auto rects = std::list<Clip>();
                auto curIdx = 0;
                for (auto i = 0; i < _gridPop_rectCount[0]; i++)
                {
                    for (auto j = 0; j < _gridPop_rectCount[1]; j++)
                    {
                        rects.emplace_back(Clip(
                            glm::ivec2((float)(i * _gridPop_rectWidth[0] + _gridPop_rectOffset[0]), (float)(j * _gridPop_rectWidth[1] + _gridPop_rectOffset[1])),
                            glm::ivec2((float)_gridPop_rectWidth[0], (float)_gridPop_rectWidth[1])));
                    }
                }
                ClipCtrl::push(rects);
            }

            ImGui::End();
        }

        // -------------------------------------------- 切片列表
        bool _clipList_startDrag                        = false;
        bool _clipList_startDragAndListOperationEnabled = false;
        bool _clipList_enableListOperation              = false;
        int _clipList_dragStart                         = 0;
        int _clipList_selectItem_gridList               = 0;
        std::string _clipList_clipName;
        void clipList()
        {
            auto h = ImGui::GetWindowHeight();
            if (ImGui::BeginTabItem("遴选"))
            {
                const auto& clips = ClipCtrl::getCurClips();
                ImGui::BeginListBox("切片", ImVec2(-FLT_MIN, h - 120));
                auto i = size_t(0);

                curHoveredClip = -1;
                ImGuiListClipper clipper;
                clipper.Begin(clips.size(), ImGui::GetTextLineHeightWithSpacing());
                while (clipper.Step())
                for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++)
                {
                    ImGui::PushID(i);
                    _clipList_clipName = "        [" + std::to_string(i) + "] " + (std::string)clips[i];
                    if (ImGui::Selectable(_clipList_clipName.c_str(), curSelectedClips[i]))
                    {
                        curSelectedClips[i] = !curSelectedClips[i];
                    }
                    if (rectTest(ImGui::GetMousePos(), ImGui::GetItemRectMin(), ImGui::GetItemRectMax()))
                    {
                        curHoveredClip = i;
                        if (ImGui::IsWindowFocused() && _clipList_enableListOperation)
                        {
                            _clipList_selectItem_gridList = i;
                            if (_clipList_startDragAndListOperationEnabled)
                            {
                                ImGui::GetWindowDrawList()->AddRectFilled(
                                    ImGui::GetItemRectMin(),
                                    ImGui::GetItemRectMax(),
                                    ImColor(1.f, 0.2f, 0.f, 0.25f));
                            }
                        }
                    }
                    drawPicIcon(clips[i]);
                    if (_clipList_startDragAndListOperationEnabled && i == _clipList_dragStart)
                    {
                        ImGui::GetWindowDrawList()->AddRectFilled(
                            ImGui::GetItemRectMin(),
                            ImGui::GetItemRectMax(),
                            ImColor(0.f, 0.2f, 1.f, 0.25f));
                    }

                    if (ImGui::IsWindowFocused())
                    {
                        if (ImGui::IsMouseDragging(ImGuiMouseButton_Left))
                        {
                            if (!_clipList_startDrag)
                            {
                                _clipList_startDragAndListOperationEnabled = _clipList_enableListOperation;
                                _clipList_startDrag = true;
                                if (_clipList_startDragAndListOperationEnabled)
                                    for (auto& i : curSelectedClips) i = false;
                                if (_clipList_selectItem_gridList >= 0 && _clipList_selectItem_gridList < clips.size())
                                    _clipList_dragStart = _clipList_selectItem_gridList;
                                curDragedClip = _clipList_dragStart;
                            }
                        }
                        else if (_clipList_startDrag)
                        {
                            if (_clipList_startDragAndListOperationEnabled)
                            {
                                int n_next = _clipList_selectItem_gridList;
                                if (n_next != _clipList_dragStart && n_next >= 0 && n_next < clips.size())
                                    ClipCtrl::swap(_clipList_dragStart, n_next);
                            }
                            curSelectedClipsReset = _clipList_startDragAndListOperationEnabled;
                            _clipList_startDrag = false;
                            _clipList_startDragAndListOperationEnabled = false;
                            curDragedClip = -1;
                        }
                    }
                    ImGui::Separator();
                    ImGui::PopID();
                }

                ImGui::EndListBox();
                if (rectTest(ImGui::GetMousePos(),
                    ImGui::GetItemRectMin(),
                    {ImGui::GetItemRectMax().x - 65, ImGui::GetItemRectMax().y}))
                    _clipList_enableListOperation = true;
                else
                {
                    _clipList_startDrag = false;
                    _clipList_startDragAndListOperationEnabled = false;
                    _clipList_enableListOperation = false;
                    _clipList_dragStart           = 0;
                    _clipList_selectItem_gridList = 0;
                    curDragedClip = -1;
                }
                if (ImGui::Button("合"))
                {
                    auto cs = std::list<size_t>();
                    for (auto i = size_t(0); i < curSelectedClips.size(); i++)
                        if (curSelectedClips[i])
                            cs.push_back(i);
                    ClipCtrl::merge(cs);
                    curSelectedClipsReset = true;
                }
                ImGui::SameLine();
                if (ImGui::Button("分"))
                {
                    for (auto i = size_t(0); i < curSelectedClips.size(); i++)
                        if (curSelectedClips[i])
                            ClipCtrl::devid(i);
                    curSelectedClipsReset = true;
                }
                ImGui::SameLine();
                if (ImGui::Button("清"))
                {
                    auto cs = std::list<size_t>();
                    for (auto i = size_t(0); i < curSelectedClips.size(); i++)
                        if (curSelectedClips[i])
                            cs.push_back(i);
                    ClipCtrl::erase(cs);
                    curSelectedClipsReset = true;
                }
                ImGui::SameLine();
                if (ImGui::Button("删"))
                {
                    auto cs = std::list<size_t>();
                    for (auto i = size_t(0); i < curSelectedClips.size(); i++)
                        if (curSelectedClips[i])
                            cs.push_back(i);
                    ClipCtrl::del(cs);
                    curSelectedClipsReset = true;
                }
                ImGui::SameLine();
                if (ImGui::Button("删除空位"))
                {
                    _delEmptyPop_show = true;
                }
                ImGui::EndTabItem();
            }
        }

        // -------------------------------------------- 缩略图
        void drawPicIcon(const Clip& clip)
        {
            auto dl = ImGui::GetWindowDrawList();
            auto minP = ImGui::GetItemRectMin();
            auto aabb = clip.getAABB();
            aabb.z = aabb.z - aabb.x;
            aabb.w = aabb.w - aabb.y;
            float sc = glm::max(aabb.z, aabb.w);
            if (sc <= 0.00000000001f) return;

            ImgCtrl::draw(clip, [this, dl, minP, sc]
                (const ImTextureID& id, const glm::ivec2& p, const glm::ivec2& s, const glm::ivec2& uvMin, const glm::ivec2& uvMax)
            {
                ImVec2 p0 = {minP.x + (p.x / sc) * 26 + 4, minP.y + (p.y / sc) * 26 + 4};
                dl->AddImage(id, p0, {p0.x + (s.x / sc) * 26, p0.y + (s.y / sc) * 26},
                    {(float)uvMin.x, (float)uvMin.y}, {(float)uvMax.x, (float)uvMax.y});
            });
            dl->AddRect({minP.x + 4, minP.y + 4}, {minP.x + 30, minP.y + 30},
                IM_COL32_BLACK);
        }

        // -------------------------------------------- 询问弹窗
        bool _delEmptyPop_show                          = false;
        void delEmptyPop()
        {
            if (!_delEmptyPop_show) return;
            if (ImGui::Begin("删除空位", &_delEmptyPop_show, popFlag))
            {
                ImGui::Text("注意: 删除空位会导致编号大量改变!");
                ImGui::Text("确定执行?");
                if (ImGui::Button("确定"))
                {
                    auto cs = std::list<size_t>();
                    for (auto i = size_t(0); i < ClipCtrl::getCurClips().size(); i++)
                        if (ClipCtrl::getCurClips()[i].empty)
                            cs.push_back(i);
                    ClipCtrl::del(cs);
                    curSelectedClipsReset = true;
                    _delEmptyPop_show = false;
                }
                ImGui::SameLine();
                if (ImGui::Button("取消"))
                    _delEmptyPop_show = false;
            }
            ImGui::End();
        }
    };

}