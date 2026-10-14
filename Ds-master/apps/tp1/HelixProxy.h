#ifndef __HelixProxy_h
#define __HelixProxy_h

#include "graph/PrimitiveProxy.h"
#include "graph/SceneWindow.h"
#include "graphics/TriangleMeshMapper.h"
#include "Polyline.h"

namespace cg::graph
{

class HelixProxy: public PrimitiveProxy
{
public:
  struct Parameters
  {
    float initialWidth = 4.0f;    // we: largura inicial
    float revolutions = 1.0f;     // re: número de revoluções
    int subdivisions = 16;        // n_e^s: subdivisões por revolução
    float deltaHeight = 0.0f;     // delta_e^h: incremento de altura
    float deltaWidth = 0.0f;      // delta_e^w: incremento de largura
    bool makeCaps = true;         // tampas nas extremidades
  };

  HelixProxy();

  // Retorna o mapeador da malha de triângulos
  TriangleMeshMapper* mapper() const
  {
    return static_cast<TriangleMeshMapper*>(PrimitiveProxy::mapper());
  }

  void updateMesh();
  bool inspect(SceneWindow& window);
  Polyline::Parameters& polylineParams() { return m_polylineParams; }
  Parameters& helixParams() { return m_helixParams; }

private:
  Polyline m_polyline;
  Polyline::Parameters m_polylineParams;
  Parameters m_helixParams;
  char m_meshName[64] = "HelixMesh";

  void buildSpiralMesh();
};

} // namespace cg::graph

#endif // __HelixProxy_h