#include "SweepMeshProxy.h"
#include "geometry/TriangleMesh.h"
#include "graphics/Assets.h"
#include <cmath>
#include <algorithm>

namespace cg::graph
{

constexpr float PI = 3.14159265358979323846f;

inline TriangleMesh& createInitialSweepMesh()
{
  TriangleMesh::Data data{};
  data.vertexCount = 0;
  data.triangleCount = 0;
  data.vertices = nullptr;
  data.vertexNormals = nullptr;
  data.triangles = nullptr;
  return *new TriangleMesh{std::move(data)};
}

SweepMeshProxy::SweepMeshProxy():
  PrimitiveProxy{*new TriangleMeshMapper{createInitialSweepMesh()}}
{
  m_polyline.update(m_polylineParams);
  updateMesh();
}

void SweepMeshProxy::updateMesh()
{
  m_polyline.update(m_polylineParams);
  buildSweepMesh();
}

void SweepMeshProxy::buildSweepMesh()
{
  const auto& polyPoints = m_polyline.points();
  int np = m_polyline.size();
  if (np < 2) return;

  float bx = 0.0f, by = 0.0f;
  m_polyline.getBoundingBox(bx, by);

  // Utilização segura de std::max e std::min em vez de std::clamp (C++14 safe)
  float lv = std::max(1.0f, std::min(m_sweepParams.length, 50.0f));
  float ovw = std::max(-bx, std::min(m_sweepParams.offsetW, bx));
  float ovh = std::max(-by, std::min(m_sweepParams.offsetH, by));
  int nvs = std::max(1, std::min(m_sweepParams.subdivisions, 50));
  float svb = std::max(0.1f, std::min(m_sweepParams.scaleBegin, 5.0f));
  float sve = std::max(0.1f, std::min(m_sweepParams.scaleEnd, 5.0f));
  float rv = std::max(-2.0f, std::min(m_sweepParams.twist, 2.0f));

  int numSteps = nvs + 1;
  float totalAngleRad = (360.0f * rv) * (PI / 180.0f);

  int segments = m_polyline.isClosed() ? np : np - 1;
  
  bool hasCaps = m_sweepParams.makeCaps && m_polyline.isClosed() && !m_polylineParams.enableExtrusion;
  int vertexCount = numSteps * np + (hasCaps ? 2 * np : 0);
  int triangleCount = 2 * nvs * segments + (hasCaps ? 2 * (np - 2) : 0);

  TriangleMesh::Data data{};
  data.vertexCount = vertexCount;
  data.triangleCount = triangleCount;
  data.vertices = new vec3f[vertexCount];
  data.vertexNormals = nullptr;
  data.triangles = new TriangleMesh::Triangle[triangleCount];

  int vIdx = 0;
  for (int k = 0; k < numSteps; ++k)
  {
    float factor = static_cast<float>(k) / static_cast<float>(nvs);
    float sv = svb + (sve - svb) * factor;
    float dz = lv * factor;
    
    float angle = totalAngleRad * factor;
    float cosA = std::cos(angle);
    float sinA = std::sin(angle);

    for (int i = 0; i < np; ++i)
    {
      const auto& p = polyPoints[i];
      
      float sx = p.x * sv;
      float sy = p.y * sv;
      float sz = p.z * sv;
      
      float tx = sx + ovw;
      float ty = sy + ovh;
      float tz = sz + dz;
      
      float rx = tx * cosA - ty * sinA;
      float ry = tx * sinA + ty * cosA;
      float rz = tz;

      data.vertices[vIdx++] = vec3f{rx, ry, rz};
    }
  }

  int tIdx = 0;
  for (int k = 0; k < nvs; ++k)
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

  if (hasCaps)
  {
    int bottomOffset = vIdx;
    for (int i = 0; i < np; ++i) {
      data.vertices[vIdx++] = data.vertices[i];
    }
    for (int i = 1; i < np - 1; ++i) {
      data.triangles[tIdx++].setVertices(bottomOffset, bottomOffset + i + 1, bottomOffset + i);
    }

    int topOffset = vIdx;
    int lastRingOffset = nvs * np;
    for (int i = 0; i < np; ++i) {
      data.vertices[vIdx++] = data.vertices[lastRingOffset + i];
    }
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

bool SweepMeshProxy::inspect(SceneWindow& window)
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

  if (ImGui::CollapsingHeader("Parâmetros da Varredura (Sweep)", ImGuiTreeNodeFlags_DefaultOpen))
  {
    float bx = 0.0f, by = 0.0f;
    m_polyline.getBoundingBox(bx, by);

    if (ImGui::SliderFloat("Comprimento (lv)", &m_sweepParams.length, 1.0f, 50.0f, "%.1f")) {
      changed = true;
    }
    if (ImGui::SliderFloat("Desloc. Horizontal (ov_w)", &m_sweepParams.offsetW, -bx, bx, "%.2f")) {
      changed = true;
    }
    if (ImGui::SliderFloat("Desloc. Vertical (ov_h)", &m_sweepParams.offsetH, -by, by, "%.2f")) {
      changed = true;
    }
    if (ImGui::SliderInt("Subdivisões (nv_s)", &m_sweepParams.subdivisions, 1, 50)) {
      changed = true;
    }
    if (ImGui::SliderFloat("Escala Inicial (sv_b)", &m_sweepParams.scaleBegin, 0.1f, 5.0f, "%.2f")) {
      changed = true;
    }
    if (ImGui::SliderFloat("Escala Final (sv_e)", &m_sweepParams.scaleEnd, 0.1f, 5.0f, "%.2f")) {
      changed = true;
    }
    if (ImGui::SliderFloat("Torção (rv)", &m_sweepParams.twist, -2.0f, 2.0f, "%.2f voltas")) {
      changed = true;
    }
    if (ImGui::Checkbox("Tampas nas Pontas", &m_sweepParams.makeCaps)) {
      changed = true;
    }
  }

  if (changed)
  {
    updateMesh();
  }

  return changed;
}

} // namespace cg::graph