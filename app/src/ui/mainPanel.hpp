#pragma once

#include <list>
#include <sstream>

#include "xdBase.h"
#include "imgui_internal.h"

#include "clipCtrlPanel.hpp"
#include "clipPicPanel.hpp"
#include "core/imgCtrl.h"
#include "xdBase/wndBase.hpp"

#include "staticPanelData.h"

namespace CC::UI
{
    using namespace XD;

    static size_t picPanelId = 0;
    static void openPicPanel()
    {
        if (ImgCtrl::empty()) ImgCtrl::pushBack();
        if (picPanelId == 0)
        {
            auto data = new XD::App::WndDataDefault();
            data->onHideCB.emplace_back([](XD::App::WndBaseHolder& wnd)
            {
                picPanelId = 0;
                App::logInfo("CC::MainPanel log - Failure to add file\n");
            });
            picPanelId = XD::App::WndMgr::open<ClipPicPanel>(data);
        }
    }

    /// @brief 主窗口遮盖
    class MainPanel_cover : public XD::App::WndBase<MainPanel_cover>
    {
    public:
        MainPanel_cover() : WndBase<MainPanel_cover>(XD::App::LoopLayer::WndTop) {}
        static void showCover(bool show) {
            if (!_this) return;
            _this->open = show;
    }

    private:
        ImGuiWindowFlags windowFlags    = 0;
        bool open                       = true;

        static MainPanel_cover* _this;
    protected:
        void onShow(XD::App::WndDataBaseHolder*) override
        {
            windowFlags |= ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar;
            windowFlags |= ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoNav;
            windowFlags |= ImGuiWindowFlags_NoDocking;

            if (_this) { closeSelf(); return; }
            _this = this;

            registerEvent<StaticEvent::OnWindowResizeBegin>([](){ showCover(true); });
            registerEvent<StaticEvent::OnWindowResizeEnd>([](){ showCover(false); });
        }

        void onRefresh() override {
            ImGui::SetWindowSize({(float)App::getW() + 128, (float)App::getH() + 128});
            ImGui::SetWindowPos({-64, -64});
            ImGui::BringWindowToFocusFront(ImGui::GetCurrentWindow());
        }

        bool onWndBegin() override {
            if (open) {
                ImGui::PushStyleColor(ImGuiCol_WindowBg, (ImU32)ImColor(1.f, 1.f, 1.f, 0.f));
                ImGui::Begin("mainPanel_cover", nullptr, windowFlags);
                return true;
            }
            return false;
        }

        void onWndEnd() override
        {
            if (open)
            {
                ImGui::End();
                ImGui::PopStyleColor();
            }
        }

        void onHide() override
        {
            _this = nullptr;
        }
    };

    class MainPanel : public XD::App::WndBase<MainPanel>
    {
    private:
        ImGuiWindowFlags windowFlags    = 0;
        ImGuiDockNodeFlags dockFlags    = 0;
        ImGuiWindowFlags btnPanelFlags  = 0;
        bool open                       = true;
        ImDrawList* drawList            = nullptr;
        ImVec2 _pLU                     = ImVec2(0, 0);
        ImVec2 _pRD                     = ImVec2(0, 0);
        int _lineWid                    = 25;

        /// @brief 背景的 log 信息
        std::list<std::string> _info;

    protected:
        void onShow(XD::App::WndDataBaseHolder* ) override
        {
            XD::App::setBGColor(ImVec4(0.22f, 0.22f, 0.22f, 1.00f));
            windowFlags |= ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar;
            windowFlags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoScrollbar;
            windowFlags |= ImGuiWindowFlags_NoDocking;

            btnPanelFlags |= ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar;
            btnPanelFlags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoScrollbar;
            btnPanelFlags |= ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_AlwaysAutoResize;

            XD::App::WndMgr::open<MainPanel_cover>();
            MainPanel_cover::showCover(false);

            onRegister();
        };

        void onRefresh() override
        {
            drawList = ImGui::GetWindowDrawList();
            ImGui::SetWindowFontScale(1.f);
            drawGridWnd();
            docking();
        };

        bool onWndBegin() override
        {
            drawWinBtn();
            const ImGuiViewport* viewport = ImGui::GetMainViewport();
            ImGui::SetNextWindowPos(viewport->WorkPos);
            ImGui::SetNextWindowSize(viewport->WorkSize);
            ImGui::SetNextWindowViewport(viewport->ID);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 0.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
            ImGui::PushStyleColor(ImGuiCol_DockingEmptyBg, (ImU32)ImColor(1.f, 1.f, 1.f, 0.f));
            _pLU = viewport->WorkPos;
            _pRD = viewport->WorkSize;
            if (ImGui::Begin("mainPanel", &open, windowFlags)) return true;
            return false;
        }

        void onWndEnd() override
        {
            ImGui::End();
            ImGui::PopStyleVar(4);
            ImGui::PopStyleColor();
        }

    private:
        // --------------------------- 文字背景
        /// @brief 绘制背景
        void drawGridWnd()
        {
            static float startX = 24.f;
            static float startY = 6.f;

            drawList->AddRectFilled({_pLU.x + 12, _pLU.y - 10}, {_pLU.x + 16, _pRD.y + 10}, ImColor(0.4f, 0.4f, 0.4f));
            if (FileCtrl::getInst().fileQueue.empty())
            {
                startY = -10.f;
                for (auto pY = startY; pY < _pRD.y; pY += _lineWid)
                {
                    drawList->AddText({startX, pY}, (ImU32)ImColor(0.4f, 0.4f, 0.4f), ">> 请，拖拽图片文件或文件夹到窗口。");
                }
            }
            else
            {
                startY = 4.f; auto pY = startY;
                static std::string outputDirView;
                outputDirView = "outputDir = \"" + FileCtrl::getInst().curDir.string() + "\"";
                drawList->AddText({startX, pY}, (ImU32)ImColor(0.4f, 0.4f, 0.4f), outputDirView.c_str()); pY += _lineWid;
                drawList->AddText({startX, pY}, (ImU32)ImColor(0.4f, 0.4f, 0.4f),
                "// -----------------------------------------------------------------------------------------------------");
            }
        }

        // --------------------------- 刷新窗口停靠
        void docking()
        {
            ImGui::DockSpace(ImGui::GetID("mainPanel_docking"), {0.f, 0.f}, dockFlags);
        }

        void drawWinBtn()
        {
            // const ImGuiViewport* viewport = ImGui::GetMainViewport();
            // ImGui::SetNextWindowPos({viewport->WorkPos.x, viewport->WorkSize.y - 50.f});
            // ImGui::SetNextWindowViewport(viewport->ID);
            // ImGui::Begin("winBtnPanel", nullptr, btnPanelFlags);
            // ImGui::End();
        }

        // ---------------------------- 绑定事件
        void onRegister()
        {
            registerEvent<StaticEvent::OnFilePush>(openPicPanel);
        }
    };
}