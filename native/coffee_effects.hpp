#pragma once
#include "renderer.hpp"

namespace command {
class CoffeeEffects {
  rdzen::Com<ID3D11Texture2D> source, horizontal;
  rdzen::Com<ID3D11ShaderResourceView> sourceView, horizontalView;
  rdzen::Com<ID3D11RenderTargetView> horizontalTarget;
  int width = 0, height = 0;

public:
  void blur(rdzen::Renderer& r, float fatigue) {
    if (fatigue <= 0)
      return;
    using namespace rdzen;
    ID3D11ShaderResourceView* none = nullptr;
    r.context->PSSetShaderResources(0, 1, &none);
    if (width != r.width || height != r.height) {
      D3D11_TEXTURE2D_DESC d{};
      d.Width = r.width;
      d.Height = r.height;
      d.MipLevels = d.ArraySize = d.SampleDesc.Count = 1;
      d.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
      d.BindFlags = D3D11_BIND_SHADER_RESOURCE;
      check(r.device->CreateTexture2D(&d, nullptr, source.put()), "Coffee blur source");
      check(r.device->CreateShaderResourceView(source, nullptr, sourceView.put()),
            "Coffee source view");
      d.BindFlags |= D3D11_BIND_RENDER_TARGET;
      check(r.device->CreateTexture2D(&d, nullptr, horizontal.put()), "Coffee blur intermediate");
      check(r.device->CreateShaderResourceView(horizontal, nullptr, horizontalView.put()),
            "Coffee intermediate view");
      check(r.device->CreateRenderTargetView(horizontal, nullptr, horizontalTarget.put()),
            "Coffee intermediate target");
      width = r.width;
      height = r.height;
    }
    Com<ID3D11Texture2D> backbuffer;
    check(r.swap->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)backbuffer.put()),
          "Coffee frame buffer");
    r.context->OMSetRenderTargets(0, nullptr, nullptr);
    r.context->CopyResource(source, backbuffer);
    Instance screen{r.width * .5f,
                    r.height * .5f,
                    0,
                    float(r.width),
                    float(r.height),
                    1,
                    0,
                    fatigue,
                    1,
                    1,
                    1,
                    0};
    ID3D11RenderTargetView* rt = horizontalTarget;
    r.context->OMSetRenderTargets(1, &rt, nullptr);
    r.draw(r.quad, &screen, 1, sourceView, 19);
    rt = r.target;
    r.context->OMSetRenderTargets(1, &rt, r.depth);
    r.draw(r.quad, &screen, 1, horizontalView, 20);
    r.context->PSSetShaderResources(0, 1, &none);
  }
  void eyelids(rdzen::Renderer& r, float closure) {
    if (closure <= 0)
      return;
    rdzen::Instance screen{r.width * .5f,
                           r.height * .5f,
                           0,
                           float(r.width),
                           float(r.height),
                           1,
                           0,
                           0,
                           0,
                           0,
                           closure,
                           0};
    r.draw(r.quad, &screen, 1, r.white, 21);
  }
};
} // namespace command
