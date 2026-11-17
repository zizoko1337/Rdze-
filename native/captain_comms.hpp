#pragma once
#include "command_scene.hpp"
namespace command {
class CaptainComms {
  Com<ID3D11ShaderResourceView> closed, open;

public:
  CaptainComms(Renderer& r, const std::filesystem::path& assets) {
    r.loadTexture(assets / "captain-closed.png", closed, false);
    r.loadTexture(assets / "captain-open.png", open, false);
  }
  void draw(Renderer& r, CommandUi& ui, const Session& s, bool help = false) {
    ui.captainVisible = false;
    if (!s.expedition.enabled || s.mode == Mode::Selection || s.expedition.dead() || help ||
        s.terminalOpen || s.servicesOpen || s.radioOpen || s.upgradeLabOpen || s.bestiaryOpen ||
        s.coffee.active)
      return;
    const auto& mutations = s.frontier.mutations;
    const auto* notice = mutations.current();
    if (!notice)
      return;
    ui.captainVisible = true;
    const auto panel = ui.captainPanel();
    float x = panel.x, y = panel.y;
    Color color = RARITIES[int(notice->rarity)].color;
    ui.panel(r, panel);
    ui.rect(r, {x, y, 328, 3}, color);
    ui.text(r, x + 12, y + 11, "BAZA / KAPITAN", 11, GameUi::white);
    ui.text(r, x + 197, y + 11, RARITIES[int(notice->rarity)].name, 10, color);
    ui.text(r, x + 12, y + 29, "PRZECIWNICY MUTUJĄ", 13, color);
    bool talking = !s.blocked() && EnemyMutationSystem::mouthOpen(mutations.noticeAge);
    float scale = ui.scale;
    auto portrait =
        instance({(x + 55) * scale, (y + 97) * scale, 0}, {86 * scale, 86 * scale, 1}, {1, 1, 1});
    r.draw(r.quad, &portrait, 1, talking ? open : closed, 16);
    ui.paragraph(r, x + 110, y + 53, notice->title, 27, 14, color);
    ui.paragraph(r, x + 110, y + 91, notice->description, 35, 11, GameUi::white);
    for (int n = 0; n < 16; ++n) {
      float wave = talking ? 2 + 5 * std::abs(std::sin(mutations.noticeAge * 17 + n * 1.3f)) : 2;
      ui.rect(r, {x + 14 + n * 4.8f, y + 151 - wave * .5f, 2, wave}, color, .8f);
    }
    r.flushText();
  }
};
} // namespace command
