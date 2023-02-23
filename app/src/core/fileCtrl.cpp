#include "fileCtrl.h"

namespace CC
{
    using namespace XD;
    static const std::array support_path =
    {
        std::filesystem::u8path(".png"),
        std::filesystem::u8path(".jpg"),
        std::filesystem::u8path(".bmp"),
        std::filesystem::u8path(".tga"),
        std::filesystem::u8path(".psd")
    };

    static void OnDropFile(std::filesystem::directory_entry dir)
    {
        App::logInfo("CC::FileCtrl log - Add files:");
        if (dir.is_directory())
        {
            auto f_itr = std::filesystem::directory_iterator(dir);
            for(auto& f : f_itr)
            {
                if (std::find(
                    std::begin(support_path),
                    std::end(support_path),
                    f.path().extension()) != std::end(support_path))
                {
                    StaticEventMgr::broadcastAsync<StaticEvent::OnFilePush>();
                    FileCtrl::getInst().fileQueue.push_back(f.path());
                    App::logInfo("\n    ", f.path());
                }
            }
            App::logInfo("\n");
        }
        else
        {
            if (std::find(
                std::begin(support_path),
                std::end(support_path),
                dir.path().extension()) != std::end(support_path))
            {
                StaticEventMgr::broadcastAsync<StaticEvent::OnFilePush>();
                FileCtrl::getInst().fileQueue.push_back(dir.path());
                App::logInfo("\n    ", dir.path());
            }
            App::logInfo("\n");
        }

        FileCtrl::getInst().curDir = dir.path().parent_path();
    }

    std::unique_ptr<FileCtrl::FileCtrlData> FileCtrl::_inst = nullptr;

    void FileCtrl::init()
    {
        if (!_inst) _inst = std::make_unique<FileCtrlData>();
        StaticEventMgr::registerEvent<StaticEvent::OnDropFile>((std::ptrdiff_t)_inst.get(), OnDropFile);
    }

    FileCtrl::FileCtrlData& FileCtrl::getInst()
    {
        if (!_inst) init();
        return *_inst;
    }

    void FileCtrl::setOutputPath(const std::filesystem::path& out)
    {
        if (!_inst) return;
        if (out.is_relative())
        {
            _inst->outputPath = _inst->curDir;
            _inst->outputPath += out;
        }
        else _inst->outputPath = out;
    }
}