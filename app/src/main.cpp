#include "core/clipCtrl.h"
#include "core/fileCtrl.h"
#include "ui/mainPanel.hpp"

#include "CONF_CLIPTOOL_INI.h"
#include "IMGS_CLIPTOOLICON_PNG.h"

namespace CC
{
    /// @brief 主循环启动器
    class MainUnit : public XD::App::LoopUnit
    {
    public:
        MainUnit() : XD::App::LoopUnit() {}

        void start() override
        {
            FileCtrl::init();
            ClipCtrl::init();

            XD::App::WndMgr::open<UI::MainPanel>();
        }

        void update() override
        {
            using namespace XD;
            using namespace CC::UI;
            if (ImguiMgr::notLoadingFinished()) return;
            if (ImguiMgr::getIO().KeysDown[ImGuiKey_Escape])
                App::quit();
        }
    };

    UI::MainPanel_cover* UI::MainPanel_cover::_this = nullptr;
    static std::unique_ptr<MainUnit> mainUnit = nullptr;

    glm::ivec2 UI::gridCreatePreviewLineCount = {};
    glm::ivec2 UI::gridCreatePreviewSize = {};
    glm::ivec2 UI::gridCreatePreviewP0 = {};
    bool UI::gridCreatePreviewShown = false;

    ssize_t UI::curHoveredClip = -1;
    ssize_t UI::curDragedClip = -1;
    std::vector<uint8_t> UI::curSelectedClips = {};
    std::vector<glm::vec4> UI::clipChanges;
    bool UI::curSelectedClipsReset = true;
}

void onInit(int argc, char* argv[])
{
    XD::xdWndInitConf_confFileName = "clipTool.ini";
    XD::xdWndInitConf_wndName = "🔪🩸💔😨";
    XD::xdWndInitConf_iconPngData = XD::RC::XD_RC_IMGS_CLIPTOOLICON_PNG.data();
    XD::xdWndInitConf_iconPngSize = XD::RC::XD_RC_IMGS_CLIPTOOLICON_PNG.size();

    std::filesystem::path path = XD::xdWndInitConf_confFileName;
    if (!std::filesystem::exists(path))
    {
        std::fstream f(path, std::ios_base::out | std::ios_base::binary);
        f.write((const char *)XD::RC::XD_RC_CONF_CLIPTOOL_INI.data(), XD::RC::XD_RC_CONF_CLIPTOOL_INI.size());
        f.close();
    }
}

void onStart(int argc, char* argv[])
{
    CC::mainUnit = std::make_unique<CC::MainUnit>();
}

void onClose()
{
    CC::ImgCtrl::clear();
    CC::mainUnit.reset();
}