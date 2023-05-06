#pragma once

#include <filesystem>
#include <list>
#include <memory>

#include "xdBase.h"

#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCUnusedGlobalDeclarationInspection"

namespace CC::FileCtrl
{
    class FileCtrlData
    {
    public:
        std::list<std::filesystem::path>    fileQueue;
        std::filesystem::path               curDir;
        std::filesystem::path               outputPath;
        uuids::uuid                         uuid = XD::UUID::gen();
    };

    void init();
    FileCtrlData& getInst();
    void setOutputPath(const std::filesystem::path& out);
}
#pragma clang diagnostic pop