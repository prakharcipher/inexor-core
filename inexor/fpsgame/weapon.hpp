#pragma once



#include "inexor/fpsgame/fpsent.hpp"
#include "inexor/fpsgame/ai.hpp"
#include "inexor/shared/geom.hpp"

namespace game {

extern void shoot(fpsent *d, const vec &targ);
extern void shoteffects(int gun, const vec &from, const vec &to, fpsent *d, bool local, int id, int prevaction);
extern void explode(bool local, fpsent *owner, const vec &v, dynent *safe, int dam, int gun);
extern void explodeeffects(int gun, fpsent *d, bool local, int id = 0);
extern void damageeffect(int damage, fpsent *d, bool thirdperson = true);
extern void gibeffect(int damage, const vec &vel, fpsent *d);
extern float intersectdist;
extern bool intersect(dynent *d, const vec &from, const vec &to, float &dist = intersectdist);
extern dynent *intersectclosest(const vec &from, const vec &to, fpsent *at, float &dist = intersectdist);

extern int getweapon(const char *name);
extern void removeweapons(fpsent *owner);
extern void updateweapons(int curtime);
extern void gunselect(int gun, fpsent *d);
extern void weaponswitch(fpsent *d);
extern void avoidweapons(ai::avoidset &obstacles, float radius);

}
