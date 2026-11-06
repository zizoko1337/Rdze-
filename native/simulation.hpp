#pragma once
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <limits>
#include <vector>

namespace rdzen {
constexpr float PI = 3.14159265359f;
struct Vec3 { float x=0,y=0,z=0; };
inline Vec3 operator+(Vec3 a,Vec3 b){return {a.x+b.x,a.y+b.y,a.z+b.z};}
inline Vec3 operator-(Vec3 a,Vec3 b){return {a.x-b.x,a.y-b.y,a.z-b.z};}
inline Vec3 operator*(Vec3 a,float s){return {a.x*s,a.y*s,a.z*s};}
inline float dot(Vec3 a,Vec3 b){return a.x*b.x+a.y*b.y+a.z*b.z;}
inline Vec3 cross(Vec3 a,Vec3 b){return {a.y*b.z-a.z*b.y,a.z*b.x-a.x*b.z,a.x*b.y-a.y*b.x};}
inline float length(Vec3 v){return std::sqrt(dot(v,v));}
inline Vec3 normalize(Vec3 v){float l=length(v);return l>1e-6f?v*(1/l):Vec3{0,0,1};}
struct Random {
  uint32_t state=42017;
  uint32_t next(){state^=state<<13;state^=state>>17;state^=state<<5;return state;}
  float unit(){return (next()>>8)*(1.f/16777216.f);}
};
struct Enemy {float x,z,hp,speed,size,phase; uint32_t generation=1;};
struct Turret {float x,z,angle=0,cooldown=0; int target=-1; uint32_t generation=0;};
struct Projectile {Vec3 pos,previous; int target=-1; uint32_t generation=0; float speed=60,life=0;};
struct Instance {float x,y,z,sx,sy,sz,yaw,r,g,b,a,extra;};
static_assert(sizeof(Instance)==48);

// Fixed-size uniform grid. No heap allocation during update/query. Indices stay stable.
class Grid {
public:
  static constexpr int side=64;
  static constexpr float cell=8, origin=-256;
  std::array<int,side*side> heads{};
  std::vector<int> next;
  void resize(size_t n){next.resize(n);}
  static int coordinate(float p){return std::clamp(int(std::floor((p-origin)/cell)),0,side-1);}
  void rebuild(const std::vector<Enemy>& enemies){
    heads.fill(-1);
    for(int i=0;i<int(enemies.size());++i){if(enemies[i].hp<=0){next[i]=-1;continue;} int c=coordinate(enemies[i].x)+coordinate(enemies[i].z)*side;next[i]=heads[c];heads[c]=i;}
  }
  int nearest(float x,float z,float range,const std::vector<Enemy>& enemies,uint64_t* checks=nullptr)const{
    int result=-1;float best=range*range;
    int x0=coordinate(x-range),x1=coordinate(x+range),z0=coordinate(z-range),z1=coordinate(z+range);
    for(int cz=z0;cz<=z1;++cz)for(int cx=x0;cx<=x1;++cx){
      float lx=origin+cx*cell,lz=origin+cz*cell;
      float dx=std::max({lx-x,0.f,x-(lx+cell)}),dz=std::max({lz-z,0.f,z-(lz+cell)});
      if(dx*dx+dz*dz>best)continue;
      for(int i=heads[cx+cz*side];i>=0;i=next[i]){const auto& e=enemies[i];if(checks)++*checks;if(e.hp<=0)continue;float ex=e.x-x,ez=e.z-z,d=ex*ex+ez*ez;if(d<best||(d==best&&(result<0||i<result))){best=d;result=i;}}
    }
    return result;
  }
};
inline int nearestBrute(float x,float z,float range,const std::vector<Enemy>& enemies){
  int result=-1;float best=range*range;for(int i=0;i<int(enemies.size());++i){const auto&e=enemies[i];if(e.hp<=0)continue;float dx=e.x-x,dz=e.z-z,d=dx*dx+dz*dz;if(d<best||(d==best&&(result<0||i<result))){result=i;best=d;}}return result;
}
inline bool validTarget(const Turret&t,const std::vector<Enemy>&enemies,float range=48){
  if(t.target<0||t.target>=int(enemies.size()))return false;const auto&e=enemies[t.target];float dx=e.x-t.x,dz=e.z-t.z;
  return e.hp>0&&e.generation==t.generation&&dx*dx+dz*dz<=range*range;
}
inline bool sweepSphere(Vec3 a,Vec3 b,Vec3 center,float radius){Vec3 ab=b-a;float sq=dot(ab,ab);float t=sq>1e-10f?std::clamp(dot(center-a,ab)/sq,0.f,1.f):0;Vec3 d=a+ab*t-center;return dot(d,d)<=radius*radius;}

class Simulation {
public:
  std::vector<Enemy> enemies;
  std::vector<Turret> turrets;
  std::vector<Projectile> projectiles;
  Grid grid; Random random;
  uint64_t hits=0,kills=0,targetChecks=0,coreContacts=0;
  float time=0,spawnInner=149,spawnOuter=154;
  Simulation(int enemyCount,int projectileCount,int turretCount){
    enemies.resize(enemyCount);projectiles.resize(projectileCount);turrets.reserve(1024);grid.resize(enemyCount);
    for(auto&e:enemies)respawn(e,true);
    // Multiple rings provide meaningful local queries throughout the arena.
    for(int i=0;i<turretCount;++i){float a=i*2.39996323f,r=16+std::sqrt(float(i)/std::max(turretCount,1))*80;turrets.push_back({std::sin(a)*r,std::cos(a)*r});}
    grid.rebuild(enemies);targetTurrets(0);
    for(auto&p:projectiles){launch(p);float f=random.unit();if(p.target>=0){auto&e=enemies[p.target];p.pos=p.pos+(Vec3{e.x,e.size*.45f,e.z}-p.pos)*f;p.previous=p.pos;}}
  }
  void respawn(Enemy&e,bool initial=false){float a=random.unit()*2*PI,r=initial?20+std::sqrt(random.unit())*132:spawnInner+random.unit()*(spawnOuter-spawnInner);e.x=std::sin(a)*r;e.z=std::cos(a)*r;e.hp=2000;e.speed=1.5f+random.unit()*3;e.size=1.8f+random.unit()*1.6f;e.phase=random.unit()*2*PI;if(!initial)++e.generation;}
  void denseCrowd(){spawnInner=12;spawnOuter=42;for(auto&e:enemies)respawn(e);for(size_t i=0;i<turrets.size();++i){float a=float(i)*2.39996323f,r=12+float(i%8)*2;turrets[i].x=std::sin(a)*r;turrets[i].z=std::cos(a)*r;turrets[i].target=-1;turrets[i].cooldown=0;}grid.rebuild(enemies);targetTurrets(0);for(auto&p:projectiles)launch(p);}
  void targetTurrets(float dt){
    for(auto&t:turrets){t.cooldown-=dt;if(!validTarget(t,enemies)){
      if(t.cooldown>0)continue;t.target=grid.nearest(t.x,t.z,48,enemies,&targetChecks);t.generation=t.target>=0?enemies[t.target].generation:0;t.cooldown=.1f;
    }if(t.target>=0){auto&e=enemies[t.target];t.angle=std::atan2(e.x-t.x,e.z-t.z);}}
  }
  void launch(Projectile&p){
    if(turrets.empty()||enemies.empty()){p.target=-1;return;}
    auto&t=turrets[random.next()%turrets.size()];int id=t.target;
    if(!validTarget(t,enemies))id=int(random.next()%enemies.size());
    p.target=id;p.generation=enemies[id].generation;p.pos={t.x,3.7f,t.z};p.previous=p.pos;p.speed=45+random.unit()*45;p.life=6;
  }
  void step(float dt){
    time+=dt;
    for(auto&e:enemies){if(e.hp<=0){respawn(e);continue;}float sq=e.x*e.x+e.z*e.z;if(sq<10*10){++coreContacts;respawn(e);continue;}float k=e.speed*dt/std::sqrt(sq);e.x-=e.x*k;e.z-=e.z*k;}
    grid.rebuild(enemies);targetTurrets(dt);
    for(auto&p:projectiles){
      if(p.target<0||p.target>=int(enemies.size())||enemies[p.target].generation!=p.generation||enemies[p.target].hp<=0||p.life<=0){launch(p);continue;}
      auto&e=enemies[p.target];Vec3 target{e.x,e.size*.45f,e.z};Vec3 delta=target-p.pos;float dist=length(delta),travel=p.speed*dt;p.previous=p.pos;
      p.pos=p.pos+delta*(std::min(travel,dist)/std::max(dist,.00001f));p.life-=dt;
      if(sweepSphere(p.previous,p.pos,target,.4f)){++hits;e.hp-=8;if(e.hp<=0)++kills;launch(p);}
    }
  }
  void enemyInstances(std::vector<Instance>&out)const{
    out.clear();for(auto&e:enemies){float bounce=std::abs(std::sin(time*8+e.phase))*.12f;out.push_back({e.x,e.size*.48f+bounce,e.z,e.size,e.size,1,0,1,1,1,1,e.phase});}
  }
  void projectileInstances(std::vector<Instance>&out)const{
    // Pass the direction vector directly. Avoid atan2 for every projectile and
    // the inverse sin/cos in the shader; GPU normalizes the two components.
    out.clear();for(auto&p:projectiles){if(p.target<0)continue;Vec3 d=p.pos-p.previous;float l=std::min(1.2f,std::max(.35f,length(d)*1.4f));out.push_back({p.pos.x,p.pos.y,p.pos.z,.065f,l,1,d.x,1,.16f,.07f,.75f,d.z});}
  }
};
}
