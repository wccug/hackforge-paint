#pragma once

class AnglePen
{
    std::vector<int> m_indices;

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

    void Render(
        SDL_Renderer* renderer, 
        float previousPenX, 
        float previousPenY, 
        float currentPenX, 
        float currentPenY, 
        SDL_Color const& penColor,
        int penRadius)
    {
        EnsureIndices();

        std::vector<SDL_Vertex> vertices;
        {
            SDL_Vertex v{};
            v.color = hackforge::OpaqueUnormColorToOpaqueFloatColor(penColor);
            v.position.x = previousPenX - penRadius;
            v.position.y = previousPenY - penRadius;
            vertices.push_back(v);
        }
        {
            SDL_Vertex v{};
            v.color = hackforge::OpaqueUnormColorToOpaqueFloatColor(penColor);
            v.position.x = previousPenX + penRadius;
            v.position.y = previousPenY + penRadius;
            vertices.push_back(v);
        }
        {
            SDL_Vertex v{};
            v.color = hackforge::OpaqueUnormColorToOpaqueFloatColor(penColor);
            v.position.x = currentPenX + penRadius;
            v.position.y = currentPenY + penRadius;
            vertices.push_back(v);
        }
        {
            SDL_Vertex v{};
            v.color = hackforge::OpaqueUnormColorToOpaqueFloatColor(penColor);
            v.position.x = currentPenX - penRadius;
            v.position.y = currentPenY - penRadius;
            vertices.push_back(v);
        }

        SDL_RenderGeometry(
            renderer, 
            nullptr, 
            vertices.data(), 
            static_cast<int>(vertices.size()), 
            m_indices.data(), 
            static_cast<int>(m_indices.size()));
    }
};