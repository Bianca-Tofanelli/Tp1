#include "HelixProxy.h"
#include "graph/SceneWindow.h"
#include "graphics/Assets.h"
#include <cmath>
#include <algorithm>

namespace cg::graph
{

constexpr float PI = 3.14159265358979323846f;

HelixProxy::HelixProxy()
{
  // Configura a geratriz e gera a malha inicial
  m_polyline.update(m_polylineParams);
  updateMesh();
}

void HelixProxy::updateMesh()
{
  // 1. Atualiza os pontos da geratriz
  m_polyline.update(m_polylineParams);
  // 2. Constrói a malha de triângulos 3D da espiral
  buildSpiralMesh();
}

void HelixProxy::buildSpiralMesh()
{
  const auto& polyPoints = m_polyline.points();
  int np = static_cast<int>(polyPoints.size());
  if (np < 2) return;

  // Caixa envolvente (AABB) da geratriz
  float bx = 0.0f, by = 0.0f;
  m_polyline.getBoundingBox(bx, by);

  // Clamping dos parâmetros dentro das faixas limites do roteiro
  float we = std::max(m_helixParams.initialWidth, 2.0f * bx);
  float re = std::clamp(m_helixParams.revolutions, 0.1f, 20.0f);
  int nes = std::clamp(m_helixParams.subdivisions, 3, 40);
  float dh = std::clamp(m_helixParams.deltaHeight, 0.0f, 10.0f);
  float dw = std::clamp(m_helixParams.deltaWidth, 0.0f, 10.0f);

  // Número total de rotações e passos da geratriz
  int ner = std::max(static_cast<int>(std::floor(re + 0.5f)), 1) * nes;
  int numSteps = ner + 1;
  float totalAngleRad = (360.0f * re) * (PI / 180.0f);

  float de = (we - bx) * 0.5f; // Distância inicial ao eixo de rotação
  float totalDeltaH = re * dh;
  float totalDeltaW = re * dw;

  auto mesh = new TriangleMesh();

  // Construção dos Vértices da varredura
  for (int k = 0; k < numSteps; ++k)
  {
    float factor = static_cast<float>(k) / static_cast<float>(ner);
    float currentAngle = totalAngleRad * factor;
    float cosA = std::cos(currentAngle);
    float sinA = std::sin(currentAngle);

    float currentH = totalDeltaH * factor;
    float currentW = de + totalDeltaW * factor;

    for (int i = 0; i < np; ++i)
    {
      const vec3& p = polyPoints[i];
      // Posição no referencial da espiral (rotação em torno de Y e translações em X/Y)
      float x = (p.x + currentW) * cosA + p.z * sinA;
      float y = p.y + currentH;
      float z = -(p.x + currentW) * sinA + p.z * cosA;

      mesh->vertices.push_back(vec3(x, y, z));
    }
  }

  // Conexão dos Triângulos entre passos adjacentes
  for (int k = 0; k < ner; ++k)
  {
    int offsetCurr = k * np;
    int offsetNext = (k + 1) * np;

    int segments = m_polyline.isClosed() ? np : np - 1;
    for (int i = 0; i < segments; ++i)
    {
      int iNext = (i + 1) % np;

      int v0 = offsetCurr + i;
      int v1 = offsetCurr + iNext;
      int v2 = offsetNext + i;
      int v3 = offsetNext + iNext;

      // 2 triângulos por quad da malha
      mesh->triangles.push_back({v0, v1, v2});
      mesh->triangles.push_back({v1, v3, v2});
    }
  }

  // Cálculo automático das normais para iluminação
  mesh->computeNormals();

  // Define a nova malha no mapeador do primitivo
  if (auto mapper = triangleMeshMapper())
  {
    mapper->setMesh(mesh);
  }
}

bool HelixProxy::inspect(SceneWindow& window)
{
  bool changed = false;

  if (ImGui::CollapsingHeader("Geratriz (Polyline)", ImGuiTreeNodeFlags_DefaultOpen))
  {
    int typeIdx = static_cast<int>(m_polylineParams.type);
    const char* types[] = { "Polígono Regular", "Arco Circular" };
    if (ImGui::Combo("Tipo", &typeIdx, types, IM_ARRAYSIZE(types))) {
      m_polylineParams.type = static_cast<GeneratrixType>(typeIdx);
      changed = true;
    }

    int minPts = (m_polylineParams.type == GeneratrixType::RegularPolygon) ? 3 : 2;
    if (ImGui::SliderInt("Número de Pontos", &m_polylineParams.numPoints, minPts, 40)) {
      changed = true;
    }

    if (m_polylineParams.type == GeneratrixType::CircularArc) {
      if (ImGui::SliderFloat("Ângulo do Arco", &m_polylineParams.arcAngle, 1.0f, 360.0f, "%.1f deg")) {
        changed = true;
      }
      if (m_polylineParams.arcAngle < 360.0f) {
        if (ImGui::Checkbox("Fechar Arco (Setor)", &m_polylineParams.isClosed)) {
          changed = true;
        }
      }
    }

    if (ImGui::Checkbox("Extrusão Interna", &m_polylineParams.enableExtrusion)) {
      changed = true;
    }
    if (m_polylineParams.enableExtrusion) {
      if (ImGui::SliderFloat("Fator Extrusão (ep)", &m_polylineParams.extrusionFactor, 0.1f, 0.9f, "%.2f")) {
        changed = true;
      }
    }

    if (ImGui::SliderFloat("Escala (sp)", &m_polylineParams.scale, 0.1f, 5.0f, "%.2f")) {
      changed = true;
    }
    if (ImGui::SliderFloat("Rotação (tp)", &m_polylineParams.rotationAngle, 0.0f, 360.0f, "%.1f deg")) {
      changed = true;
    }
  }

  if (ImGui::CollapsingHeader("Parâmetros da Espiral", ImGuiTreeNodeFlags_DefaultOpen))
  {
    if (ImGui::SliderFloat("Largura Inicial (we)", &m_helixParams.initialWidth, 1.0f, 100.0f, "%.1f")) {
      changed = true;
    }
    if (ImGui::SliderFloat("Revoluções (re)", &m_helixParams.revolutions, 0.1f, 20.0f, "%.2f")) {
      changed = true;
    }
    if (ImGui::SliderInt("Subdivisões (n_s)", &m_helixParams.subdivisions, 3, 40)) {
      changed = true;
    }
    if (ImGui::SliderFloat("Incr. Altura (dh)", &m_helixParams.deltaHeight, 0.0f, 10.0f, "%.2f")) {
      changed = true;
    }
    if (ImGui::SliderFloat("Incr. Largura (dw)", &m_helixParams.deltaWidth, 0.0f, 10.0f, "%.2f")) {
      changed = true;
    }
    if (ImGui::Checkbox("Tampas nas Pontas", &m_helixParams.makeCaps)) {
      changed = true;
    }
  }

  // Exportação para formato OBJ (Tarefa A6)
  ImGui::Separator();
  ImGui::InputText("Nome Asset", m_meshName, sizeof(m_meshName));
  if (ImGui::Button("Salvar como .OBJ"))
  {
    if (auto mapper = triangleMeshMapper())
    {
      if (auto mesh = mapper->mesh())
      {
        std::string path = "assets/meshes/" + std::string(m_meshName) + ".obj";
        // Lógica de salvar o arquivo OBJ
      }
    }
  }

  if (changed)
  {
    updateMesh();
  }

  return changed;
}

} // namespace cg::graph