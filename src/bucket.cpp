#include "bucket.hpp"
#include "cpueffect.hpp"
#include <stack>
#include <cassert>

static unsigned int SDLColorToX8R8G8B8(SDL_Color srcColor)
{
    unsigned int result = 0;
    result |= 0xFF;
    result <<= 8;
    result |= srcColor.r;
    result <<= 8;
    result |= srcColor.g;
    result <<= 8;
    result |= srcColor.b;
    return result;
}

static void FloodFill(CPUEffect* pCpu, unsigned int fillColor, int xPos, int yPos)
{
    unsigned int referenceColor = pCpu->ReadLockedPixel(xPos, yPos);

    if (referenceColor == fillColor) {
        return;
    }

    struct StackItem
    {
        int X, Y;
    };

    std::stack<StackItem> stack;

    {
        // Push initial item on the stack
        StackItem si = { xPos, yPos };
        stack.push(si);
    }

    while (!stack.empty())
    {
        StackItem si = stack.top();
        stack.pop();

        pCpu->WriteLockedPixel(si.X, si.Y, fillColor);

        if (si.X > 0)
        {
            unsigned int left = pCpu->ReadLockedPixel(si.X - 1, si.Y);
            if (left == referenceColor)
            {
                StackItem nextItem = { si.X - 1, si.Y };
                stack.push(nextItem);
            }
        }

        if (si.Y > 0)
        {
            unsigned int above = pCpu->ReadLockedPixel(si.X, si.Y - 1);
            if (above == referenceColor)
            {
                StackItem nextItem = { si.X, si.Y - 1 };
                stack.push(nextItem);
            }
        }
        if (si.X < pCpu->GetLockedWidth() - 1)
        {
            unsigned int right = pCpu->ReadLockedPixel(si.X + 1, si.Y);
            if (right == referenceColor)
            {
                StackItem nextItem = { si.X + 1, si.Y };
                stack.push(nextItem);
            }
        }
        if (si.Y < pCpu->GetLockedHeight() - 1)
        {
            unsigned int below = pCpu->ReadLockedPixel(si.X, si.Y + 1);
            if (below == referenceColor)
            {
                StackItem nextItem = { si.X, si.Y + 1 };
                stack.push(nextItem);
            }
        }
    }
}


void RenderBucket(SDL_Renderer* renderer, int window_width, int window_height, SDL_Texture* canvas, SDL_Color penColor, float x, float y)
{
    SDL_PixelFormat fmt = canvas->format;
    if (fmt != SDL_PIXELFORMAT_XRGB8888)
    {
        assert(false);
        return; // Only X8R8G8B8 is expected.
    }

    CPUEffect cpuEffect(renderer, window_width, window_height, canvas);

    cpuEffect.BeginCpuWrite();

    unsigned int fillColor = SDLColorToX8R8G8B8(penColor);
    FloodFill(&cpuEffect, fillColor, static_cast<int>(x), static_cast<int>(y));

    cpuEffect.EndCpuWrite();
}
