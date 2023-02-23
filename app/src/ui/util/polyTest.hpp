#pragma once

#include "glm.hpp"
#include "xdBase.h"

namespace CC
{
    static inline bool __gjk_inside(const std::array<glm::ivec2, 3>& triangle)
    {
        float ab = triangle[1].x * triangle[2].y - triangle[1].y * triangle[2].x;
        if (glm::sign(ab) != glm::sign(triangle[2].x * triangle[3].y - triangle[2].y * triangle[3].x) ||
            glm::sign(ab) != glm::sign(triangle[3].x * triangle[1].y - triangle[3].y * triangle[1].x))
            return false;
        return true;
    }

    static inline void __gjk_removeP(std::array<glm::ivec2, 3>& triangle)
    {
        auto disMax = 0.f;
        size_t remove = 4;
        for (auto i = 0; i < 3; i++)
            if ((disMax = (triangle[i].x * triangle[i].x + triangle[i].y * triangle[i].y)) > disMax)
                remove = i;
        if (remove == 1) triangle[1] = triangle[2];
        else if (remove == 0) triangle[0] = triangle[2];
    }

    static inline glm::vec2 __gjk_support(const std::list<glm::ivec2>& poly, glm::vec2 v)
    {
        float dist = 0.f, maxDist = FLT_MIN;
        glm::vec2 mP;
        for (auto& p : poly)
        {
            if ((dist = glm::dot(glm::vec2(p), v)) > maxDist)
            { maxDist = dist; mP = p; }
        }
        return mP;
    }

    static inline bool polyTest(const std::list<glm::ivec2>& poly1, const std::list<glm::ivec2>& poly2, glm::vec2 init_v = glm::vec2(1, 1))
    {
        glm::vec2 a = __gjk_support(poly1, init_v) - __gjk_support(poly2, -init_v);
        std::array<glm::ivec2, 3> s; s[0] = a;
        size_t curWritePtr = 1;
        glm::vec2 d = -a;

        while (true)
        {
            a = __gjk_support(poly1, d) - __gjk_support(poly2, -d);
            if (glm::dot(a, d) < 0) return false;
            s[curWritePtr] = a;
            curWritePtr++;

            if (curWritePtr == 3)
                if (__gjk_inside(s)) return true;
                else { __gjk_removeP(s); curWritePtr = 2; }
            if (curWritePtr == 2)
            {
                d = s[0] - s[1];
                d = glm::vec2(-d.y, d.x);
                if (glm::dot(glm::vec2(s[0]), d) < 0) d = -d;
            }
        }
        return false;
    }

    static inline bool triangleTest(const glm::vec2& t0, const glm::vec2& t1, const glm::vec2& t2, const glm::vec2& pos)
    {
        auto a = t0 - pos, b = t1 - pos, c = t2 - pos;
        float ab = a.x * b.y - a.y * b.x;
        if (glm::sign(ab) != glm::sign(b.x * c.y - b.y * c.x) ||
            glm::sign(ab) != glm::sign(c.x * a.y - c.y * a.x))
            return false;
        return true;
    }
}