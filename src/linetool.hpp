#ifndef LINETOOL_H
#define LINETOOL_H

class LineTool
{
    // This tool works differently from some of the others in that it shows a "forecasted" shape
    // as you drag the mouse, then "commits" the shape to the canvas when you let go of the mouse.
    bool m_forecasting = false;

    // The tool has a fastpath for pen radius == 1.
    float m_penRadius;
    std::vector<int> m_indices;
    std::vector<SDL_Vertex> m_vertices;
    float m_lineStartX = 0;
    float m_lineStartY = 0;
    float m_lineEndX = 0;
    float m_lineEndY = 0;
    SDL_Color m_penColor{};

    void EnsureIndices()
    {
        if (!m_indices.empty())
            return;

        if (m_penRadius == 1)
            return;

        m_indices.push_back(0);
        m_indices.push_back(1);
        m_indices.push_back(2);
        m_indices.push_back(0);
        m_indices.push_back(2);
        m_indices.push_back(3);
    }

    void EnsureVertices()
    {
        if (m_penRadius == 1)
            return;

        m_vertices.clear();

        float dx = m_lineEndX - m_lineStartX;
        float dy = m_lineEndY - m_lineStartY;

        float mag = sqrt((dx * dx) + (dy * dy));

        if (mag == 0)
            return;

        float perpX = dy * m_penRadius / mag;
        float perpY = -dx * m_penRadius / mag;

        SDL_FColor fColor = hackforge::OpaqueUnormColorToOpaqueFloatColor(m_penColor);

        {
            SDL_Vertex v{};
            v.color = fColor;
            v.position.x = m_lineStartX - perpX;
            v.position.y = m_lineStartY - perpY;
            m_vertices.push_back(v);
        }
        {
            SDL_Vertex v{};
            v.color = fColor;
            v.position.x = m_lineStartX + perpX;
            v.position.y = m_lineStartY + perpY;
            m_vertices.push_back(v);
        }
        {
            SDL_Vertex v{};
            v.color = fColor;
            v.position.x = m_lineEndX + perpX;
            v.position.y = m_lineEndY + perpY;
            m_vertices.push_back(v);
        }
        {
            SDL_Vertex v{};
            v.color = fColor;
            v.position.x = m_lineEndX - perpX;
            v.position.y = m_lineEndY - perpY;
            m_vertices.push_back(v);
        }
    }

    void DrawLineToTarget(SDL_Renderer* renderer)
    {
        if (m_penRadius == 1)
        {
            SDL_RenderLine(renderer, m_lineStartX, m_lineStartY, m_lineEndX, m_lineEndY);
        }
        else
        {
            if (m_vertices.empty()) return;

            SDL_RenderGeometry(
                renderer,
                nullptr,
                m_vertices.data(),
                static_cast<int>(m_vertices.size()),
                m_indices.data(),
                static_cast<int>(m_indices.size()));
        }
    }

public:

    inline void Update(
        SDL_Renderer* renderer, // for drawing to canvas
        bool penDown,
        float currentPenX,
        float currentPenY,
        SDL_Color penColor,
        float penRadius)
    {
        if (penDown && !m_forecasting)
        {
            m_lineStartX = currentPenX;
            m_lineStartY = currentPenY;
            m_lineEndX = currentPenX;
            m_lineEndY = currentPenY;
            m_penRadius = penRadius;
            m_penColor = penColor;
            m_forecasting = true;
            return;
        }

        if (penDown)
        {
            m_lineEndX = currentPenX;
            m_lineEndY = currentPenY;
            EnsureIndices();
            EnsureVertices();
            return;
        }

        if (m_forecasting)
        {
            DrawLineToTarget(renderer); // Commits the line to canvas
            m_forecasting = false;
        }
    }

    inline void RenderUI(
        SDL_Renderer* renderer)
    {
        if (m_forecasting)
        {
            DrawLineToTarget(renderer); // Draws the line as temporary UI
        }
    }
};

#endif