#ifndef __Polyline_h
#define __Polyline_h

#include "math/Vector3.h"
#include <vector>

namespace cg {

// Alias do tipo vetorial de 3 dimensões da biblioteca Ds
using vec3 = vec3f;

enum class GeneratrixType {
    RegularPolygon,
    CircularArc
};

class Polyline {
public:
    Polyline();

    // Enum e Estrutura de Parâmetros
    struct Parameters {
        GeneratrixType type = GeneratrixType::RegularPolygon;
        int numPoints = 4;           // np para polígono (3 a 40) ou nc para arco (2 a 40)
        float arcAngle = 180.0f;     // theta_c (0 a 360)
        bool isClosed = true;        // se o arco é fechado (setor circular)
        float scale = 1.0f;          // sp (0.1 a 5.0)
        float rotationAngle = 0.0f;  // theta_p (0 a 360 graus)
        bool enableExtrusion = false;
        float extrusionFactor = 0.2f;// ep (0.1 a 0.9)
    };

    // Atualiza os pontos da polilinha com base nos parâmetros
    void update(const Parameters& params);

    // Getters
    const std::vector<vec3>& points() const { return m_points; }
    int size() const { return static_cast<int>(m_points.size()); }
    bool isClosed() const { return m_isClosed; }
    bool isExtruded() const { return m_isExtruded; }

    // Calcula a AABB (Caixa Envolvente) no plano xy: bx = max_x - min_x, by = max_y - min_y
    void getBoundingBox(float& bx, float& by) const;

private:
    std::vector<vec3> m_points; // Armazena até 80 pontos
    bool m_isClosed = true;
    bool m_isExtruded = false;

    void buildRegularPolygon(int np);
    void buildCircularArc(int nc, float arcAngle, bool isClosed);
    void applyExtrusion(float ep);
    void applyTransformations(float scale, float rotationAngleDeg);
};

} // namespace cg

#endif // __Polyline_h