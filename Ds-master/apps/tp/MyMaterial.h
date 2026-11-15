#ifndef __MyMaterial_h
#define __MyMaterial_h

#include "graphics/Material.h"
#include "math/Vector3.h"

namespace cg
{ // begin namespace cg

class MyMaterial: public Material
{
public:
  Color Od;        // Cor de reflexão difusa
  Color Os;        // Cor de reflexão especular
  float roughness; // Rugosidade r em [0, 1]
  float metalness; // Metalicidade m em [0, 1] (0 = dielétrico, 1 = metal)

  static constexpr float sMin = 0.04f;

  MyMaterial(const Color& diffuseColor = Color::white,
             const Color& specularColor = Color::white,
             float r = 0.5f,
             float m = 0.0f);

  // Tonalização direta PBR para uma fonte de luz (Equações 1, 2 e 3)
  // N: normal unitária apontando para o lado do observador
  // V: direção unitária do raio incidente (do observador para P)
  // L: direção unitária de P para a luz
  Color shade(const vec3f& N,
              const vec3f& V,
              const vec3f& L,
              const Color& lightColor) const;

  // Cálculo da cor de reflexão especular indireta Or (Equação 4 - Karis)
  Color indirectSpecular(const vec3f& N, const vec3f& V) const;

  // Mantém os campos base da classe Material sincronizados
  void syncBaseFields();
};

} // end namespace cg

#endif // __MyMaterial_h