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
    float length = 10.0f;      // lv: comprimento
    float offsetW = 0.0f;      // ov_w: deslocamento horizontal
    float offsetH = 0.0f;      // ov_h: deslocamento vertical
    int subdivisions = 10;     // nv_s: número de subdivisões da varredura
    float scaleBegin = 1.0f;   // sv_b: escala inicial
    float scaleEnd = 1.0f;     // sv_e: escala final
    float twist = 0.0f;        // rv: torção (revoluções em Z)
    bool makeCaps = true;      // tampas nas extremidades
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

} // namespace cg::graph

#endif // __SweepMeshProxy_h