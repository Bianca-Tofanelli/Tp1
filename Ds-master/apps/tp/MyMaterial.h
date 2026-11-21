#ifndef __MyMaterial_h
#define __MyMaterial_h

#include "graphics/Material.h"
#include "math/Vector3.h"

namespace cg
{ // begin namespace cg

class MyMaterial: public Material
{
public:
  Color Od;        
  Color Os;       
  float roughness; 
  float metalness; 

  static constexpr float sMin = 0.04f;

  MyMaterial(const Color& diffuseColor = Color::white,
             const Color& specularColor = Color::white,
             float r = 0.5f,
             float m = 0.0f);

  Color shade(const vec3f& N,
              const vec3f& V,
              const vec3f& L,
              const Color& lightColor) const;

  Color indirectSpecular(const vec3f& N, const vec3f& V) const;

  void syncBaseFields();
};

} // end namespace cg

#endif // __MyMaterial_h