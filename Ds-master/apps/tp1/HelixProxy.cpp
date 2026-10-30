#include "HelixProxy.h"
#include "graphics/Assets.h"
#include <cmath>
#include <algorithm>
#include <utility>
#include "ObjExporter.h"

namespace cg::graph
{

constexpr float PI = 3.14159265358979323846f;

// Função auxiliar para criar uma malha inicial vazia para o TriangleMeshMapper
inline TriangleMesh& createInitialMesh()
{
  TriangleMesh::Data data{};
  data.vertexCount = 0;
  data.triangleCount = 0;
  data.vertices = nullptr;
  data.vertexNormals = nullptr;
  data.triangles = nullptr;
  return *new TriangleMesh{std::move(data)};
}

// Passa a malha inicial para o construtor do TriangleMeshMapper (resolve o erro 289)
HelixProxy::HelixProxy():
  PrimitiveProxy{*new TriangleMeshMapper{createInitialMesh()}}
{
  m_polyline.update(m_polylineParams);
  updateMesh();
}

void HelixProxy::updateMesh()
{
  m_polyline.update(m_polylineParams);
  buildSpiralMesh();
}

void HelixProxy::buildSpiralMesh()
{
  const auto& polyPoints = m_polyline.points();
  int np = static_cast<int>(polyPoints.size());
  if (np < 2) return;

  float bx = 0.0f, by = 0.0f;
  m_polyline.getBoundingBox(bx, by);

  // OPERADORES DE MULTIPLICAÇÃO E SUBTRAÇÃO CORRIGIDOS
  float we = std::max(m_helixParams.initialWidth, 2.0f * bx);
  float re = std::clamp(m_helixParams.revolutions, 0.1f, 20.0f);
  int nes = std::clamp(m_helixParams.subdivisions, 3, 40);
  float dh = std::clamp(m_helixParams.deltaHeight, 0.0f, 10.0f);
  float dw = std::clamp(m_helixParams.deltaWidth, 0.0f, 10.0f);

  int ner = std::max(static_cast<int>(std::floor(re + 0.5f)), 1) * nes;
  int numSteps = ner + 1;
  float totalAngleRad = (360.0f * re) * (PI / 180.0f);

  float de = (we - bx) * 0.5f;
  float totalDeltaH = re * dh;
  float totalDeltaW = re * dw;

  // Verificação para as tampas nas extremidades
  int segments = m_polyline.isClosed() ? np : np - 1;
  bool hasCaps = m_helixParams.makeCaps && m_polyline.isClosed() && !m_polylineParams.enableExtrusion;

  // Atualização da contagem de vértices e triângulos incluindo as tampas
  int vertexCount = numSteps * np + (hasCaps ? 2 * np : 0);
  int triangleCount = 2 * ner * segments + (hasCaps ? 2 * (np - 2) : 0);

  TriangleMesh::Data data{};
  data.vertexCount = vertexCount;
  data.triangleCount = triangleCount;
  data.vertices = new vec3f[vertexCount];
  data.vertexNormals = nullptr;
  data.triangles = new TriangleMesh::Triangle[triangleCount];

  // 1. Preenchimento dos Vértices da Espiral
  int vIdx = 0;
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
      float x = (p.x + currentW) * cosA + p.z * sinA;
      float y = p.y + currentH;
      float z = -(p.x + currentW) * sinA + p.z * cosA;

      data.vertices[vIdx++] = vec3f{x, y, z};
    }
  }

  // 2. Preenchimento dos Triângulos da Malha
  int tIdx = 0;
  for (int k = 0; k < ner; ++k)
  {
    int offsetCurr = k * np;
    int offsetNext = (k + 1) * np;

    for (int i = 0; i < segments; ++i)
    {
      int iNext = (i + 1) % np;

      int v0 = offsetCurr + i;
      int v1 = offsetCurr + iNext;
      int v2 = offsetNext + i;
      int v3 = offsetNext + iNext;

      data.triangles[tIdx++].setVertices(v0, v1, v2);
      data.triangles[tIdx++].setVertices(v1, v3, v2);
    }
  }

  // 3. Construção das tampas
  if (hasCaps)
  {
    // Tampa inferior (início da espiral)
    int bottomOffset = vIdx;
    for (int i = 0; i < np; ++i) {
      data.vertices[vIdx++] = data.vertices[i]; // Copia a primeira face
    }
    // Triangulação em leque
    for (int i = 1; i < np - 1; ++i) {
      data.triangles[tIdx++].setVertices(bottomOffset, bottomOffset + i + 1, bottomOffset + i);
    }

    // Tampa superior (fim da espiral)
    int topOffset = vIdx;
    int lastRingOffset = ner * np; // Índice de onde começa o último anel
    for (int i = 0; i < np; ++i) {
      data.vertices[vIdx++] = data.vertices[lastRingOffset + i]; // Copia a última face
    }
    // Triangulação em leque invertida para a normal apontar para fora
    for (int i = 1; i < np - 1; ++i) {
      data.triangles[tIdx++].setVertices(topOffset, topOffset + i, topOffset + i + 1);
    }
  }

  auto mesh = new TriangleMesh{std::move(data)};
  mesh->computeNormals();

  if (auto m = mapper())
  {
    m->setMesh(*mesh);
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

  if (changed)
  {
    updateMesh();
  }

  // --- CÓDIGO DO BOTÃO DE SALVAR (TAREFA A6) ---
  ImGui::Separator();
  ImGui::InputText("Nome do Arquivo", m_meshName, IM_ARRAYSIZE(m_meshName));
  if (ImGui::Button("Salvar malha em .OBJ"))
  {
    if (auto m = mapper()->mesh()) {
      cg::exportMeshToOBJ(*m, m_meshName);
    }
  }
  // ---------------------------------------------

  return changed;
}

} // namespace cg::graph