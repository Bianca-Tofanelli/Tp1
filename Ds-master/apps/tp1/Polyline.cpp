#include "Polyline.h"
#include <cmath>
#include <algorithm>

namespace cg {

constexpr float PI = 3.14159265358979323846f;

Polyline::Polyline() {
    m_points.reserve(80); // Reserva capacidade estática máxima para até 80 pontos
}

void Polyline::update(const Parameters& params) {
    m_points.clear();
    m_isExtruded = params.enableExtrusion;

    if (params.type == GeneratrixType::RegularPolygon) {
        int np = std::clamp(params.numPoints, 3, 40);
        buildRegularPolygon(np);
        m_isClosed = true;
    } else {
        int nc = std::clamp(params.numPoints, 2, 40);
        float angle = std::clamp(params.arcAngle, 0.1f, 360.0f);
        buildCircularArc(nc, angle, params.isClosed);
    }

    if (m_isExtruded) {
        float ep = std::clamp(params.extrusionFactor, 0.1f, 0.9f);
        applyExtrusion(ep);
    }

    float sp = std::clamp(params.scale, 0.1f, 5.0f);
    float tp = params.rotationAngle;
    applyTransformations(sp, tp);
}

void Polyline::buildRegularPolygon(int np) {
    // P0 = (0, 1, 0)
    // Pi obtido girando P0 em torno de Z de (360 / np) * i
    float stepAngle = (2.0f * PI) / static_cast<float>(np);

    for (int i = 0; i < np; ++i) {
        float a = stepAngle * static_cast<float>(i);
        // Rotação no plano XY
        float x = -std::sin(a);
        float y = std::cos(a);
        m_points.push_back(vec3(x, y, 0.0f));
    }
}

void Polyline::buildCircularArc(int nc, float arcAngleDeg, bool isClosed) {
    if (arcAngleDeg >= 360.0f) {
        buildRegularPolygon(nc);
        m_isClosed = true;
        return;
    }

    int np = nc + 1;
    m_isClosed = isClosed;

    float arcAngleRad = arcAngleDeg * (PI / 180.0f);
    float startAngle = (PI - arcAngleRad) / 2.0f; // Simetria em relação ao eixo Y (plano YZ)
    float stepAngle = arcAngleRad / static_cast<float>(nc);

    for (int i = 0; i < np; ++i) {
        float a = startAngle + stepAngle * static_cast<float>(i);
        float x = std::cos(a);
        float y = std::sin(a);
        m_points.push_back(vec3(x, y, 0.0f));
    }
}

void Polyline::applyExtrusion(float ep) {
    int np = static_cast<int>(m_points.size());
    float factor = 1.0f - ep;

    for (int i = 0; i < np; ++i) {
        vec3 pExt = m_points[i] * factor;
        m_points.push_back(pExt);
    }
}

void Polyline::applyTransformations(float scale, float rotationAngleDeg) {
    float rotRad = rotationAngleDeg * (PI / 180.0f);
    float cosA = std::cos(rotRad);
    float sinA = std::sin(rotRad);

    for (auto& p : m_points) {
        // 1. Escala (sp, sp, 1)
        p.x *= scale;
        p.y *= scale;

        // 2. Rotação em torno do eixo Z
        float rx = p.x * cosA - p.y * sinA;
        float ry = p.x * sinA + p.y * cosA;

        p.x = rx;
        p.y = ry;
    }
}

void Polyline::getBoundingBox(float& bx, float& by) const {
    if (m_points.empty()) {
        bx = by = 0.0f;
        return;
    }

    float minX = m_points[0].x, maxX = m_points[0].x;
    float minY = m_points[0].y, maxY = m_points[0].y;

    for (const auto& p : m_points) {
        minX = std::min(minX, p.x);
        maxX = std::max(maxX, p.x);
        minY = std::min(minY, p.y);
        maxY = std::max(maxY, p.y);
    }

    bx = maxX - minX;
    by = maxY - minY;
}

} // namespace cg