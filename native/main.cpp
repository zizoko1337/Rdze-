#include "renderer.hpp"
#include <SDL3/SDL_main.h>
#include <psapi.h>
#include <chrono>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <numeric>
#include <sstream>

using namespace rdzen;
using Clock=std::chrono::steady_clock;
double milliseconds(Clock::time_point a,Clock::time_point b){return std::chrono::duration<double,std::milli>(b-a).count();}
double seconds(Clock::time_point a,Clock::time_point b){return std::chrono::duration<double>(b-a).count();}
struct Config {int enemies=5000,projectiles=50000,turrets=256;bool fps=false;std::string name="5000_50000_tactical";bool dense=false;};
struct Measurements {
  Config config;std::vector<double>frame,sim,prepare,cpuSubmit,present,gpu;uint64_t hits=0,kills=0,checks=0;int drawCalls=0;size_t memory=0;double wallSeconds=0,simSeconds=0;bool incomplete=false;
  Measurements(Config c):config(c){for(auto*v:{&frame,&sim,&prepare,&cpuSubmit,&present,&gpu})v->reserve(20000);}
};
double mean(const std::vector<double>&v){return v.empty()?0:std::accumulate(v.begin(),v.end(),0.0)/v.size();}
double percentile(std::vector<double>v,double p){if(v.empty())return 0;std::sort(v.begin(),v.end());return v[std::min(v.size()-1,size_t(p*(v.size()-1)))];}
std::string fixed(double n,int precision=2){std::ostringstream out;out<<std::fixed<<std::setprecision(precision)<<n;return out.str();}
struct SceneData {
  std::vector<Instance>enemies,projectiles,posts,heads,emitters,rocks;
  SceneData(){enemies.reserve(20000);projectiles.reserve(150000);posts.reserve(2048);heads.reserve(2048);emitters.reserve(1024);rocks.reserve(300);Random r;
    for(int i=0;i<180;++i){float a=r.unit()*2*PI,rad=110+r.unit()*52,s=1+r.unit()*5;rocks.push_back({std::sin(a)*rad,s*.2f,std::cos(a)*rad,s*1.3f,s,s,r.unit()*6.28f,.2f,.27f,.29f,1,0});}}
  void prepare(const Simulation&s){s.enemyInstances(enemies);s.projectileInstances(projectiles);posts.clear();heads.clear();emitters.clear();for(auto&t:s.turrets){
    posts.push_back({t.x,1.8f,t.z,.4f,3.4f,.4f,0,.65f,.72f,.75f,1,0});posts.push_back({t.x,.16f,t.z,1.8f,.3f,1.8f,0,.3f,.4f,.43f,1,0});
    heads.push_back({t.x,3.7f,t.z,1.5f,.9f,1.8f,t.angle,.8f,.82f,.84f,1,0});heads.push_back({t.x+std::sin(t.angle)*1.25f,3.7f,t.z+std::cos(t.angle)*1.25f,.22f,.22f,1.4f,t.angle,.35f,.42f,.45f,1,0});
    emitters.push_back({t.x+std::sin(t.angle)*1.95f,3.7f,t.z+std::cos(t.angle)*1.95f,.3f,.3f,.3f,0,1,.025f,.045f,1,0});}}
  void render(Renderer&r){
    Instance floor{0,-.15f,0,320,.2f,320,0,.20f,.26f,.28f,1,0};r.draw(r.cube,&floor,1,r.white,3);
    r.draw(r.cube,rocks,r.metal,0);
    Instance core[]={ {0,2,0,8,4,8,0,.5f,.62f,.65f,1,0},{0,4.1f,0,8.2f,.2f,8.2f,0,.65f,.73f,.75f,1,0},{0,5,0,.8f,1.6f,.8f,0,.2f,1,.7f,1,0} };r.draw(r.cylinder,core,3,r.metal,0);
    r.draw(r.cube,posts,r.metal,0);r.draw(r.cube,heads,r.metal,0);r.draw(r.cube,emitters,r.white,0);
    r.draw(r.quad,enemies,r.beast,1);r.draw(r.quad,projectiles,r.white,2);
  }
};
void writeReport(const std::filesystem::path&folder,const Renderer&r,const std::vector<Measurements>&results,int width,int height){
  std::ofstream summary(folder/"summary.csv");summary<<"scenario,width,height,enemies,projectiles,turrets,frames,wall_s,simulated_s,fps_mean,frame_p50_ms,frame_p95_ms,frame_p99_ms,frame_max_ms,sim_tick_mean_ms,sim_tick_p99_ms,prepare_mean_ms,submit_mean_ms,present_mean_ms,gpu_mean_ms,gpu_p99_ms,gpu_samples,draw_calls,working_set_mb,hits,kills,target_checks,incomplete\n";
  for(auto&m:results){std::ofstream raw(folder/(m.config.name+"-frames.csv"));raw<<"frame,frame_ms,prepare_ms,submit_ms,present_ms\n";for(size_t i=0;i<m.frame.size();++i)raw<<i<<','<<m.frame[i]<<','<<m.prepare[i]<<','<<m.cpuSubmit[i]<<','<<m.present[i]<<'\n';std::ofstream gpu(folder/(m.config.name+"-gpu.csv"));gpu<<"sample,gpu_ms\n";for(size_t i=0;i<m.gpu.size();++i)gpu<<i<<','<<m.gpu[i]<<'\n';
    summary<<m.config.name<<','<<width<<','<<height<<','<<m.config.enemies<<','<<m.config.projectiles<<','<<m.config.turrets<<','<<m.frame.size()<<','<<m.wallSeconds<<','<<m.simSeconds<<','<<(m.frame.empty()?0:1000/mean(m.frame))<<','<<percentile(m.frame,.5)<<','<<percentile(m.frame,.95)<<','<<percentile(m.frame,.99)<<','<<percentile(m.frame,1)<<','<<mean(m.sim)<<','<<percentile(m.sim,.99)<<','<<mean(m.prepare)<<','<<mean(m.cpuSubmit)<<','<<mean(m.present)<<','<<mean(m.gpu)<<','<<percentile(m.gpu,.99)<<','<<m.gpu.size()<<','<<m.drawCalls<<','<<m.memory/(1024.*1024.)<<','<<m.hits<<','<<m.kills<<','<<m.checks<<','<<m.incomplete<<'\n';
  }
  std::ofstream info(folder/"environment.txt");info<<"GPU: "<<r.adapter<<"\nDedicated GPU memory bytes: "<<r.vram<<"\nResolution: "<<width<<'x'<<height<<"\nRenderer: Direct3D 11, hardware device, instanced meshes and billboards\nCompiler: "<<__clang_version__<<"\nOptimization: -O3 -DNDEBUG, C++20, static C++ runtime\nSDL: "<<SDL_GetVersion()<<"\nVSync: off during automated benchmark\nSimulation: fixed 60 Hz, maximum 8 catch-up ticks per frame; time debt retained\nProjectile workload: every projectile moves, homes and performs swept collision against its assigned target; constant population is replenished\nEnemy workload: radial movement, health/damage, respawn with generation-safe handles\nTargeting: uniform grid + persistent target locks, 48 m range\nRendering: all configured enemies/projectiles submitted, no CPU frustum/occlusion culling, alpha-tested enemy sprites, additive projectiles, mipmapped textures\nNot included: crowd separation/pathfinding, arbitrary projectile-vs-all-enemy collisions, shadow maps, bloom, destructible terrain, final game UI, audio\nGPU time: asynchronous D3D11 timestamp/disjoint queries; invalid or unavailable samples are excluded\nCPU submit time includes driver calls and streaming upload; frame time includes Present\nScreenshots and report writing are excluded from measured intervals\n";
}
int main(int argc,char**argv){
  bool benchmark=false,smoke=false;int width=1920,height=1080;double duration=12,warmup=3;std::string only;
  for(int i=1;i<argc;++i){std::string arg=argv[i];if(arg=="--benchmark")benchmark=true;else if(arg=="--smoke"){benchmark=true;smoke=true;duration=2;warmup=1;}else if(arg=="--seconds"&&i+1<argc)duration=std::max(1.,std::stod(argv[++i]));else if(arg=="--width"&&i+1<argc)width=std::stoi(argv[++i]);else if(arg=="--height"&&i+1<argc)height=std::stoi(argv[++i]);else if(arg=="--only"&&i+1<argc)only=argv[++i];}
  try{
    check(CoInitializeEx(nullptr,COINIT_MULTITHREADED),"COM initialization");
    if(!SDL_Init(SDL_INIT_VIDEO|SDL_INIT_EVENTS))throw std::runtime_error(SDL_GetError());
    auto project=std::filesystem::current_path();auto assets=project/"public"/"assets";
    if(!std::filesystem::exists(assets/"alien-beast.png"))throw std::runtime_error("Run from project root using TEST_NATYWNY.bat / BENCHMARK.bat.");
    std::filesystem::path report=project/"artifacts"/"native"/(smoke?"smoke":benchmark?(only.empty()?"benchmark":only):"interactive");std::filesystem::create_directories(report);
    SDL_Window*window=SDL_CreateWindow("RDZEN | Native performance laboratory",width,height,SDL_WINDOW_RESIZABLE);
    if(!window)throw std::runtime_error(SDL_GetError());
    if(benchmark){SDL_SetWindowBordered(window,false);SDL_SetWindowPosition(window,0,0);}
    Renderer renderer(window,width,height,assets);std::cout<<"GPU: "<<renderer.adapter<<", VRAM: "<<renderer.vram/(1024*1024)<<" MiB\n"<<std::flush;
    std::vector<Config>configs={{1000,10000,128,false,"1000_10000_tactical"},{5000,50000,256,false,"5000_50000_tactical"},{10000,100000,512,false,"10000_100000_tactical"},{1000,10000,128,true,"1000_10000_fps"},{5000,50000,256,true,"5000_50000_fps"},{10000,100000,512,true,"10000_100000_fps"},{5000,50000,256,true,"5000_50000_dense_fps",true}};
    if(smoke)configs={{5000,50000,256,false,"smoke_tactical"}};
    if(!only.empty()){configs.erase(std::remove_if(configs.begin(),configs.end(),[&](auto&c){return c.name!=only;}),configs.end());if(configs.empty())throw std::runtime_error("Unknown --only scenario");}
    if(!benchmark)configs={{5000,50000,256,false,"interactive"}};
    std::vector<Measurements>results;for(auto c:configs)results.emplace_back(c);
    SceneData scene;Camera camera;Vec3 player{0,5.7f,6};float yaw=0,pitch=-.04f,zoom=225;bool running=true,paused=false,vsync=!benchmark,fpsView=false;
    for(size_t scenario=0;scenario<configs.size()&&running;++scenario){
      Config cfg=configs[scenario];auto simulation=std::make_unique<Simulation>(cfg.enemies,cfg.projectiles,cfg.turrets);if(cfg.dense)simulation->denseCrowd();auto&m=results[scenario];fpsView=cfg.fps;
      double accumulator=0;auto scenarioStart=Clock::now(),last=scenarioStart,uiTime=last;float displayFps=0,displayCpu=0;bool measuring=false;uint64_t initialHits=0,initialKills=0,initialChecks=0;float initialSim=0;Clock::time_point measuredStart{};
      std::cout<<"Scenario "<<cfg.name<<" "<<width<<'x'<<height<<"\n"<<std::flush;
      while(running){
        auto frameStart=Clock::now();double elapsed=seconds(scenarioStart,frameStart),dt=seconds(last,frameStart);last=frameStart;
        if(benchmark&&!measuring&&elapsed>=warmup){measuring=true;measuredStart=frameStart;initialHits=simulation->hits;initialKills=simulation->kills;initialChecks=simulation->targetChecks;initialSim=simulation->time;}
        if(benchmark&&measuring&&seconds(measuredStart,frameStart)>=duration){
          m.wallSeconds=seconds(measuredStart,frameStart);m.simSeconds=simulation->time-initialSim;m.hits=simulation->hits-initialHits;m.kills=simulation->kills-initialKills;m.checks=simulation->targetChecks-initialChecks;
          PROCESS_MEMORY_COUNTERS memory{};memory.cb=sizeof(memory);GetProcessMemoryInfo(GetCurrentProcess(),&memory,sizeof(memory));m.memory=memory.WorkingSetSize;
          // Screenshot readback deliberately outside the measurement interval.
          renderer.begin(camera,simulation->time,-1);scene.render(renderer);renderer.text(24,18,"RDZEN / NATIVE STRESS TEST",26);renderer.text(24,60,cfg.name+" | "+fixed(1000/mean(m.frame),1)+" FPS | p99 "+fixed(percentile(m.frame,.99))+" ms",20);renderer.flushText();renderer.end();renderer.screenshot(report/(cfg.name+".png"));renderer.present(false);
          std::cout<<"Done: "<<fixed(1000/mean(m.frame),1)<<" FPS, p99 "<<fixed(percentile(m.frame,.99))<<" ms; sim "<<fixed(mean(m.sim))<<" ms/tick; hits "<<m.hits<<"\n"<<std::flush;
          break;
        }
        SDL_Event event;
        while(SDL_PollEvent(&event)){
          if(event.type==SDL_EVENT_QUIT)running=false;
          if(event.type==SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED){if(!benchmark)renderer.resize(event.window.data1,event.window.data2);}
          if(event.type==SDL_EVENT_KEY_DOWN&&!event.key.repeat){auto key=event.key.scancode;
            if(key==SDL_SCANCODE_ESCAPE){if(SDL_GetWindowRelativeMouseMode(window))SDL_SetWindowRelativeMouseMode(window,false);else running=false;}
            if(!benchmark){if(key==SDL_SCANCODE_TAB){fpsView=!fpsView;SDL_SetWindowRelativeMouseMode(window,fpsView);}if(key==SDL_SCANCODE_P)paused=!paused;if(key==SDL_SCANCODE_V)vsync=!vsync;
              if(key>=SDL_SCANCODE_1&&key<=SDL_SCANCODE_3){int k=key-SDL_SCANCODE_1;int n[]={1000,5000,10000},p[]={10000,50000,100000},t[]={128,256,512};simulation=std::make_unique<Simulation>(n[k],p[k],t[k]);accumulator=0;}
            }
          }
          if(!benchmark&&event.type==SDL_EVENT_MOUSE_BUTTON_DOWN&&fpsView)SDL_SetWindowRelativeMouseMode(window,true);
          if(!benchmark&&event.type==SDL_EVENT_MOUSE_MOTION&&fpsView&&SDL_GetWindowRelativeMouseMode(window)){yaw+=event.motion.xrel*.0025f;pitch=std::clamp(pitch-event.motion.yrel*.0025f,-1.3f,1.3f);}
          if(!benchmark&&event.type==SDL_EVENT_MOUSE_WHEEL&&!fpsView)zoom=std::clamp(zoom-event.wheel.y*12,45.f,300.f);
        }
        if(!running){m.incomplete=true;break;}
        auto simStart=Clock::now();int steps=0;
        if(!paused){accumulator+=std::min(dt,.25);while(accumulator>=1./60&&steps<8){auto a=Clock::now();simulation->step(1.f/60);auto b=Clock::now();if(measuring)m.sim.push_back(milliseconds(a,b));accumulator-=1./60;++steps;}}
        else accumulator=0;
        auto simEnd=Clock::now();
        if(fpsView){
          if(!benchmark&&!paused){const bool*keys=SDL_GetKeyboardState(nullptr);float forward=float(keys[SDL_SCANCODE_W])-float(keys[SDL_SCANCODE_S]),side=float(keys[SDL_SCANCODE_D])-float(keys[SDL_SCANCODE_A]);float norm=std::max(1.f,std::sqrt(forward*forward+side*side));float speed=keys[SDL_SCANCODE_LSHIFT]?6:3.5f;player.x+=(std::sin(yaw)*forward+std::cos(yaw)*side)*speed*float(dt)/norm;player.z+=(std::cos(yaw)*forward-std::sin(yaw)*side)*speed*float(dt)/norm;float radius=std::hypot(player.x,player.z);if(radius>7.3f){player.x*=7.3f/radius;player.z*=7.3f/radius;}if(radius<1.6f){player.x*=1.6f/std::max(radius,.001f);player.z*=1.6f/std::max(radius,.001f);}}
          camera.eye=player;camera.target=player+Vec3{std::sin(yaw)*std::cos(pitch),std::sin(pitch),std::cos(yaw)*std::cos(pitch)};
          if(benchmark){float a=simulation->time*.08f;camera.eye={std::sin(a)*6,5.7f,std::cos(a)*6};camera.target=camera.eye+Vec3{std::sin(a),-.08f,std::cos(a)};}
        }else{camera.eye={0,zoom,-zoom*.68f};camera.target={0,0,0};}
        auto prepStart=Clock::now();scene.prepare(*simulation);auto prepEnd=Clock::now();
        renderer.begin(camera,simulation->time,measuring?int(scenario):-1);scene.render(renderer);
        if(seconds(uiTime,frameStart)>.3){displayFps=float(1/std::max(dt,.00001));displayCpu=float(milliseconds(simStart,simEnd));uiTime=frameStart;}
        renderer.rect(16,16,550,168,.025f,.055f,.065f,.92f);renderer.text(30,25,"RDZEN / NATIVE PERFORMANCE LAB",23);
        renderer.text(30,65,"ENEMIES "+std::to_string(simulation->enemies.size())+"   PROJECTILES "+std::to_string(simulation->projectiles.size()),18);
        renderer.text(30,92,"TURRETS "+std::to_string(simulation->turrets.size())+"   FPS "+fixed(displayFps,0)+"   CPU SIM "+fixed(displayCpu)+" ms",17);
        renderer.text(30,119,"HITS "+std::to_string(simulation->hits)+"   KILLS "+std::to_string(simulation->kills)+"   "+(vsync?"VSYNC ON":"VSYNC OFF"),17);
        renderer.text(30,146,benchmark?(measuring?"MEASURING "+fixed(seconds(measuredStart,frameStart),1)+" / "+fixed(duration,0)+" s":"WARMUP / shaders, driver, clocks"):(paused?"PAUSED":"LIVE SIMULATION / fixed 60 Hz"),16);
        renderer.rect(16,float(renderer.height)-55,float(renderer.width)-32,39,.025f,.055f,.065f,.92f);
        renderer.text(30,float(renderer.height)-51,benchmark?"AUTOMATED BENCHMARK | "+cfg.name+" | ESC: stop":"1 / 2 / 3: 1K / 5K / 10K enemies | TAB: FPS / tactical | WASD: deck | wheel: zoom | P: pause | V: vsync | ESC: exit",16);
        renderer.flushText();renderer.end();auto submitEnd=Clock::now();renderer.present(vsync);auto frameEnd=Clock::now();
        if(measuring){m.frame.push_back(milliseconds(frameStart,frameEnd));m.prepare.push_back(milliseconds(prepStart,prepEnd));m.cpuSubmit.push_back(milliseconds(prepEnd,submitEnd));m.present.push_back(milliseconds(submitEnd,frameEnd));m.drawCalls=renderer.drawCalls;}
        for(auto sample:renderer.completed)if(sample.tag>=0&&sample.tag<int(results.size()))results[sample.tag].gpu.push_back(sample.ms);renderer.completed.clear();
      }
    }
    // Drain outstanding timestamp queries after all measured work (not timed).
    renderer.context->Flush();for(int i=0;i<25;++i){renderer.collectQueries();SDL_Delay(2);}for(auto sample:renderer.completed)if(sample.tag>=0&&sample.tag<int(results.size()))results[sample.tag].gpu.push_back(sample.ms);
    if(benchmark){writeReport(report,renderer,results,width,height);std::cout<<"Reports: "<<report.string()<<"\n";}
    SDL_SetWindowRelativeMouseMode(window,false);
    // COM renderer is destroyed on scope exit before SDL teardown by process exit.
    return running?0:2;
  }catch(const std::exception&e){std::cerr<<"ERROR: "<<e.what()<<'\n';SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,"RDZEN native test",e.what(),nullptr);return 1;}
}
