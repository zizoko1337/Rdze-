#pragma once
#include "manual_scene.hpp"

namespace command {
class SniperMonitor {
  static constexpr UINT WIDTH = 512, HEIGHT = 320;
  Com<ID3D11RenderTargetView> target;
  Com<ID3D11DepthStencilView> depth;
  Com<ID3D11ShaderResourceView> texture;
  float clock = 0;
  bool valid = false;
  void create(Renderer& r) {
    if (target.p)
      return;
    D3D11_TEXTURE2D_DESC desc{};
    desc.Width = WIDTH;
    desc.Height = HEIGHT;
    desc.MipLevels = desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
    Com<ID3D11Texture2D> color;
    check(r.device->CreateTexture2D(&desc, nullptr, color.put()), "Sniper screen texture");
    check(r.device->CreateRenderTargetView(color, nullptr, target.put()), "Sniper screen target");
    check(r.device->CreateShaderResourceView(color, nullptr, texture.put()), "Sniper screen view");
    desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    Com<ID3D11Texture2D> z;
    check(r.device->CreateTexture2D(&desc, nullptr, z.put()), "Sniper screen depth");
    check(r.device->CreateDepthStencilView(z, nullptr, depth.put()), "Sniper screen depth view");
  }

public:
  uint64_t frames = 0;
  void update(Renderer& r, GameScene& worldScene, const Session& s, float dt) {
    if (s.module.kind != Gun::Rail || !s.seated() || s.tactical) {
      valid = false;
      clock = 0;
      return;
    }
    clock += dt;
    if (valid && clock < 1.f / 20)
      return;
    clock = std::fmod(clock, 1.f / 20);
    create(r);
    // Unbind last frame's sampled screen before using it as a render target.
    ID3D11ShaderResourceView* none = nullptr;
    r.context->PSSetShaderResources(0, 1, &none);
    ID3D11RenderTargetView* rt = target;
    r.context->OMSetRenderTargets(1, &rt, depth);
    float clear[] = {.01f, .03f, .03f, 1};
    r.context->ClearRenderTargetView(target, clear);
    r.context->ClearDepthStencilView(depth, D3D11_CLEAR_DEPTH, 1, 0);
    D3D11_VIEWPORT viewport{0, 0, float(WIDTH), float(HEIGHT), 0, 1};
    r.context->RSSetViewports(1, &viewport);
    Constants saved = r.data;
    Camera zoom;
    zoom.eye = s.eye();
    zoom.target = zoom.eye + s.forward();
    zoom.fov = 8; // About 8.3x the main view's focal length.
    zoom.farClip = 5000;
    r.data.vp = zoom.matrix(float(WIDTH) / HEIGHT);
    std::memcpy(r.data.right, &zoom.right, 12);
    std::memcpy(r.data.up, &zoom.up, 12);
    std::memcpy(r.data.eye, &zoom.eye, 12);
    r.data.eye[3] = std::tan(zoom.fov * PI / 360);
    r.data.params[2] = float(WIDTH);
    r.data.params[3] = float(HEIGHT);
    worldScene.render(r); // Prepared world only; no gun, monitor or HUD recursion.
    r.data = saved;
    rt = r.target;
    r.context->OMSetRenderTargets(1, &rt, r.depth);
    viewport.Width = float(r.width);
    viewport.Height = float(r.height);
    r.context->RSSetViewports(1, &viewport);
    valid = true;
    ++frames;
  }
  void draw(Renderer& r, const Session& s) {
    if (s.module.kind == Gun::Flame && s.mode == Mode::Mounted && !s.tactical) {
      // One ground quad; the shader clips it against the real 3D flame cone and 63m range.
      float range = s.flameRange();
      auto footprint = instance(s.eye(), {range * 2, range * 2, std::cos(s.flameHalfAngle())},
                                {1, .32f, .045f}, s.gunYaw, s.gunPitch);
      r.draw(r.quad, &footprint, 1, r.white, 14);
      if (s.module.dualFlame) {
        auto focused = instance(s.eye(), {189, 189, std::cos(std::atan(2.5f / 94.5f))},
                                {1, .65f, .15f}, s.gunYaw, s.gunPitch);
        r.draw(r.quad, &focused, 1, r.white, 14);
      }
    }
    if (!valid || s.module.kind != Gun::Rail || s.tactical)
      return;
    auto screen = instance(s.gunPoint({.95f, .75f, 1.18f}), {1.28f, .8f, 1}, {1, 1, 1}, s.gunYaw,
                           -s.gunPitch);
    r.draw(r.quad, &screen, 1, texture, 13);
  }
};
} // namespace command
