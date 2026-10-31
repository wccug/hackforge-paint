#pragma once

class Spraypaint
{
    bool m_needRecreateRaster = true;
    std::vector<SDL_Point> m_relativeRaster;
    int m_rasterIndex = 0;

    int m_radius = 0;
    std::mt19937 m_rng;

    void EnsureRaster()
    {
        if (!m_needRecreateRaster)
            return;

        m_relativeRaster.clear();
        int radsq = m_radius * m_radius;
        for (int y = -m_radius; y < m_radius; ++y)
        {
            for (int x = -m_radius; x < m_radius; ++x)
            {
                SDL_Point coord = { x, y }; // Use a circle-shaped brush
                if ((x * x) + (y * y) < radsq)
                {
                    m_relativeRaster.push_back(coord);
                }
            }
        }
        std::shuffle(m_relativeRaster.begin(), m_relativeRaster.end(), m_rng);
        m_rasterIndex = 0;
        m_needRecreateRaster = false;
    }

    void PlotPixel(
        SDL_Renderer* renderer,
        float currentPenX,
        float currentPenY,
        SDL_Color const& penColor)
    {
        float x = currentPenX + m_relativeRaster[m_rasterIndex].x;
        float y = currentPenY + m_relativeRaster[m_rasterIndex].y;

        SDL_FRect rect{};
        rect.x = x;
        rect.y = y;
        rect.w = 1.0f;
        rect.h = 1.0f;

        SDL_SetRenderDrawColor(renderer, penColor.r, penColor.g, penColor.b, penColor.a);
        SDL_RenderFillRect(renderer, &rect);

        m_rasterIndex = (m_rasterIndex + 1) % static_cast<int>(m_relativeRaster.size());
    }

public:

    void Render(
        SDL_Renderer* renderer,
        float currentPenX,
        float currentPenY,
        SDL_Color const& penColor,
        int penRadius)
    {
        if (penRadius != m_radius)
        {
            // Recreate raster pattern because the pen radius changed
            m_needRecreateRaster = true;
            m_radius = penRadius;
        }

        EnsureRaster();

        PlotPixel(renderer, currentPenX, currentPenY, penColor);
    }
};