#include "simulation.hpp"
#include <chrono>
#include <iostream>
#include <stdexcept>
#include <string>
using namespace rdzen;
void require(bool ok,const char*message){if(!ok)throw std::runtime_error(message);}
int main(){try{
  Simulation s(5000,50000,256);Random r;
  for(int i=0;i<10000;++i){float x=(r.unit()-.5f)*400,z=(r.unit()-.5f)*400,range=1+r.unit()*80;require(s.grid.nearest(x,z,range,s.enemies)==nearestBrute(x,z,range,s.enemies),"Grid nearest differs from exhaustive search");}
  std::cout<<"PASS: 10000 grid queries match exhaustive search including map edges.\n";
  std::vector<Enemy> e={{20,0,10,0,1,0,1},{22,0,10,0,1,0,1}};Turret t{0,0,0,0,1,1};require(validTarget(t,e),"Valid lock lost");e[0].x=1;require(validTarget(t,e),"Closer enemy breaks lock");e[1].generation++;require(!validTarget(t,e),"Recycled slot keeps stale target");e[1].generation--;e[1].x=100;require(!validTarget(t,e),"Out of range target kept");e[1].x=22;e[1].hp=0;require(!validTarget(t,e),"Dead target kept");
  std::cout<<"PASS: target lock, range, death and generation-safe reuse.\n";
  require(sweepSphere({0,0,0},{100,0,0},{50,0,0},1),"Fast shot tunnels through target");require(!sweepSphere({0,2,0},{100,2,0},{50,0,0},1),"False collision");require(sweepSphere({1,0,0},{1,0,0},{0,0,0},1),"Zero displacement collision");
  std::cout<<"PASS: swept hit detection (fast projectiles and zero displacement).\n";
  auto*ep=s.enemies.data();auto*pp=s.projectiles.data();auto*gp=s.grid.next.data();
  for(int i=0;i<600;++i)s.step(1.f/60);
  require(s.hits>0&&s.kills>0,"No combat in stress test");require(ep==s.enemies.data()&&pp==s.projectiles.data()&&gp==s.grid.next.data(),"Simulation storage reallocated");
  for(auto&p:s.projectiles)require(std::isfinite(p.pos.x)&&std::isfinite(p.pos.y)&&std::isfinite(p.pos.z),"Nonfinite projectile");
  std::cout<<"PASS: 10 seconds / 5000 enemies / 50000 moving homing projectiles, stable storage, hits="<<s.hits<<", kills="<<s.kills<<".\n";
  // Same query workload, only acceleration structure changes.
  std::vector<Vec3> queries;for(int i=0;i<20000;++i)queries.push_back({(r.unit()-.5f)*260,0,(r.unit()-.5f)*260});
  uint64_t checksumA=0,checksumB=0;auto now=[](){return std::chrono::steady_clock::now();};auto a=now();for(auto q:queries)checksumA+=nearestBrute(q.x,q.z,48,s.enemies)+1;auto b=now();for(auto q:queries)checksumB+=s.grid.nearest(q.x,q.z,48,s.enemies)+1;auto c=now();require(checksumA==checksumB,"Benchmark correctness mismatch");
  double brute=std::chrono::duration<double,std::milli>(b-a).count(),grid=std::chrono::duration<double,std::milli>(c-b).count();
  std::cout<<"QUERY_BENCHMARK brute_ms="<<brute<<" grid_ms="<<grid<<" speedup="<<brute/grid<<" checksum="<<checksumA<<"\n";
  return 0;
}catch(const std::exception&e){std::cerr<<e.what()<<'\n';return 1;}}
