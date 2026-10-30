#ifndef __SweepMeshProxy_h
#define __SweepMeshProxy_h

#include "graph/PrimitiveProxy.h"
#include "graph/SceneWindow.h"
#include "graphics/TriangleMeshMapper.h"
#include "Polyline.h"

namespace cg::graph
{

class SweepMeshProxy: public PrimitiveProxy
{
public:
  struct Parameters
  {
    float length = 10.0f;      
    float offsetW = 0.0f;      
    float offsetH = 0.0f;      
    int subdivisions = 10;     
    float scaleBegin = 1.0f;  
    float scaleEnd = 1.0f;     
    float twist = 0.0f;        
    bool makeCaps = true;      
  };

  SweepMeshProxy();

  TriangleMeshMapper* mapper() const
  {
    return static_cast<TriangleMeshMapper*>(PrimitiveProxy::mapper());
  }

  void updateMesh();
  bool inspect(SceneWindow& window);

  Polyline::Parameters& polylineParams() { return m_polylineParams; }
  Parameters& sweepParams() { return m_sweepParams; }

private:
  Polyline m_polyline;
  Polyline::Parameters m_polylineParams;
  Parameters m_sweepParams;
  char m_meshName[64] = "SweepMesh";

  void buildSweepMesh();
};

} 

#endif 