#pragma once

#include <filesystem>
#include <list>
#include <memory>

#include "xdBase.h"

namespace CC
{
    class FileCtrl
    {
    public:
        class FileCtrlData
        {
        public:
            std::list<std::filesystem::path>    fileQueue;
            std::filesystem::path               curDir;
            std::filesystem::path               outputPath;
        };
    private:
        static std::unique_ptr<FileCtrlData> _inst;

    public:
        static void init();
        static FileCtrlData& getInst();
        static void setOutputPath(const std::filesystem::path& out);
    };
}