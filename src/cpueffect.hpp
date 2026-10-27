#ifndef CPUEFFECT_H
#define CPUEFFECT_H

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
    CPUEffect(SDL_Renderer* renderer, int w, int h, SDL_Texture* c)
        : m_renderer(renderer)
        , m_lockedWidth(w)
        , m_lockedHeight(h)
        , m_canvas(c)
    {}

    void BeginCpuWrite()
    {
        SDL_Rect lockRect{ .w = m_lockedWidth, .h = m_lockedHeight };
        m_intermediateCpu = SDL_RenderReadPixels(m_renderer, &lockRect);
        m_pixelData = reinterpret_cast<unsigned int*>(m_intermediateCpu->pixels);
    }

    unsigned int ReadLockedPixel(int x, int y)
    {
        assert(x >= 0 && y >= 0 && x < m_lockedWidth&& y < m_lockedHeight);
        return m_pixelData[y * m_lockedWidth + x];
    }

    void WriteLockedPixel(int x, int y, unsigned int value)
    {
        assert(x >= 0 && y >= 0 && x < m_lockedWidth&& y < m_lockedHeight);
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

#endif