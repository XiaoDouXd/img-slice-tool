
#pragma once

#include "glm.hpp"
#include "imgui_internal.h"

namespace CC
{
    static inline float __p1p2_cross_p1p(glm::ivec2 p1, glm::ivec2 p2, glm::ivec2 p)
    {
        return (p2.x - p1.x) * (p.y - p1.y) - (p.x - p1.x) * (p2.y - p1.y);
    }

    static inline bool rectTestUnknowMinMax(glm::ivec2 a, glm::ivec2 b, glm::ivec2 pos)
    {
        auto a2 = glm::ivec2(a.x, b.y);
        auto b2 = glm::ivec2(b.x, a.y);
        return  __p1p2_cross_p1p(a, a2, pos) * __p1p2_cross_p1p(b, b2, pos) >= 0 &&
                __p1p2_cross_p1p(a2, b, pos) * __p1p2_cross_p1p(b2, a, pos) >= 0;
    }

    static inline bool twoRectTest(glm::ivec2 min, glm::ivec2 size, glm::ivec2 min2, glm::ivec2 size2)
    {
        return  min.x + size.x > min2.x && min2.x + size2.x > min.x &&
                min.y + size.y > min2.y && min2.y + size2.y > min.y;
    }

    static inline bool twoRectTestFloat(glm::vec2 min, glm::vec2 size, glm::vec2 min2, glm::vec2 size2)
    {
        return  min.x + size.x > min2.x && min2.x + size2.x > min.x &&
                min.y + size.y > min2.y && min2.y + size2.y > min.y;
    }

    static inline bool rectTest(glm::ivec2 mousePos, glm::ivec2 rectMin, glm::ivec2 rectMax)
    {
        return  mousePos.x >= rectMin.x && mousePos.x <= rectMax.x &&
                mousePos.y >= rectMin.y && mousePos.y <= rectMax.y;
    }

    static inline bool rectTest(ImVec2 mousePos, ImVec2 rectMin, ImVec2 rectMax)
    {
        return  mousePos.x >= rectMin.x && mousePos.x <= rectMax.x &&
                mousePos.y >= rectMin.y && mousePos.y <= rectMax.y;
    }
}