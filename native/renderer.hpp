#pragma once
#define NOMINMAX
#include <windows.h>
#include <d3d11.h>
#include <dxgi.h>
#include <d3dcompiler.h>
#include <wincodec.h>
#include <SDL3/SDL.h>
#include <array>
#include <filesystem>
#include <stdexcept>
#include <string>
#include <cstring>
#include "simulation.hpp"

namespace rdzen {
template <class T> struct Com {
  T* p = nullptr;
  Com() = default;
  ~Com() {
    if (p)
      p->Release();
  }
  Com(const Com&) = delete;
  Com& operator=(const Com&) = delete;
  T* operator->() const { return p; }
  T** put() {
    if (p) {
      p->Release();
      p = nullptr;
    }
    return &p;
  }
  operator T*() const { return p; }
};
inline void check(HRESULT hr, const char* message) {
  if (FAILED(hr))
    throw std::runtime_error(std::string(message) + " HRESULT=" + std::to_string(uint32_t(hr)));
}
struct Mat {
  float m[16]{};
};
inline Mat multiply(const Mat& a, const Mat& b) {
  Mat c;
  for (int r = 0; r < 4; ++r)
    for (int col = 0; col < 4; ++col)
      for (int k = 0; k < 4; ++k)
        c.m[r * 4 + col] += a.m[r * 4 + k] * b.m[k * 4 + col];
  return c;
}
struct Camera {
  Vec3 eye{0, 225, -175}, target{0, 0, 0}, right, up;
  float fov = 60, farClip = 1200, roll = 0;
  Mat matrix(float aspect) {
    Vec3 z = normalize(target - eye);
    right = normalize(cross({0, 1, 0}, z));
    up = cross(z, right);
    Vec3 baseRight = right;
    right = baseRight*std::cos(roll)+up*std::sin(roll);
    up = up*std::cos(roll)-baseRight*std::sin(roll);
    Mat v{{right.x, up.x, z.x, 0, right.y, up.y, z.y, 0, right.z, up.z, z.z, 0, -dot(right, eye),
           -dot(up, eye), -dot(z, eye), 1}};
    float ys = 1 / std::tan(fov * PI / 360), nearPlane = .1f, farPlane = farClip;
    Mat p{{ys / aspect, 0, 0, 0, 0, ys, 0, 0, 0, 0, farPlane / (farPlane - nearPlane), 1, 0, 0,
           -nearPlane * farPlane / (farPlane - nearPlane), 0}};
    return multiply(v, p);
  }
};
struct Vertex {
  float x, y, z, nx, ny, nz, u, v;
};
struct Mesh {
  Com<ID3D11Buffer> vertices;
  UINT count = 0;
};
struct Constants {
  Mat vp;
  float right[4], up[4], eye[4], params[4];
};
struct GpuSample {
  double ms;
  int tag;
};
struct QuerySet {
  Com<ID3D11Query> disjoint, start, end;
  bool pending = false;
  int tag = -1;
};

inline const char* shaderSource = R"HLSL(
cbuffer Scene : register(b0) {
  row_major float4x4 vp; float4 camRight; float4 camUp; float4 eye; float4 params;
};
Texture2D tex : register(t0); SamplerState samp : register(s0);
SamplerState groundSamp : register(s1);
struct VSIn {float3 p:POSITION;float3 n:NORMAL;float2 uv:TEXCOORD0;float3 origin:INSTANCE0;float3 scale:INSTANCE1;float yaw:INSTANCE2;float4 color:INSTANCE3;float extra:INSTANCE4;};
struct VSOut {float4 p:SV_POSITION;float2 uv:TEXCOORD0;float3 normal:TEXCOORD1;float3 world:TEXCOORD2;float4 color:COLOR0;float extra:TEXCOORD3;};
float3 safeDirection(float3 v,float3 fallback){float sq=dot(v,v);return sq>1e-12?v*rsqrt(max(sq,1e-12)):fallback;}
float3 beamSide(float3 dir,float3 toEye){
 dir=safeDirection(dir,float3(0,0,1));
 float3 side=cross(dir,safeDirection(toEye,-dir));
 if(dot(side,side)<1e-6)side=camRight.xyz-dir*dot(camRight.xyz,dir);
 return safeDirection(side,float3(1,0,0));
}
VSOut vsMain(VSIn i){VSOut o;float mode=params.x;float3 p=i.p*i.scale;
 if(mode==1||mode==8||mode==9||mode==17){p=i.origin+camRight.xyz*p.x+camUp.xyz*p.y;o.normal=normalize(eye.xyz-i.origin);}
 else if(mode==2){float3 dir=safeDirection(float3(i.yaw,0,i.extra),float3(0,0,1));float3 side=beamSide(dir,eye.xyz-i.origin);p=i.origin+side*p.x+dir*p.y;o.normal=float3(0,1,0);}
 else if(mode==7){float3 dir=float3(i.scale.y,i.scale.z,i.yaw);float3 side=beamSide(dir,eye.xyz-i.origin);p=i.origin+dir*i.p.y+side*i.p.x*i.scale.x;o.normal=float3(0,1,0);}
 else if(mode==10||mode==12){p=i.origin+float3(i.p.x*i.scale.x,0,i.p.y*i.scale.y);o.normal=float3(0,1,0);}
 else if(mode==23||mode==24){float x=i.p.x*i.scale.x,z=i.p.y*i.scale.y;float s=sin(i.yaw),c=cos(i.yaw);p=i.origin+float3(x*c+z*s,0,-x*s+z*c);o.normal=float3(0,1,0);}
 else if(mode==14){p=float3(i.origin.x+i.p.x*i.scale.x,.12,i.origin.z+i.p.y*i.scale.y);o.normal=float3(sin(i.yaw)*cos(i.extra),sin(i.extra),cos(i.yaw)*cos(i.extra));}
 else {float pitch=mode==18?i.color.a:((mode==0||mode==6||mode==13)?i.extra:0);float sp=sin(pitch),cp=cos(pitch);p=float3(p.x,p.y*cp-p.z*sp,p.y*sp+p.z*cp);float s=sin(i.yaw),c=cos(i.yaw);p=i.origin+float3(p.x*c+p.z*s,p.y,-p.x*s+p.z*c);float3 n=i.n/max(i.scale,.0001);n=float3(n.x,n.y*cp-n.z*sp,n.y*sp+n.z*cp);o.normal=normalize(float3(n.x*c+n.z*s,n.y,-n.x*s+n.z*c));}
 o.world=p;o.uv=i.uv;o.color=i.color;o.extra=i.extra;
 if(mode==15||mode==18){if(mode==18)o.color.a=1;float ch=i.extra-32;o.uv=(float2(fmod(ch,16),floor(ch/16))+i.uv)/float2(16,8);}
 if(mode==14){o.world=float3(i.p.x*i.scale.x,1.8-i.origin.y,i.p.y*i.scale.y);o.uv=float2(i.scale.x*.5,i.scale.z);}
 if(mode==11){o.p=float4(i.p.xy*2,1,1);o.world=normalize(cross(camRight.xyz,camUp.xyz)+camRight.xyz*i.p.x*2*params.z/params.w*eye.w+camUp.xyz*i.p.y*2*eye.w);}
 else if(mode==4||mode==5||mode==16||mode==19||mode==20||mode==21){o.p=float4(p.x/params.z*2-1,1-p.y/params.w*2,0,1);if(mode==16||mode==19||mode==20||mode==21)o.uv=float2(i.uv.x,1-i.uv.y);if(mode==4){float ch=i.extra-32;o.uv=(float2(fmod(ch,16),floor(ch/16))+float2(i.uv.x,1-i.uv.y))/float2(16,8);}}
 else o.p=mul(float4(p,1),vp);
 return o;
}
float4 psMain(VSOut i):SV_TARGET{
 float mode=params.x;float4 c=i.color;
 if(mode==22){float4 t=tex.Sample(samp,i.uv);clip(t.a-.2);return float4(pow(max(t.rgb*c.rgb,0),1/2.2),1);}
 if(mode==19||mode==20){
  float radius=c.r*3*params.w/900;
  float2 stepUV=mode==19?float2(radius/params.z,0):float2(0,radius/params.w);
  float3 blurred=tex.Sample(samp,i.uv).rgb*.227027;
  const float weights[4]={.1945946,.1216216,.054054,.016216};
  [unroll] for(int n=1;n<=4;++n)blurred+=(tex.Sample(samp,i.uv+stepUV*n).rgb+tex.Sample(samp,i.uv-stepUV*n).rgb)*weights[n-1];
  if(mode==20){float2 p=i.uv*2-1;blurred*=1-c.r*.10*dot(p,p);}
  return float4(blurred,1);
 }
 if(mode==21){float2 p=i.uv*2-1;float opening=(1-c.a)*1.2-.04;float edge=abs(p.y)+.18*p.x*p.x*(1-c.a)-opening;return float4(.002,.003,.005,smoothstep(-.025,.025,edge));}
 if(mode==17){float border=(i.uv.x<.008||i.uv.x>.992||i.uv.y<.12||i.uv.y>.88)?1:0;float3 fill=i.uv.x<=c.a?c.rgb:float3(.035,.045,.055);return float4(lerp(fill,float3(.65,.7,.72),border),1);}
 if(mode==16)return c*tex.Sample(samp,i.uv);
 if(mode==4){c.a*=tex.Sample(samp,i.uv).a;return c;}
 if(mode==15||mode==18){c.a*=tex.Sample(samp,i.uv).a;return c;}
 if(mode==5)return c;
 if(mode==13){
  float tick=floor(params.y*20);float2 p=i.uv*2-1;
  float2 uv=.5+.5*p*(1+.035*dot(p,p));
  float glitch=step(.985,frac(sin(tick*12.9898)*43758.5453));
  uv.x+=glitch*.0018*sin(floor(uv.y*80)*9+tick);
  float3 image=tex.Sample(samp,saturate(uv)).rgb;
  float2 q=abs(uv-.5);float sight=((q.x<.002&&q.y<.065)||(q.y<.002&&q.x<.04))?1:0;
  image=lerp(image,float3(.25,1,.8),sight);
  float noise=frac(sin(dot(floor(i.uv*float2(512,320)),float2(12.9898,78.233))+tick*37.719)*43758.5453)-.5;
  float scan=.965+.035*sin(i.uv.y*320*3.14159265);
  float roll=1-.018*exp(-pow((frac(i.uv.y-params.y*.09)-.5)*28,2));
  float vignette=1-.12*dot(p,p);
  float mask=1-smoothstep(.075,.11,length(max(abs(p)-float2(.90,.84),0)));
  image=image*float3(.96,1,.98)*scan*roll*vignette+noise*.025;
  return float4(max(image,0)*mask,1);
 }
 if(mode==14){float d=length(i.world.xz),cone=dot(normalize(i.world),normalize(i.normal));clip(i.uv.x-d);clip(cone-i.uv.y);float rim=1-smoothstep(0,1,i.uv.x-d);float edge=1-smoothstep(0,min(.012,(1-i.uv.y)*.3),cone-i.uv.y);float band=frac(d/21)*21;float ticks=1-smoothstep(.12,.4,min(band,21-band));return float4(c.rgb,c.a*(.12+.42*max(rim,edge)+.16*ticks));}
 if(mode==11){float3 ray=normalize(i.world);float h=saturate(ray.y);float3 sky=lerp(float3(.19,.28,.31),float3(.008,.02,.04),pow(h,.45));float moon=dot(ray,normalize(float3(-.55,.34,.8)));float disc=smoothstep(.9966,.9968,moon);sky=lerp(sky,float3(.25,.42,.45)*( .7+.25*sin(ray.y*650)),disc);float2 stars=floor(ray.xz/max(ray.y,.05)*380);float star=step(.9991,frac(sin(dot(stars,float2(12.9898,78.233)))*43758.5453))*smoothstep(.1,.6,ray.y);sky+=star*.25;return float4(sky,1);}
 if(mode==6)return float4(pow(max(c.rgb,0),1/2.2),c.a);
 if(mode==7){float edge=saturate(1-abs(i.uv.x*2-1));return float4(c.rgb*(.8+edge),c.a*edge);}
 if(mode==8||mode==9){float d=length(i.uv*2-1);clip(1-d);float a=pow(saturate(1-d),mode==8?1.5:1.1);return float4(c.rgb,c.a*a);}
 if(mode==10){float d=length(i.uv*2-1);float rim=1-smoothstep(.006,.018,abs(d-.97));clip(rim-.01);return float4(c.rgb,c.a*rim);}
 if(mode==12){float d=length(i.uv*2-1);clip(1-d);return float4(c.rgb,c.a*pow(saturate(1-d),1.6));}
 if(mode==23||mode==24){
  float2 p=(i.uv*2-1)*float2(1,i.extra);
  float d=length(float2(p.x,max(abs(p.y)-(i.extra-1),0)));
  float aa=max(fwidth(d),.03);float mask=1-smoothstep(.65-aa,1+aa,d);
  float fog=exp(-length(i.world-eye.xyz)*camUp.w);clip(mask-.002);
  float join=i.extra>1.01?.5+.5*saturate(i.extra-1-abs(p.y)):1;
  float opacity=mode==23?1-pow(max(1-c.a*mask,0),join):c.a*mask*join;
  return float4(c.rgb,opacity*fog);
 }
 if(mode==1){float4 t=tex.Sample(samp,i.uv);clip(t.a-.32);c*=t;c.a=1;}
 else if(mode==2){float edge=1-abs(i.uv.x*2-1);c.rgb*=pow(saturate(edge),1.2)*2;c.a*=edge;return c;}
 else if(mode==3){float d=length(i.world.xz);clip(camRight.w-d);float3 detail=sqrt(max(tex.Sample(groundSamp,i.world.xz/10).rgb,0));float3 broad=sqrt(max(tex.Sample(groundSamp,i.world.xz/47).rgb,0));c.rgb*=.65+1.3*(detail*.8+broad*.2);}
 else c*=tex.Sample(samp,i.uv);
 if(mode!=1){float light=.48+max(dot(normalize(i.normal),normalize(float3(-.5,.9,-.35))),0)*.85;c.rgb*=light;}
 float fog=1-exp(-length(i.world-eye.xyz)*camUp.w);c.rgb=lerp(c.rgb,float3(.026,.061,.077),fog);
 return float4(pow(max(c.rgb,0),1/2.2),c.a);
}
)HLSL";

class Renderer {
public:
  Com<ID3D11Device> device;
  Com<ID3D11DeviceContext> context;
  Com<IDXGISwapChain> swap;
  Com<ID3D11RenderTargetView> target;
  Com<ID3D11DepthStencilView> depth;
  Com<ID3D11VertexShader> vs;
  Com<ID3D11PixelShader> ps;
  Com<ID3D11InputLayout> layout;
  Com<ID3D11Buffer> constants, instances;
  Com<ID3D11SamplerState> sampler, groundSampler;
  Com<ID3D11RasterizerState> raster;
  Com<ID3D11DepthStencilState> depthWrite, depthRead, depthOff;
  Com<ID3D11BlendState> opaque, additive, alpha;
  Com<ID3D11ShaderResourceView> beast, metal, white, font, terrain;
  Com<IWICImagingFactory> wic;
  Mesh quad, cube, cylinder, tube, rock, ground;
  std::array<QuerySet, 12> queries;
  int queryIndex = 0, activeQuery = -1;
  std::vector<GpuSample> completed;
  std::vector<Instance> ui;
  std::string adapter;
  size_t vram = 0;
  Constants data{};
  int width, height, drawCalls = 0;
  UINT maxInstances = 250000, instanceOffset = 0;
  uint64_t submitted = 0;
  float groundRadius = 160, fogDensity = .0018f;

  Renderer(SDL_Window* window, int w, int h, const std::filesystem::path& assets)
      : width(w), height(h) {
    HWND hwnd = (HWND)SDL_GetPointerProperty(SDL_GetWindowProperties(window),
                                             SDL_PROP_WINDOW_WIN32_HWND_POINTER, nullptr);
    DXGI_SWAP_CHAIN_DESC desc{};
    desc.BufferDesc.Width = w;
    desc.BufferDesc.Height = h;
    desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    desc.BufferCount = 2;
    desc.OutputWindow = hwnd;
    desc.Windowed = TRUE;
    desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    D3D_FEATURE_LEVEL level;
    D3D_FEATURE_LEVEL levels[] = {D3D_FEATURE_LEVEL_11_0};
    check(D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0, levels, 1,
                                        D3D11_SDK_VERSION, &desc, swap.put(), device.put(), &level,
                                        context.put()),
          "Create hardware Direct3D 11 device");
    Com<IDXGIDevice> dxgi;
    check(device->QueryInterface(__uuidof(IDXGIDevice), (void**)dxgi.put()), "DXGI device");
    Com<IDXGIAdapter> ad;
    check(dxgi->GetAdapter(ad.put()), "Get adapter");
    DXGI_ADAPTER_DESC info{};
    ad->GetDesc(&info);
    char name[256];
    WideCharToMultiByte(CP_UTF8, 0, info.Description, -1, name, 256, nullptr, nullptr);
    adapter = name;
    vram = info.DedicatedVideoMemory;
    Com<IDXGIFactory> factory;
    check(ad->GetParent(__uuidof(IDXGIFactory), (void**)factory.put()), "Get factory");
    factory->MakeWindowAssociation(hwnd, DXGI_MWA_NO_ALT_ENTER);
    Com<IDXGIDevice1> dxgi1;
    if (SUCCEEDED(device->QueryInterface(__uuidof(IDXGIDevice1), (void**)dxgi1.put())))
      dxgi1->SetMaximumFrameLatency(1);
    resize(w, h);
    Com<ID3DBlob> vb, pb, error;
    HRESULT hr =
        D3DCompile(shaderSource, std::strlen(shaderSource), "rdzen", nullptr, nullptr, "vsMain",
                   "vs_5_0", D3DCOMPILE_OPTIMIZATION_LEVEL3, 0, vb.put(), error.put());
    if (FAILED(hr))
      throw std::runtime_error(error.p ? (char*)error->GetBufferPointer() : "Vertex shader failed");
    hr = D3DCompile(shaderSource, std::strlen(shaderSource), "rdzen", nullptr, nullptr, "psMain",
                    "ps_5_0", D3DCOMPILE_OPTIMIZATION_LEVEL3, 0, pb.put(), error.put());
    if (FAILED(hr))
      throw std::runtime_error(error.p ? (char*)error->GetBufferPointer() : "Pixel shader failed");
    check(
        device->CreateVertexShader(vb->GetBufferPointer(), vb->GetBufferSize(), nullptr, vs.put()),
        "Vertex shader");
    check(device->CreatePixelShader(pb->GetBufferPointer(), pb->GetBufferSize(), nullptr, ps.put()),
          "Pixel shader");
    D3D11_INPUT_ELEMENT_DESC elems[] = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"INSTANCE", 0, DXGI_FORMAT_R32G32B32_FLOAT, 1, 0, D3D11_INPUT_PER_INSTANCE_DATA, 1},
        {"INSTANCE", 1, DXGI_FORMAT_R32G32B32_FLOAT, 1, 12, D3D11_INPUT_PER_INSTANCE_DATA, 1},
        {"INSTANCE", 2, DXGI_FORMAT_R32_FLOAT, 1, 24, D3D11_INPUT_PER_INSTANCE_DATA, 1},
        {"INSTANCE", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 28, D3D11_INPUT_PER_INSTANCE_DATA, 1},
        {"INSTANCE", 4, DXGI_FORMAT_R32_FLOAT, 1, 44, D3D11_INPUT_PER_INSTANCE_DATA, 1}};
    check(device->CreateInputLayout(elems, 8, vb->GetBufferPointer(), vb->GetBufferSize(),
                                    layout.put()),
          "Input layout");
    D3D11_BUFFER_DESC bd{};
    bd.ByteWidth = sizeof(Constants);
    bd.Usage = D3D11_USAGE_DYNAMIC;
    bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    check(device->CreateBuffer(&bd, nullptr, constants.put()), "Constants");
    bd.ByteWidth = maxInstances * sizeof(Instance);
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    check(device->CreateBuffer(&bd, nullptr, instances.put()), "Instance buffer");
    D3D11_SAMPLER_DESC sd{};
    sd.Filter = D3D11_FILTER_ANISOTROPIC;
    sd.MaxAnisotropy = 16;
    sd.AddressU = sd.AddressV = sd.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    sd.MaxLOD = D3D11_FLOAT32_MAX;
    check(device->CreateSamplerState(&sd, sampler.put()), "Sampler");
    sd.AddressU = sd.AddressV = sd.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    check(device->CreateSamplerState(&sd, groundSampler.put()), "Ground sampler");
    D3D11_RASTERIZER_DESC rd{};
    rd.FillMode = D3D11_FILL_SOLID;
    rd.CullMode = D3D11_CULL_NONE;
    rd.DepthClipEnable = TRUE;
    check(device->CreateRasterizerState(&rd, raster.put()), "Raster state");
    D3D11_DEPTH_STENCIL_DESC dd{};
    dd.DepthEnable = TRUE;
    dd.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    dd.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
    check(device->CreateDepthStencilState(&dd, depthWrite.put()), "Depth state");
    dd.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
    check(device->CreateDepthStencilState(&dd, depthRead.put()), "Depth read");
    dd.DepthEnable = FALSE;
    check(device->CreateDepthStencilState(&dd, depthOff.put()), "Depth off");
    D3D11_BLEND_DESC blend{};
    blend.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    check(device->CreateBlendState(&blend, opaque.put()), "Opaque blend");
    auto& bt = blend.RenderTarget[0];
    bt.BlendEnable = TRUE;
    bt.SrcBlend = D3D11_BLEND_SRC_ALPHA;
    bt.DestBlend = D3D11_BLEND_ONE;
    bt.BlendOp = D3D11_BLEND_OP_ADD;
    bt.SrcBlendAlpha = D3D11_BLEND_ONE;
    bt.DestBlendAlpha = D3D11_BLEND_ZERO;
    bt.BlendOpAlpha = D3D11_BLEND_OP_ADD;
    check(device->CreateBlendState(&blend, additive.put()), "Additive blend");
    bt.DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    check(device->CreateBlendState(&blend, alpha.put()), "Alpha blend");
    check(CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER,
                           __uuidof(IWICImagingFactory), (void**)wic.put()),
          "WIC factory");
    loadTexture(assets / L"alien-beast.png", beast);
    loadTexture(assets / L"turret-metal.png", metal);
    loadTexture(assets / L"ground-basalt.png", terrain);
    uint32_t pixel = 0xffffffff;
    texture(1, 1, &pixel, white, false);
    makeFont();
    makeMeshes();
    for (auto& q : queries) {
      D3D11_QUERY_DESC qd{D3D11_QUERY_TIMESTAMP_DISJOINT, 0};
      check(device->CreateQuery(&qd, q.disjoint.put()), "GPU disjoint query");
      qd.Query = D3D11_QUERY_TIMESTAMP;
      check(device->CreateQuery(&qd, q.start.put()), "GPU start query");
      check(device->CreateQuery(&qd, q.end.put()), "GPU end query");
    }
    completed.reserve(10000);
    ui.reserve(4096);
  }
  void resize(int w, int h) {
    if (w <= 0 || h <= 0)
      return;
    // SDL can repeat size notifications during focus/fullscreen changes. Recreating an
    // unchanged swap chain needlessly interrupts presentation and can expose a blank frame.
    if (target.p && w == width && h == height)
      return;
    context->OMSetRenderTargets(0, nullptr, nullptr);
    target.put();
    depth.put();
    width = w;
    height = h;
    check(swap->ResizeBuffers(0, w, h, DXGI_FORMAT_UNKNOWN, 0), "Resize swap chain");
    Com<ID3D11Texture2D> bb;
    check(swap->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)bb.put()), "Backbuffer");
    check(device->CreateRenderTargetView(bb, nullptr, target.put()), "Render target");
    D3D11_TEXTURE2D_DESC td{};
    td.Width = w;
    td.Height = h;
    td.MipLevels = td.ArraySize = 1;
    td.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    td.SampleDesc.Count = 1;
    td.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    Com<ID3D11Texture2D> dt;
    check(device->CreateTexture2D(&td, nullptr, dt.put()), "Depth texture");
    check(device->CreateDepthStencilView(dt, nullptr, depth.put()), "Depth view");
  }
  void texture(UINT w, UINT h, const void* pixels, Com<ID3D11ShaderResourceView>& out,
               bool srgb = true) {
    D3D11_TEXTURE2D_DESC td{};
    td.Width = w;
    td.Height = h;
    td.MipLevels = 0;
    td.ArraySize = 1;
    td.Format = srgb ? DXGI_FORMAT_R8G8B8A8_UNORM_SRGB : DXGI_FORMAT_R8G8B8A8_UNORM;
    td.SampleDesc.Count = 1;
    td.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
    td.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;
    Com<ID3D11Texture2D> t;
    check(device->CreateTexture2D(&td, nullptr, t.put()), "Texture");
    context->UpdateSubresource(t, 0, nullptr, pixels, w * 4, 0);
    check(device->CreateShaderResourceView(t, nullptr, out.put()), "Texture view");
    context->GenerateMips(out);
  }
  void loadTexture(const std::filesystem::path& path, Com<ID3D11ShaderResourceView>& out) {
    Com<IWICBitmapDecoder> decoder;
    check(wic->CreateDecoderFromFilename(path.c_str(), nullptr, GENERIC_READ,
                                         WICDecodeMetadataCacheOnLoad, decoder.put()),
          "Load PNG");
    Com<IWICBitmapFrameDecode> frame;
    check(decoder->GetFrame(0, frame.put()), "PNG frame");
    Com<IWICFormatConverter> convert;
    check(wic->CreateFormatConverter(convert.put()), "Converter");
    check(convert->Initialize(frame, GUID_WICPixelFormat32bppRGBA, WICBitmapDitherTypeNone, nullptr,
                              0, WICBitmapPaletteTypeCustom),
          "Convert PNG");
    UINT w, h;
    convert->GetSize(&w, &h);
    std::vector<uint8_t> pixels(size_t(w) * h * 4);
    check(convert->CopyPixels(nullptr, w * 4, UINT(pixels.size()), pixels.data()), "PNG pixels");
    texture(w, h, pixels.data(), out);
  }
  void makeFont() {
    constexpr int w = 512, h = 384;
    HDC dc = CreateCompatibleDC(nullptr);
    BITMAPINFO bi{};
    bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bi.bmiHeader.biWidth = w;
    bi.bmiHeader.biHeight = -h;
    bi.bmiHeader.biPlanes = 1;
    bi.bmiHeader.biBitCount = 32;
    bi.bmiHeader.biCompression = BI_RGB;
    void* pixels;
    HBITMAP bitmap = CreateDIBSection(dc, &bi, DIB_RGB_COLORS, &pixels, nullptr, 0);
    auto old = SelectObject(dc, bitmap);
    std::memset(pixels, 0, w * h * 4);
    HFONT fontHandle =
        CreateFontW(-28, 0, 0, 0, FW_MEDIUM, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS,
                    CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY, FIXED_PITCH, L"Consolas");
    auto oldFont = SelectObject(dc, fontHandle);
    SetTextColor(dc, RGB(255, 255, 255));
    SetBkMode(dc, TRANSPARENT);
    const std::wstring polish = L"ąćęłńóśźżĄĆĘŁŃÓŚŹŻ";
    for (int c = 32; c < 146; ++c) {
      wchar_t letter = c < 128 ? wchar_t(c) : polish[c - 128];
      TextOutW(dc, ((c - 32) % 16) * 32, ((c - 32) / 16) * 48 + 5, &letter, 1);
    }
    GdiFlush();
    auto* p = (uint8_t*)pixels;
    for (int i = 0; i < w * h; ++i) {
      p[i * 4 + 3] = p[i * 4];
      p[i * 4] = p[i * 4 + 1] = p[i * 4 + 2] = 255;
    }
    texture(w, h, pixels, font, false);
    SelectObject(dc, oldFont);
    SelectObject(dc, old);
    DeleteObject(fontHandle);
    DeleteObject(bitmap);
    DeleteDC(dc);
  }
  void uploadMesh(Mesh& m, const std::vector<Vertex>& v) {
    D3D11_BUFFER_DESC d{};
    d.ByteWidth = UINT(v.size() * sizeof(Vertex));
    d.Usage = D3D11_USAGE_IMMUTABLE;
    d.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    D3D11_SUBRESOURCE_DATA init{v.data(), 0, 0};
    check(device->CreateBuffer(&d, &init, m.vertices.put()), "Mesh buffer");
    m.count = UINT(v.size());
  }
  void makeMeshes() {
    uploadMesh(quad, {{-.5f, -.5f, 0, 0, 0, -1, 0, 1},
                      {-.5f, .5f, 0, 0, 0, -1, 0, 0},
                      {.5f, .5f, 0, 0, 0, -1, 1, 0},
                      {-.5f, -.5f, 0, 0, 0, -1, 0, 1},
                      {.5f, .5f, 0, 0, 0, -1, 1, 0},
                      {.5f, -.5f, 0, 0, 0, -1, 1, 1}});
    std::vector<Vertex> v;
    auto face = [&](Vec3 n, Vec3 u, Vec3 y) {
      Vec3 c = n * .5f;
      Vec3 p[] = {c - u * .5f - y * .5f, c - u * .5f + y * .5f, c + u * .5f + y * .5f,
                  c + u * .5f - y * .5f};
      int ix[] = {0, 1, 2, 0, 2, 3};
      float uv[][2] = {{0, 1}, {0, 0}, {1, 0}, {1, 1}};
      for (int k : ix)
        v.push_back({p[k].x, p[k].y, p[k].z, n.x, n.y, n.z, uv[k][0], uv[k][1]});
    };
    face({0, 0, 1}, {1, 0, 0}, {0, 1, 0});
    face({0, 0, -1}, {-1, 0, 0}, {0, 1, 0});
    face({1, 0, 0}, {0, 0, -1}, {0, 1, 0});
    face({-1, 0, 0}, {0, 0, 1}, {0, 1, 0});
    face({0, 1, 0}, {1, 0, 0}, {0, 0, -1});
    face({0, -1, 0}, {1, 0, 0}, {0, 0, 1});
    uploadMesh(cube, v);
    v.clear();
    for (int i = 0; i < 64; ++i) {
      float a = i * 2 * PI / 64, b = (i + 1) * 2 * PI / 64;
      float x = std::sin(a), z = std::cos(a), xx = std::sin(b), zz = std::cos(b);
      Vertex p[] = {{x, -.5f, z, x, 0, z, float(i) / 64, 1},
                    {x, .5f, z, x, 0, z, float(i) / 64, 0},
                    {xx, .5f, zz, xx, 0, zz, float(i + 1) / 64, 0},
                    {xx, -.5f, zz, xx, 0, zz, float(i + 1) / 64, 1}};
      for (int k : {0, 1, 2, 0, 2, 3})
        v.push_back(p[k]);
      v.push_back({0, .5f, 0, 0, 1, 0, .5f, .5f});
      v.push_back({x, .5f, z, 0, 1, 0, x * .5f + .5f, z * .5f + .5f});
      v.push_back({xx, .5f, zz, 0, 1, 0, xx * .5f + .5f, zz * .5f + .5f});
    }
    uploadMesh(cylinder, v);
  }
  void makeArsenalMeshes() {
    std::vector<Vertex> v;
    // A subdivided plane avoids precision cracks from stretching the two top triangles
    // of a single cube across the entire 6.4 km world.
    constexpr int groundSide = 128;
    v.reserve(size_t(groundSide) * groundSide * 6);
    for (int z = 0; z < groundSide; ++z) {
      for (int x = 0; x < groundSide; ++x) {
        float x0 = float(x) / groundSide - .5f, x1 = float(x + 1) / groundSide - .5f;
        float z0 = float(z) / groundSide - .5f, z1 = float(z + 1) / groundSide - .5f;
        Vertex p[] = {{x0, 0, z0, 0, 1, 0, 0, 0},
                      {x0, 0, z1, 0, 1, 0, 0, 1},
                      {x1, 0, z1, 0, 1, 0, 1, 1},
                      {x1, 0, z0, 0, 1, 0, 1, 0}};
        for (int k : {0, 1, 2, 0, 2, 3})
          v.push_back(p[k]);
      }
    }
    uploadMesh(ground, v);
    v.clear();
    for (int i = 0; i < 12; ++i) {
      float a = i * 2 * PI / 12, b = (i + 1) * 2 * PI / 12;
      float x = std::sin(a), z = std::cos(a), xx = std::sin(b), zz = std::cos(b);
      Vertex p[] = {{x, -.5f, z, x, 0, z, float(i) / 12, 1},
                    {x, .5f, z, x, 0, z, float(i) / 12, 0},
                    {xx, .5f, zz, xx, 0, zz, float(i + 1) / 12, 0},
                    {xx, -.5f, zz, xx, 0, zz, float(i + 1) / 12, 1}};
      for (int k : {0, 1, 2, 0, 2, 3})
        v.push_back(p[k]);
      for (float sign : {-1.f, 1.f}) {
        v.push_back({0, sign * .5f, 0, 0, sign, 0, .5f, .5f});
        v.push_back({x, sign * .5f, z, 0, sign, 0, x * .5f + .5f, z * .5f + .5f});
        v.push_back({xx, sign * .5f, zz, 0, sign, 0, xx * .5f + .5f, zz * .5f + .5f});
      }
    }
    uploadMesh(tube, v);
    v.clear();
    Vec3 points[] = {{1, 0, 0}, {-1, 0, 0}, {0, 1.3f, 0}, {0, -.65f, 0}, {0, 0, 1}, {0, 0, -1}};
    int triangles[][3] = {{2, 0, 4}, {2, 4, 1}, {2, 1, 5}, {2, 5, 0},
                          {3, 4, 0}, {3, 1, 4}, {3, 5, 1}, {3, 0, 5}};
    for (auto& tri : triangles) {
      Vec3 normal =
          normalize(cross(points[tri[1]] - points[tri[0]], points[tri[2]] - points[tri[0]]));
      for (int k : tri) {
        auto p = points[k];
        v.push_back(
            {p.x, p.y, p.z, normal.x, normal.y, normal.z, p.x * .5f + .5f, p.z * .5f + .5f});
      }
    }
    uploadMesh(rock, v);
  }
  void collectQueries() {
    for (auto& q : queries)
      if (q.pending) {
        D3D11_QUERY_DATA_TIMESTAMP_DISJOINT d{};
        UINT64 a, b;
        if (context->GetData(q.disjoint, &d, sizeof(d), D3D11_ASYNC_GETDATA_DONOTFLUSH) == S_OK &&
            context->GetData(q.end, &b, sizeof(b), D3D11_ASYNC_GETDATA_DONOTFLUSH) == S_OK &&
            context->GetData(q.start, &a, sizeof(a), D3D11_ASYNC_GETDATA_DONOTFLUSH) == S_OK) {
          if (!d.Disjoint && q.tag >= 0)
            completed.push_back({double(b - a) * 1000 / d.Frequency, q.tag});
          q.pending = false;
        }
      }
  }
  void begin(Camera& camera, float time, int tag) {
    collectQueries();
    activeQuery = -1;
    auto& q = queries[queryIndex];
    if (!q.pending) {
      q.tag = tag;
      context->Begin(q.disjoint);
      context->End(q.start);
      activeQuery = queryIndex;
      queryIndex = (queryIndex + 1) % int(queries.size());
    }
    drawCalls = 0;
    submitted = 0;
    instanceOffset = 0;
    float clear[] = {.035f, .075f, .095f, 1};
    context->ClearRenderTargetView(target, clear);
    context->ClearDepthStencilView(depth, D3D11_CLEAR_DEPTH, 1, 0);
    ID3D11RenderTargetView* rt = target;
    context->OMSetRenderTargets(1, &rt, depth);
    D3D11_VIEWPORT viewport{0, 0, float(width), float(height), 0, 1};
    context->RSSetViewports(1, &viewport);
    context->RSSetState(raster);
    context->IASetInputLayout(layout);
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    context->VSSetShader(vs, nullptr, 0);
    context->PSSetShader(ps, nullptr, 0);
    ID3D11Buffer* cb = constants;
    context->VSSetConstantBuffers(0, 1, &cb);
    context->PSSetConstantBuffers(0, 1, &cb);
    ID3D11SamplerState* ss = sampler;
    context->PSSetSamplers(0, 1, &ss);
    ID3D11SamplerState* gs = groundSampler;
    context->PSSetSamplers(1, 1, &gs);
    data.vp = camera.matrix(float(width) / height);
    std::memcpy(data.right, &camera.right, 12);
    std::memcpy(data.up, &camera.up, 12);
    std::memcpy(data.eye, &camera.eye, 12);
    data.right[3] = groundRadius;
    data.up[3] = fogDensity;
    data.eye[3] = std::tan(camera.fov * PI / 360);
    data.params[1] = time;
    data.params[2] = float(width);
    data.params[3] = float(height);
  }
  void draw(Mesh& mesh, const Instance* list, size_t count, ID3D11ShaderResourceView* textureView,
            int mode) {
    if (count == 0)
      return;
    if (count > maxInstances)
      throw std::runtime_error("Instance pool exceeded");
    data.params[0] = float(mode);
    D3D11_MAPPED_SUBRESOURCE mapped;
    check(context->Map(constants, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped), "Map constants");
    std::memcpy(mapped.pData, &data, sizeof(data));
    context->Unmap(constants, 0);
    if (instanceOffset + count > maxInstances)
      instanceOffset = 0;
    check(context->Map(instances, 0,
                       instanceOffset == 0 ? D3D11_MAP_WRITE_DISCARD : D3D11_MAP_WRITE_NO_OVERWRITE,
                       0, &mapped),
          "Map instances");
    std::memcpy((char*)mapped.pData + instanceOffset * sizeof(Instance), list,
                count * sizeof(Instance));
    context->Unmap(instances, 0);
    ID3D11Buffer* bufs[] = {mesh.vertices, instances};
    UINT strides[] = {sizeof(Vertex), sizeof(Instance)},
         offsets[] = {0, instanceOffset * UINT(sizeof(Instance))};
    context->IASetVertexBuffers(0, 2, bufs, strides, offsets);
    if (mode == 3) textureView = terrain;
    context->PSSetShaderResources(0, 1, &textureView);
    instanceOffset += UINT(count);
    bool overlay = mode == 4 || mode == 5 || mode == 11 || mode == 16 || mode == 19 || mode == 20 || mode == 21,
         soft = mode == 2 || mode == 7 || mode == 8 || mode == 9 || mode == 10 || mode == 12 ||
                mode == 14 || mode == 15 || mode == 18 || mode == 23 || mode == 24;
    context->OMSetDepthStencilState(overlay ? depthOff : soft ? depthRead : depthWrite, 0);
    bool add = mode == 2 || mode == 7 || mode == 8 || mode == 24;
    bool blend =
        mode == 4 || mode == 5 || mode == 9 || mode == 10 || mode == 12 || mode == 14 || mode == 15 || mode == 16 || mode == 18 || mode == 21 || mode == 23;
    context->OMSetBlendState(add ? additive : blend ? alpha : opaque, nullptr, 0xffffffff);
    context->DrawInstanced(mesh.count, UINT(count), 0, 0);
    ++drawCalls;
    submitted += count;
  }
  void draw(Mesh& m, const std::vector<Instance>& v, ID3D11ShaderResourceView* t, int mode) {
    draw(m, v.data(), v.size(), t, mode);
  }
  void rect(float x, float y, float w, float h, float r, float g, float b, float a) {
    Instance i{x + w / 2, y + h / 2, 0, w, h, 1, 0, r, g, b, a, 0};
    draw(quad, &i, 1, white, 5);
  }
  void text(float x, float y, const std::string& s, float size = 18, float r = .72f, float g = .9f,
            float b = .84f) {
    const std::wstring polish = L"ąćęłńóśźżĄĆĘŁŃÓŚŹŻ";
    std::wstring wide(s.size(), 0);
    int n = MultiByteToWideChar(CP_UTF8, 0, s.data(), int(s.size()), wide.data(), int(wide.size()));
    float start = x;
    for (int i = 0; i < n; ++i) {
      wchar_t c = wide[i];
      if (c == '\n') {
        x = start;
        y += size * 1.4f;
        continue;
      }
      int glyph = int(c);
      if (c < 32 || c >= 128) {
        size_t p = polish.find(c);
        glyph = p != std::wstring::npos ? 128 + int(p) : c == L'—' ? '-' : '?';
      }
      ui.push_back(
          {x + size * .5f, y + size * .75f, 0, size, size * 1.5f, 1, 0, r, g, b, 1, float(glyph)});
      x += size * .56f;
    }
  }
  void flushText() {
    draw(quad, ui, font, 4);
    ui.clear();
  }
  void end() {
    if (activeQuery >= 0) {
      auto& q = queries[activeQuery];
      context->End(q.end);
      context->End(q.disjoint);
      q.pending = true;
      activeQuery = -1;
    }
  }
  void present(bool vsync) {
    HRESULT result = swap->Present(vsync ? 1 : 0, 0);
    if (FAILED(result))
      check(result, "Present / device removed");
    if (result == DXGI_STATUS_OCCLUDED)
      SDL_Delay(16);
  }
  void screenshot(const std::filesystem::path& path) {
    Com<ID3D11Texture2D> source;
    check(swap->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)source.put()), "Screenshot buffer");
    D3D11_TEXTURE2D_DESC d{};
    source->GetDesc(&d);
    d.Usage = D3D11_USAGE_STAGING;
    d.BindFlags = 0;
    d.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
    d.MiscFlags = 0;
    Com<ID3D11Texture2D> copy;
    check(device->CreateTexture2D(&d, nullptr, copy.put()), "Screenshot staging");
    context->CopyResource(copy, source);
    D3D11_MAPPED_SUBRESOURCE map;
    check(context->Map(copy, 0, D3D11_MAP_READ, 0, &map), "Screenshot readback");
    std::vector<uint8_t> bgra(size_t(d.Width) * d.Height * 4);
    for (UINT y = 0; y < d.Height; ++y) {
      auto* src = (uint8_t*)map.pData + y * map.RowPitch;
      auto* dst = bgra.data() + size_t(y) * d.Width * 4;
      for (UINT x = 0; x < d.Width; ++x) {
        dst[x * 4] = src[x * 4 + 2];
        dst[x * 4 + 1] = src[x * 4 + 1];
        dst[x * 4 + 2] = src[x * 4];
        dst[x * 4 + 3] = 255;
      }
    }
    context->Unmap(copy, 0);
    Com<IWICStream> stream;
    check(wic->CreateStream(stream.put()), "PNG output");
    check(stream->InitializeFromFilename(path.c_str(), GENERIC_WRITE), "PNG filename");
    Com<IWICBitmapEncoder> encoder;
    check(wic->CreateEncoder(GUID_ContainerFormatPng, nullptr, encoder.put()), "PNG encoder");
    check(encoder->Initialize(stream, WICBitmapEncoderNoCache), "PNG init");
    Com<IWICBitmapFrameEncode> frame;
    check(encoder->CreateNewFrame(frame.put(), nullptr), "PNG frame");
    check(frame->Initialize(nullptr), "PNG frame init");
    check(frame->SetSize(d.Width, d.Height), "PNG size");
    WICPixelFormatGUID format = GUID_WICPixelFormat32bppBGRA;
    check(frame->SetPixelFormat(&format), "PNG format");
    check(frame->WritePixels(d.Height, d.Width * 4, UINT(bgra.size()), bgra.data()), "PNG write");
    check(frame->Commit(), "PNG frame commit");
    check(encoder->Commit(), "PNG commit");
  }
};
} // namespace rdzen
