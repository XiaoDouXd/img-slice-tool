#pragma once

#include <cmath>
#include <string>

#include "clipCtrlPanel.hpp"
#include "core/clipCtrl.h"
#include "core/fileCtrl.h"
#include "core/imgCtrl.h"
#include "math.h"
#include "unit/imgUnit.h"
#include "xdBase/wndBase.hpp"

#include "imgui_internal.h"

#include "staticPanelData.h"

namespace CC::UI
{
    const ImageUnit::ImageUnitDesc ImageUnit::_emptyImgUnitDesc = {};

    /// @brief 主界面 UI
    class ClipPicPanel : public XD::WndBase<ClipPicPanel>
    {
    private:
        // ------------------- 窗口设置
        ImGuiWindowFlags windowFlags    = 0;
        bool _refreshP0                 = true;
        std::stringstream _ss;

        // ------------------- 图像绘制相关
        bool _isMouseInVp               = false;
        glm::ivec2 _p0                  = {};
        glm::vec2 _focus                = {};
        glm::ivec2 _min                 = {};
        glm::ivec2 _max                 = {};
        glm::ivec2 _aabbRect            = {};
        float _s                        = 1.f;
        float _alphaRectWid             = 16.0f;

        // ------------------- 选择框
        std::vector<uint8_t> _selectedCache = {};
        glm::ivec4 _selectBox           = {};
        bool _selectStart               = true;

        // ------------------- 背景绘制相关
        ImDrawList* _drawList      = nullptr;
        glm::ivec2 _wMax               = {};
        glm::ivec2 _wPos               = {};
        glm::ivec2 _wSize              = {};
        glm::ivec2 _wMousePos          = {};
        glm::ivec2 _cMousePos          = {};
        std::string _posStr            = {};

    protected:
        void onShow(XD::WndDataBaseHolder*) override
        {
            if (ImgCtrl::empty())
            {
                closeSelf();
                return;
            }

            _refreshP0 = true;
            refreshData();
            XD::WndMgr::open<CtrlPanel>();
            registerEvent<XD::StaticEvent::OnShortcut>([this](XD::Shortcut k){this->onShortcut(k);});
        }

        void onRefresh() override
        {
            ctrlScale();
            ctrlChange();
            _drawList->AddText({_wPos.x + 4.f, (float)_wMax.y - 26}, IM_COL32_BLACK, _ss.str().c_str());
            ImGui::PushClipRect({(float)_wPos.x, (float)_wPos.y}, {(float)_wMax.x, (float)_wMax.y - 26}, true);
            drawAlphaBg();
            drawImg();
            drawClips();
            ImGui::PopClipRect();
        }

        bool onWndBegin() override
        {
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
            if (ImGui::Begin("队顶文件", nullptr, windowFlags))
            {
                _wPos = {
                    ImGui::GetWindowContentRegionMin().x + ImGui::GetWindowPos().x,
                    ImGui::GetWindowContentRegionMin().y + ImGui::GetWindowPos().y
                };
                _wSize = {
                    ImGui::GetWindowContentRegionMax().x - ImGui::GetWindowContentRegionMin().x,
                    ImGui::GetWindowContentRegionMax().y - ImGui::GetWindowContentRegionMin().y
                };
                _wMax = {
                    ImGui::GetWindowContentRegionMax().x + ImGui::GetWindowPos().x,
                    ImGui::GetWindowContentRegionMax().y + ImGui::GetWindowPos().y
                };

                _wMousePos = {
                    floor(ImGui::GetMousePos().x - _wPos.x),
                    floor(ImGui::GetMousePos().y - _wPos.y)
                };

                if (_refreshP0)
                {
                    _p0 = {
                        _wSize.x * 0.5f - scale(ImgCtrl::getAABB().z) * 0.5f,
                        _wSize.y * 0.5f - scale(ImgCtrl::getAABB().w) * 0.5f
                    };
                    _s = 1.f;
                    _refreshP0 = false;
                }

                _cMousePos = {
                    floor((_wMousePos.x - _p0.x) / _s),
                    floor((_wMousePos.y - _p0.y) / _s)
                };


                _drawList = ImGui::GetWindowDrawList();

                if (_selectedCache.size() != curSelectedClips.size())
                {
                    _selectedCache.resize(curSelectedClips.size());
                    for (auto& i : _selectedCache) i = false;
                }

                _ss.str(std::string());
                _ss << "| (" << std::to_string(_cMousePos.x) << ", " << std::to_string(_cMousePos.y) << ") | ";
                _ss << std::fixed << std::setprecision(2) << _s * 100.f;
                _ss << "%";

                return true;
            }
            return false;
        }

        void onWndEnd() override
        {
            ImGui::End();
            ImGui::PopStyleVar();
        }

    private:
        // ---------------------------------------------------------------- 控制
        void ctrlScale()
        {
            _isMouseInVp = _wMousePos.x >= 0 &&
                _wMousePos.x <= _wSize.x &&
                _wMousePos.y >= 0 &&
                _wMousePos.y <= _wSize.y;
            if (_isMouseInVp && !ImGui::IsWindowDocked()) windowFlags |= ImGuiWindowFlags_NoMove;
            else windowFlags &= !ImGuiWindowFlags_NoMove;
            if (!ImGui::IsWindowFocused()) return;

            if (rectTest({ImGui::GetMousePos().x, ImGui::GetMousePos().y}, _wPos, _wMax))
            {
                if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left) && !XD::ImguiMgr::getIO().KeyAlt)
                {
                    for (auto& i : _selectedCache) i = false;
                    curSelectedClipsReset = true;
                }
                if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Middle))
                    _s = 1.f;

                if (XD::ImguiMgr::getIO().KeyCtrl && ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !XD::ImguiMgr::getIO().KeyAlt)
                {
                    for (size_t i = 0; i < ClipCtrl::getCurClips().size(); i++)
                    {
                        if (i >= curSelectedClips.size()) break;
                        if (!ClipCtrl::getCurClips()[i].empty && ClipCtrl::getCurClips()[i].test(_cMousePos))
                            curSelectedClips[i] = !curSelectedClips[i];
                    }
                }
                if (XD::ImguiMgr::getIO().KeyCtrl && !XD::ImguiMgr::getIO().KeyAlt && ImGui::IsMouseDragging(ImGuiMouseButton_Left))
                {
                    if (!_selectStart) {_selectBox.x = _cMousePos.x; _selectBox.y = _cMousePos.y;}
                    _selectBox.z = _cMousePos.x; _selectBox.w = _cMousePos.y;
                    _selectStart = true;
                }
                else if (_selectStart)
                {
                    for (size_t i = 0; i < _selectedCache.size(); i++)
                        if (i < curSelectedClips.size() && !ClipCtrl::getCurClips()[i].empty)
                        {
                            curSelectedClips[i] |= _selectedCache[i];
                            _selectedCache[i] = false;
                        }
                    _selectStart = false;
                }

                if (_selectStart)
                {
                    auto rMin = glm::ivec2(glm::min(_selectBox.x, _selectBox.z), glm::min(_selectBox.y, _selectBox.w));
                    auto rSize = glm::abs(glm::ivec2(_selectBox.z - _selectBox.x, _selectBox.w - _selectBox.y));
                    for (size_t i = 0; i < ClipCtrl::getCurClips().size(); i++)
                    {
                        if (i >= _selectedCache.size()) break;
                        _selectedCache[i] = ClipCtrl::getCurClips()[i].test(rMin, rSize);
                    }
                }

                if (XD::ImguiMgr::getIO().MouseWheel != 0)
                {
                    auto _sOld = _s;
                    _s += ImSign(XD::ImguiMgr::getIO().MouseWheel) * (_s * 0.1f);
                    if (_s <= 0.05f) _s = 0.05f;
                    if (_s > 36.f) _s = 36.f;

                    if (_isMouseInVp)
                    {
                        _p0.x += (_cMousePos.x * _sOld - _cMousePos.x * _s) / 2;
                        _p0.y += (_cMousePos.y * _sOld - _cMousePos.y * _s) / 2;
                    }
                    else
                    {
                        _p0.x += (_focus.x * _sOld - _focus.x * _s) / 2;
                        _p0.y += (_focus.y * _sOld - _focus.y * _s) / 2;
                    }
                }
            }

            if (_isMouseInVp && !XD::ImguiMgr::getIO().KeyAlt)
            {
                if (!XD::ImguiMgr::getIO().KeyCtrl &&
                    ImGui::IsMouseDragging(ImGuiMouseButton_Left) &&
                    !ImGui::IsWindowCollapsed())
                {
                    auto dragDelta = ImGui::GetMouseDragDelta(ImGuiMouseButton_Left);
                    _p0.x += XD::ImguiMgr::getIO().MouseDelta.x;
                    _p0.y += XD::ImguiMgr::getIO().MouseDelta.y;

                }
            }
            if (_p0.x < -2 * _aabbRect.x * _s) _p0.x = -2 * _aabbRect.x * _s;
            if (_p0.y < -2 * _aabbRect.y * _s) _p0.y = -2 * _aabbRect.y * _s;
            if (_p0.x > _aabbRect.x * _s + _wSize.x) _p0.x = _aabbRect.x * _s + _wSize.x;
            if (_p0.y > _aabbRect.y * _s + _wSize.y) _p0.y = _aabbRect.y * _s + _wSize.y;
        }

        bool _changeData = false;
        bool _dragOuter = false;
        bool _isSelectedEmpty = true;
        bool _startDrag = false;
        glm::ivec2 _changeDDcache;
        glm::ivec2 _changeFrameP0;
        glm::ivec2 _changeFrameSize;
        glm::ivec2 _changeOffset;
        glm::vec2 _changeScale = {1.f, 1.f};
        void ctrlChange()
        {
            if (!ImGui::IsWindowFocused() || !XD::ImguiMgr::getIO().KeyAlt || _dragOuter)
            {
                if (_changeData && (_changeOffset != glm::ivec2() || _changeScale != glm::vec2(1.f, 1.f)))
                {
                    auto l = std::list<std::pair<size_t, Clip>>();
                    glm::ivec2 scaleOffset, scaleSize;
                    for (size_t i = 0; i < curSelectedClips.size(); i++)
                    if (curSelectedClips[i] && i < ClipCtrl::getCurClips().size())
                    {
                        auto c = ClipCtrl::getCurClips()[i];
                        scaleOffset.x = glm::floor((c.min - _changeFrameP0).x * _changeScale.x);
                        scaleOffset.y = glm::floor((c.min - _changeFrameP0).y * _changeScale.y);
                        scaleSize.x = glm::floor(c.size.x * _changeScale.x);
                        scaleSize.y = glm::floor(c.size.y * _changeScale.y);
                        c.min = scaleOffset + _changeFrameP0 + _changeOffset;
                        c.size = scaleSize;
                        l.push_back({i, c});
                    }
                    ClipCtrl::set(l);
                    _changeOffset = {};
                }
                if (ImGui::IsMouseDragging(ImGuiMouseButton_Left)) _dragOuter = true;
                else _dragOuter = false;
                _changeData = false;
                return;
            }

            // 初始化数据
            if (!_changeData)
            {
                _changeFrameP0 = {INT_MAX, INT_MAX};
                _changeFrameSize = {INT_MIN, INT_MIN};
                _changeOffset = {};
                _changeScale = {1.f, 1.f};
                _isSelectedEmpty = true;
                for (size_t i = 0; i < curSelectedClips.size(); i++)
                {
                    if (!curSelectedClips[i] || i >= ClipCtrl::getCurClips().size()) continue;
                    _isSelectedEmpty = false;
                    auto aabb = ClipCtrl::getCurClips()[i].getAABB();
                    _changeFrameP0 = glm::min(glm::ivec2(aabb.x, aabb.y), _changeFrameP0);
                    _changeFrameSize = glm::max(
                        glm::ivec2(aabb.z, aabb.w),
                        _changeFrameSize);
                }
                if (!_isSelectedEmpty) _changeFrameSize -= _changeFrameP0;
            }
            _changeData = true;
            if (ImGui::IsKeyPressed(ImGuiKey_LeftArrow))
                _changeOffset -= glm::ivec2(1, 0);
            if (ImGui::IsKeyPressed(ImGuiKey_RightArrow))
                _changeOffset += glm::ivec2(1, 0);
            if (ImGui::IsKeyPressed(ImGuiKey_UpArrow))
                _changeOffset -= glm::ivec2(0, 1);
            if (ImGui::IsKeyPressed(ImGuiKey_DownArrow))
                _changeOffset += glm::ivec2(0, 1);

            if (ImGui::IsMouseDragging(ImGuiMouseButton_Left))
            {
                if (!_startDrag) _changeDDcache = _changeOffset;
                _startDrag = true;

                auto dd = ImGui::GetMouseDragDelta(ImGuiMouseButton_Left);
                _changeOffset = glm::ivec2(dd.x / _s, dd.y / _s) + _changeDDcache;
            }
            else _startDrag = false;

            _ss << " |+(" << _changeOffset.x << ", " << _changeOffset.y << ") = ("
                << _changeFrameP0.x + _changeOffset.x << ", " << _changeFrameP0.y + _changeOffset.y << ")";
        }

        void onShortcut(XD::Shortcut k)
        {
            switch (k)
            {
            case XD::Shortcut::Undo: ClipCtrl::undo();
                curSelectedClipsReset = true;
                return;
            case XD::Shortcut::Redo: ClipCtrl::redo();
                curSelectedClipsReset = true;
                return;
            default: return;
            }
        }

        // ---------------------------------------------------------------- 绘制
        /// @brief 绘制 image
        void drawImg()
        {
            ImgCtrl::draw([this](const ImTextureID& id, const glm::ivec2& pos, const glm::ivec2& size){
                this->addPic(id, pos, size);
            });
        }

        void drawClips()
        {
            const auto& clips = ClipCtrl::getCurClips();
            for (auto i = size_t(0); i < clips.size(); i++)
                addClip(i);
            if (_selectStart)
                addRect({_selectBox.x, _selectBox.y}, {_selectBox.z - _selectBox.x, _selectBox.w - _selectBox.y}, IM_COL32_BLACK);
            if (!_isSelectedEmpty && _changeData)
                addRect(_changeFrameP0 + _changeOffset, _changeFrameSize, IM_COL32_BLACK);

            if (gridCreatePreviewShown)
            {
                for (size_t i = 0; i < gridCreatePreviewLineCount.x; i++)
                for (size_t j = 0; j < gridCreatePreviewLineCount.y; j++)
                {
                    addRect(
                        gridCreatePreviewP0 + glm::ivec2(i * gridCreatePreviewSize.x, j * gridCreatePreviewSize.y),
                        gridCreatePreviewSize,
                        ImColor(1.f, 1.f, 0.f));
                }
            }
        }

        void drawAlphaBg()
        {
            auto rectWid_scale = scale(_alphaRectWid);
            while (rectWid_scale < 8.f) rectWid_scale *= 2;

            for (double i = _p0.x; i < rectWid_scale + _wSize.x; i += rectWid_scale * 2)
            {
                for (double j = _p0.y; j < rectWid_scale + _wSize.y; j += rectWid_scale * 2)
                    addAlphaRect({floor(i), floor(j)}, rectWid_scale);
                for (double j = _p0.y - 2*rectWid_scale; j > -rectWid_scale; j -= rectWid_scale * 2)
                    addAlphaRect({floor(i), floor(j)}, rectWid_scale);
            }
            for (double i = _p0.x - 2*rectWid_scale; i > -rectWid_scale; i -= rectWid_scale * 2)
            {
                for (double j = _p0.y; j < rectWid_scale + _wSize.y; j += rectWid_scale * 2)
                    addAlphaRect({floor(i), floor(j)}, rectWid_scale);
                for (double j = _p0.y - 2*rectWid_scale; j > -rectWid_scale; j -= rectWid_scale * 2)
                    addAlphaRect({floor(i), floor(j)}, rectWid_scale);
            }
        }

        // ---------------------------------------------------------------- 绘制 unit
        void addPic(ImTextureID imgId, glm::ivec2 pos, glm::ivec2 size)
        {
            addImg(imgId, pos, size);
            addRect(pos, size, IM_COL32_BLACK);
        }

        void addImg(const ImTextureID& imgId, const glm::ivec2& pos, const glm::ivec2& size)
        {
            _drawList->AddImage(imgId,
                {_wPos.x + _p0.x + scale(pos.x), _wPos.y + _p0.y + scale(pos.y)},
                {_wPos.x + _p0.x + scale(size.x + pos.x), _wPos.y + _p0.y + scale(size.y + pos.y)});
        }

        void addClip(size_t idx)
        {
            const auto& clip = ClipCtrl::getCurClips()[idx];

            if (clip.empty) return;
                clip.traverseWithOffset([this, idx](const Clip& cClip, const glm::ivec2& pOffset){
                    if (idx == curHoveredClip)
                            this->addRectFilled(cClip.min + pOffset, cClip.size, ImColor(0.f, 1.f, 1.f, 0.4f));
                    if (idx == curDragedClip)
                        this->addRectFilled(cClip.min + pOffset, cClip.size, ImColor(1.f, 0.f, 0.f, 0.2f));
                    if ((idx < curSelectedClips.size() && curSelectedClips[idx]) ||
                        (idx < _selectedCache.size() && _selectedCache[idx]))
                    {
                        this->addRect(cClip.min + pOffset + this->_changeOffset, cClip.size, ImColor(1.f, 0.f, 0.f, 1.f));
                        this->addRectFilled(cClip.min + pOffset + this->_changeOffset, cClip.size, ImColor(1.f, 1.f, 0.f, 0.4f));
                    }
                    else
                        this->addRect(cClip.min + pOffset, cClip.size, ImColor(0.f, 0.f, 1.f, 1.f));
                });
        }

        void addRect(glm::ivec2 pos, glm::ivec2 size, ImColor col)
        {
            _drawList->AddRect(
                {_wPos.x + _p0.x + scale(pos.x), _wPos.y + _p0.y + scale(pos.y)},
                {_wPos.x + _p0.x + scale(size.x + pos.x), _wPos.y + _p0.y + scale(size.y + pos.y)},
                col);
        }

        void addRectFilled(glm::ivec2 pos, glm::ivec2 size, ImColor col)
        {
            _drawList->AddRectFilled(
                {_wPos.x + _p0.x + scale(pos.x), _wPos.y + _p0.y + scale(pos.y)},
                {_wPos.x + _p0.x + scale(size.x + pos.x), _wPos.y + _p0.y + scale(size.y + pos.y)},
                col);
        }

        void addAlphaRect(glm::ivec2 pos, float size)
        {
            _drawList->AddRectFilled(
                {(float)_wPos.x + pos.x, (float)_wPos.y + pos.y},
                {_wPos.x + size + pos.x, _wPos.y + size + pos.y},
                ImColor(0.f, 0.f, 0.f, 0.25f));
            _drawList->AddRectFilled(
                {_wPos.x + pos.x - size, _wPos.y + pos.y - size},
                {(float)_wPos.x  + pos.x, (float)_wPos.y + pos.y},
                ImColor(0.f, 0.f, 0.f, 0.25f));
        }

        // ---------------------------------------------------------------- 计算
        float scale(float src)
        {
            return src * _s;
        }

        void refreshData()
        {
            glm::ivec2 mi = {ImgCtrl::getAABB().x, ImgCtrl::getAABB().y};
            glm::ivec2 ma = {ImgCtrl::getAABB().x + ImgCtrl::getAABB().z, ImgCtrl::getAABB().y + ImgCtrl::getAABB().w};
            _focus = ImgCtrl::getForce();
            _min = mi;
            _max = ma;
            _aabbRect.x = _max.x - _min.x;
            _aabbRect.y = _max.y - _min.y;
        }
    };
}