#include "Bucket.hpp"
#include <stack>
#include <assert.h>

// Intended for operations where real-time performance isn't needed, to be used
// sparingly, since it copies the target to a CPU-visible resource and uploads
// it back
class CPUEffect
{
    SDL_Surface* m_intermediateCpu{};
    unsigned int* m_pixelData;
    SDL_Renderer* m_renderer{};
    SDL_Texture* m_canvas{};
    int m_lockedWidth;
    int m_lockedHeight;

public:
    void Initialize(SDL_Renderer* renderer, int w, int h, SDL_Texture* c)
    {
        m_renderer = renderer;
        m_lockedWidth = w;
        m_lockedHeight = h;
        m_canvas = c;
    }

    void BeginCpuWrite()
    {
        SDL_Rect lockRect{};
        lockRect.w = m_lockedWidth;
        lockRect.h = m_lockedHeight;
        m_intermediateCpu = SDL_RenderReadPixels(m_renderer, &lockRect);

        m_pixelData = reinterpret_cast<unsigned int*>(m_intermediateCpu->pixels);
    }

    unsigned int ReadLockedPixel(int x, int y)
    {
        return m_pixelData[y * m_lockedWidth + x];
    }

    void WriteLockedPixel(int x, int y, unsigned int value)
    {
        m_pixelData[y * m_lockedWidth + x] = value;
    }


    void EndCpuWrite()
    {
        SDL_Texture* intermediateGpu = SDL_CreateTextureFromSurface(m_renderer, m_intermediateCpu);

        SDL_SetRenderTarget(m_renderer, m_canvas);
        SDL_SetRenderScale(m_renderer, 1.0f, 1.0f);
        SDL_RenderTexture(m_renderer, intermediateGpu, NULL, NULL);

        SDL_DestroyTexture(intermediateGpu);
        SDL_DestroySurface(m_intermediateCpu);

        m_intermediateCpu = nullptr;
    }

    int GetLockedWidth() const { return m_lockedWidth; }

    int GetLockedHeight() const { return m_lockedHeight; }

};

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

    CPUEffect cpuEffect;
    cpuEffect.Initialize(renderer, window_width, window_height, canvas);

    cpuEffect.BeginCpuWrite();

    unsigned int fillColor = SDLColorToX8R8G8B8(penColor);
    FloodFill(&cpuEffect, fillColor, (int)x, (int)y);

    cpuEffect.EndCpuWrite();
}