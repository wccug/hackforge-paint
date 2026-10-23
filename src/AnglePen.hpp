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
        int penRadius,
        int penAngle)
    {
        EnsureIndices();

        const float pi = 3.14159f;
        float radians = static_cast<float>(penAngle) * (pi / 180.0f);
        float x = cosf(radians) * static_cast<float>(penRadius);
        float y = -sinf(radians) * static_cast<float>(penRadius);

        std::vector<SDL_Vertex> vertices;
        {
            SDL_Vertex v{};
            v.color = hackforge::OpaqueUnormColorToOpaqueFloatColor(penColor);
            v.position.x = previousPenX - x;
            v.position.y = previousPenY - y;
            vertices.push_back(v);
        }
        {
            SDL_Vertex v{};
            v.color = hackforge::OpaqueUnormColorToOpaqueFloatColor(penColor);
            v.position.x = previousPenX + x;
            v.position.y = previousPenY + y;
            vertices.push_back(v);
        }
        {
            SDL_Vertex v{};
            v.color = hackforge::OpaqueUnormColorToOpaqueFloatColor(penColor);
            v.position.x = currentPenX + x;
            v.position.y = currentPenY + y;
            vertices.push_back(v);
        }
        {
            SDL_Vertex v{};
            v.color = hackforge::OpaqueUnormColorToOpaqueFloatColor(penColor);
            v.position.x = currentPenX - x;
            v.position.y = currentPenY - y;
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