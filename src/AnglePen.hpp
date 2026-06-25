#pragma once

class AnglePen
{
    std::vector<int> m_indices;

    static SDL_FColor OpaqueUnormColorToOpaqueFloatColor(SDL_Color c)
    {
        SDL_FColor r;
        r.a = 1.0f;
        r.r = static_cast<float>(c.r) / 255.0f;
        r.g = static_cast<float>(c.g) / 255.0f;
        r.b = static_cast<float>(c.b) / 255.0f;
        return r;
    }

public:

    void EnsureIndices()
    {
        if (m_indices.size() > 0)
            return;

        m_indices.push_back(0);
        m_indices.push_back(1);
        m_indices.push_back(2);
        m_indices.push_back(0);
        m_indices.push_back(2);
        m_indices.push_back(3);
    }

    void Render(SDL_Renderer* renderer, float previousPenX, float previousPenY, float currentPenX, float currentPenY, SDL_Color const& penColor)
    {
        EnsureIndices();

        std::vector<SDL_Vertex> vertices;
        {
            SDL_Vertex v{};
            v.color = OpaqueUnormColorToOpaqueFloatColor(penColor);
            v.position.x = previousPenX - 10;
            v.position.y = previousPenY - 10;
            vertices.push_back(v);
        }
        {
            SDL_Vertex v{};
            v.color = OpaqueUnormColorToOpaqueFloatColor(penColor);
            v.position.x = previousPenX + 10;
            v.position.y = previousPenY + 10;
            vertices.push_back(v);
        }
        {
            SDL_Vertex v{};
            v.color = OpaqueUnormColorToOpaqueFloatColor(penColor);
            v.position.x = currentPenX + 10;
            v.position.y = currentPenY + 10;
            vertices.push_back(v);
        }
        {
            SDL_Vertex v{};
            v.color = OpaqueUnormColorToOpaqueFloatColor(penColor);
            v.position.x = currentPenX - 10;
            v.position.y = currentPenY - 10;
            vertices.push_back(v);
        }

        SDL_RenderGeometry(renderer, nullptr, vertices.data(), vertices.size(), m_indices.data(), m_indices.size());
    }
};