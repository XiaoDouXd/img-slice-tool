#pragma once

#include <cstdint>
#include <string>
#include <list>
#include <map>

#include "xdBase.h"

namespace CC
{
    enum class Op : uint8_t
    {
        // ----------- Clip
        PushClip,
        ChangeClip,
        MergeClip,
        SwapClip,
        EraseClip,
        DeleteClip,
        DevidClip,

        // ----------- File
        PushFile,
        PopFile,
        PopFileBack,

        // ----------- join
        AddJoin,
        MoveJoin,
        RemoveJoin
    };

    static inline std::string getOpName(const Op& op)
    {
#define opName(x) case Op::x: return #x;
        switch(op)
        {
        opName(PushClip)
        opName(ChangeClip)
        opName(MergeClip)
        opName(SwapClip)
        opName(EraseClip)
        opName(DeleteClip)
        opName(DevidClip)
        default: return std::to_string((int)op);
        }
#undef opName
    }

    template<typename PrstData, typename DoData>
    class PrstCtrl
    {
    protected:
        PrstCtrl() { _curOp = _ops.cend(); }

        /// @brief 根据持久化数据撤销行为 (redoAction 的逆向操作)
        /// @param op 行为
        /// @param data 持久化数据
        virtual void undoAction(Op op, const PrstData& data) = 0;

        /// @brief 开始行为前的数据预处理
        /// @param op 行为
        /// @param data 源行为数据
        /// @return 持久化数据
        virtual PrstData todoPreT(Op op, DoData&& data) = 0;

        /// @brief 用持久化数据执行行为
        ///        (这里的 todo 本质上是将持久化数据插入队尾然后执行 redoAction)
        /// @param op 行为
        /// @param data 持久化数据
        virtual void redoAction(Op op, const PrstData& data) = 0;

        /// @brief 行为过滤器
        /// @param op 行为
        /// @return 是否过滤
        virtual bool opFilter(Op op) = 0;

    public:
        bool isCanRedo() { return _curOp != _ops.cend(); }
        bool isCanUndo() { return _curOp != _ops.cbegin(); }
        void clear() { _ops.clear(); _curOp = _ops.cend(); }

        void undo()
        {
            if (_ops.empty() || _curOp == _ops.cbegin()) return;
            _curOp--;
            XD::App::logInfo("撤回: ", getOpName(_curOp->first), "\n");
            undoAction(_curOp->first, _curOp->second);
        }

        void todo(Op op, DoData&& data)
        {
            if (opFilter(op)) return;

            for (auto itr = _curOp; itr != _ops.cend();)
            {
                auto i = itr;
                itr++;
                _ops.erase(i);
            }
            if (_ops.size() == _maxOpCount && !_ops.empty()) _ops.pop_front();

            _ops.emplace_back(std::pair<Op, PrstData>(std::move(op), todoPreT(op, std::move(data))));
            _curOp = --_ops.end();
            XD::App::logInfo("执行: ", getOpName(op), "\n");
            redoAction(_curOp->first, _curOp->second);
            _curOp++;
        }

        void redo()
        {
            if (_curOp == _ops.cend()) return;
            XD::App::logInfo("重做: ", getOpName(_curOp->first), "\n");
            redoAction(_curOp->first, _curOp->second);
            _curOp++;
        }

    protected:
        size_t _maxOpCount = 100;
        std::list<std::pair<Op, PrstData>>::const_iterator _curOp;
        std::list<std::pair<Op, PrstData>> _ops;
    };
}