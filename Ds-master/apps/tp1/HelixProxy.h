#ifndef __HelixProxy_h
#define __HelixProxy_h

#include "graph/PrimitiveProxy.h"
#include "graph/SceneWindow.h"
#include "Polyline.h"

namespace cg::graph
{

class HelixProxy: public PrimitiveProxy
{
public:
  // Parâmetros do processo de varredura de Espirais (Seção 1 do roteiro)
  struct Parameters
  {
    float initialWidth = 4.0f;    // we: largura inicial (2*bx <= we <= 100)
    float revolutions = 1.0f;     // re: número de revoluções (0.1 <= re <= 20)
    int subdivisions = 16;        // n_e^s: subdivisões por revolução (3 <= n_s <= 40)
    float deltaHeight = 0.0f;     // delta_e^h: incremento de altura por revolução (0 <= delta_h <= 10)
    float deltaWidth = 0.0f;      // delta_e^w: incremento de largura por revolução (0 <= delta_w <= 10)
    bool makeCaps = true;         // tampas nas extremidades
  };

  HelixProxy();

  // Reconstrói a malha de triângulos com base na geratriz e nos parâmetros atuais
  void updateMesh();

  // Renderiza a interface ImGui no painel Inspector
  bool inspect(SceneWindow& window) override;

  // Getters/Setters dos parâmetros e da geratriz
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