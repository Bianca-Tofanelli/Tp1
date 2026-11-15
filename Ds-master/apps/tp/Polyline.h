#ifndef __Polyline_h
#define __Polyline_h

#include "math/Vector3.h"
#include <vector>

namespace cg {


using vec3 = vec3f;

enum class GeneratrixType {
    RegularPolygon,
    CircularArc
};

class Polyline {
public:
    Polyline();

    
    struct Parameters {
        GeneratrixType type = GeneratrixType::RegularPolygon;
        int numPoints = 4;           
        float arcAngle = 180.0f;     
        bool isClosed = true;        
        float scale = 1.0f;         
        float rotationAngle = 0.0f;  
        bool enableExtrusion = false;
        float extrusionFactor = 0.2f;
    };

    
    void update(const Parameters& params);

    
    const std::vector<vec3>& points() const { return m_points; }
    int size() const { return static_cast<int>(m_points.size()); }
    bool isClosed() const { return m_isClosed; }
    bool isExtruded() const { return m_isExtruded; }

    
    void getBoundingBox(float& bx, float& by) const;

private:
    std::vector<vec3> m_points; 
    bool m_isClosed = true;
    bool m_isExtruded = false;

    void buildRegularPolygon(int np);
    void buildCircularArc(int nc, float arcAngle, bool isClosed);
    void applyExtrusion(float ep);
    void applyTransformations(float scale, float rotationAngleDeg);
};

} 

#endif 