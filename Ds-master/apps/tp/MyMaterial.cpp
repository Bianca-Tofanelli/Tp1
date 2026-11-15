#include "MyMaterial.h"
#include <cmath>

namespace cg
{ // begin namespace cg

MyMaterial::MyMaterial(const Color& diffuseColor,
                       const Color& specularColor,
                       float r,
                       float m):
  Material{diffuseColor},
  Od{diffuseColor},
  Os{specularColor},
  roughness{r},
  metalness{m}
{
  syncBaseFields();
}

void
MyMaterial::syncBaseFields()
{
  float m = math::clamp(metalness, 0.0f, 1.0f);
  float r = math::clamp(roughness, 0.001f, 1.0f);

  ambient = Color::black;
  diffuse = (1.0f - m) * Od;
  spot = (1.0f - m) * sMin * Color::white + m * Os;
  specular = Os;
  shine = math::max(1.0f, (1.0f - r) * 200.0f);
}

Color
MyMaterial::shade(const vec3f& N,
                  const vec3f& V,
                  const vec3f& L,
                  const Color& lightColor) const
{
  constexpr float pi = 3.14159265358979323846f;

  float NL = math::max(N.dot(L), 0.0f);
  float NV = math::max(-N.dot(V), 0.0f);

  if (NL <= 0.0f || NV <= 0.0f)
    return Color::black;

  // Vetor a meio caminho entre L (P -> luz) e -V (P -> observador)
  vec3f H = (L - V).versor();
  float NH = math::max(N.dot(H), 0.0f);
  float LH = math::max(L.dot(H), 0.0f);

  float m = math::clamp(metalness, 0.0f, 1.0f);
  float r = math::clamp(roughness, 0.001f, 1.0f);

  // Refletância na incidência normal (interpolação dielétrico <-> metal)
  Color F0 = (1.0f - m) * sMin * Color::white + m * Os;

  // Termo de Fresnel - Aproximação de Schlick (Equação 3)
  float oneMinusLH = 1.0f - LH;
  float oneMinusLH5 = oneMinusLH * oneMinusLH * oneMinusLH * oneMinusLH * oneMinusLH;
  Color F = F0 + (Color::white - F0) * oneMinusLH5;

  // Termo Geométrico G(L, V)
  float k = (r + 1.0f) * (r + 1.0f) / 8.0f;
  float G1_L = NL / (NL * (1.0f - k) + k);
  float G1_V = NV / (NV * (1.0f - k) + k);
  float G = G1_L * G1_V;

  // Função de Distribuição Normal D(H) - Trowbridge-Reitz
  float alpha = r * r;
  float alpha2 = alpha * alpha;
  float denom = NH * NH * (alpha2 - 1.0f) + 1.0f;
  float D = alpha2 / (pi * denom * denom);

  // Parcela difusa (nula para 100% metal) e parcela especular (Equação 2)
  Color fd = ((1.0f - m) / pi) * Od;
  Color fs = (G * D / (4.0f * NL * NV)) * F;

  // Equação (1)
  return (pi * NL) * lightColor * (fd + fs);
}

Color
MyMaterial::indirectSpecular(const vec3f& N, const vec3f& V) const
{
  float cosPhi = math::max(-N.dot(V), 0.0f);
  float m = math::clamp(metalness, 0.0f, 1.0f);
  float r = math::clamp(roughness, 0.0f, 1.0f);

  Color F0 = (1.0f - m) * sMin * Color::white + m * Os;

  // Aproximação analítica de Karis (Equação 4)
  float k1 = -1.0f * r + 1.0f;
  float k2 = -0.0275f * r + 0.0425f;
  float k3 = -0.572f * r + 1.04f;
  float k4 = 0.022f * r - 0.04f;

  float a = math::min(k1 * k1, std::exp2(-9.28f * cosPhi)) * k1 + k2;
  float A = -1.04f * a + k3;
  float B = 1.04f * a + k4;

  Color Or = A * F0 + B * Color::white;

  Or.r = math::max(Or.r, 0.0f);
  Or.g = math::max(Or.g, 0.0f);
  Or.b = math::max(Or.b, 0.0f);
  return Or;
}

} // end namespace cg