#pragma once
#include "command_scene.hpp"
#include <cctype>
namespace command {
inline std::string upgradeSearchText(std::string value) {
  constexpr const char* accents[] = {"ą", "ć", "ę", "ł", "ń", "ó", "ś", "ź", "ż",
                                     "Ą", "Ć", "Ę", "Ł", "Ń", "Ó", "Ś", "Ź", "Ż"};
  constexpr const char* plain = "acelnoszzacelnoszz";
  for (int i = 0; i < 18; ++i) {
    size_t found = 0;
    while ((found = value.find(accents[i], found)) != std::string::npos) {
      value.replace(found, std::strlen(accents[i]), 1, plain[i]);
      ++found;
    }
  }
  for (auto& ch : value)
    if (static_cast<unsigned char>(ch) < 128)
      ch = char(std::tolower(static_cast<unsigned char>(ch)));
  return value;
}
struct UpgradeLab {
  std::string query, notice;
  int selected = 0;
  Rarity rarity = Rarity::Legendary;
  std::vector<Upgrade> results() const {
    std::vector<Upgrade> result;
    auto needle = upgradeSearchText(query);
    for (int i = 0; i < int(Upgrade::Count); ++i) {
      const auto& card = UPGRADES[i];
      auto text =
          upgradeSearchText(std::string(card.name) + " " + card.category + " " + card.description);
      if (text.find(needle) != std::string::npos)
        result.push_back(Upgrade(i));
    }
    return result;
  }
  void text(const char* value) {
    if (query.size() + std::strlen(value) > 96)
      return;
    if (query.size() < 96) {
      for (const char* ch = value; *ch; ++ch)
        if (*ch != ';')
          query += *ch;
      selected = 0;
    }
  }
  void backspace() {
    if (query.empty())
      return;
    size_t first = query.size() - 1;
    while (first && (static_cast<unsigned char>(query[first]) & 0xC0) == 0x80)
      --first;
    query.erase(first);
    selected = 0;
  }
  void move(int amount) {
    auto cards = results();
    selected = std::clamp(selected + amount, 0, std::max(0, int(cards.size()) - 1));
  }
  void grant(Session& s) {
    auto cards = results();
    if (cards.empty())
      return;
    selected = std::clamp(selected, 0, int(cards.size()) - 1);
    Upgrade kind = cards[selected];
    bool ok = s.debugGrant(kind, rarity);
    notice = ok ? std::string("Dodano: ") + UPGRADES[size_t(kind)].name
             : kind == Upgrade::PlasmaMortar && s.module.kind != Gun::Pulse
                 ? "Ta przemiana wymaga manualnego lasera."
                 : "Niedostępne: moduł już zdobyty, brak wymagania albo pełna plansza.";
  }
  static Rect row(const GameUi& ui, int index) {
    return {60, 218 + index * 61.f, ui.width * .45f - 60, 55};
  }
  static Rect tier(const GameUi& ui, int index) {
    return {ui.width * .48f + index * 139, 152, 130, 43};
  }
  static Rect addButton(const GameUi& ui) { return {ui.width * .49f, 600, 565, 55}; }
  void click(GameUi& ui, Session& s, float px, float py) {
    float x = px / ui.scale, y = py / ui.scale;
    for (int i = 0; i < 5; ++i)
      if (tier(ui, i).contains(x, y)) {
        rarity = Rarity(i);
        return;
      }
    if (addButton(ui).contains(x, y)) {
      grant(s);
      return;
    }
    auto cards = results();
    int first = selected / 8 * 8;
    for (int i = 0; i < 8 && first + i < int(cards.size()); ++i)
      if (row(ui, i).contains(x, y)) {
        selected = first + i;
        return;
      }
  }
  void draw(GameUi& ui, Renderer& r, const Session& s) {
    ui.resize(r.width, r.height);
    ui.rect(r, {0, 0, ui.width, ui.height}, {.008f, .025f, .035f}, .98f);
    ui.text(r, 60, 48, "LABORATORIUM ULEPSZEŃ / TRYB TESTOWY", 30, GameUi::mint);
    ui.text(r, 60, 96,
            "[;] / Esc zamknij   |   Strzałki: lista   |   Tab: rzadkość   |   Enter: dodaj", 15,
            GameUi::muted);
    ui.panel(r, {60, 145, ui.width * .45f - 60, 54});
    ui.text(r, 77, 162, query.empty() ? "Wpisz nazwę lub efekt ulepszenia..." : query + "_", 17,
            GameUi::white);
    auto cards = results();
    selected = std::clamp(selected, 0, std::max(0, int(cards.size()) - 1));
    int first = selected / 8 * 8;
    for (int i = 0; i < 8 && first + i < int(cards.size()); ++i) {
      Upgrade kind = cards[first + i];
      auto b = row(ui, i);
      ui.panel(r, b);
      Color c = first + i == selected ? GameUi::mint : GameUi::muted;
      if (first + i == selected)
        ui.rect(r, {b.x, b.y, 4, b.h}, c);
      ui.text(r, b.x + 17, b.y + 10, UPGRADES[size_t(kind)].name, 18, c);
      ui.text(r, b.x + 17, b.y + 35,
              std::string(UPGRADES[size_t(kind)].category) +
                  "  |  zdobyte: " + std::to_string(s.expedition.levels[size_t(kind)]),
              10, GameUi::muted);
    }
    for (int i = 0; i < 5; ++i) {
      auto b = tier(ui, i);
      ui.panel(r, b);
      if (int(rarity) == i)
        ui.rect(r, {b.x, b.y, b.w, 3}, RARITIES[i].color);
      ui.text(r, b.x + 10, b.y + 15, RARITIES[i].name, 13, RARITIES[i].color);
    }
    float x = ui.width * .49f;
    if (!cards.empty()) {
      auto kind = cards[selected];
      Rarity effective = std::max(rarity, minimumRarity(kind));
      if (kind == Upgrade::FlatDamage || kind == Upgrade::Luck)
        effective = Rarity::Common;
      if (kind == Upgrade::ServiceCredit)
        effective = Rarity::Rare;
      ui.text(r, x, 237, UPGRADES[size_t(kind)].name, 27, GameUi::white);
      ui.text(r, x, 286, std::string("Rzadkość dodania: ") + RARITIES[int(effective)].name, 16,
              RARITIES[int(effective)].color);
      ui.paragraph(r, x, 332, upgradeDescription(kind, effective), 62, 17, GameUi::muted);
      ui.panel(r, addButton(ui));
      ui.text(r, x + 20, 619, "[ENTER] DODAJ ULEPSZENIE", 20, GameUi::mint);
    } else
      ui.text(r, x, 245, "Brak wyników.", 24, GameUi::muted);
    ui.paragraph(r, 60, ui.height - 145, notice, 100, 16, GameUi::mint);
    ui.text(r, 60, ui.height - 75,
            "Czas stoi. Dodawanie nie zużywa nagród. Wieżyczki po zamknięciu panelu rozmieść na "
            "heksach.",
            14, GameUi::muted);
    ui.text(r, 60, ui.height - 46,
            "Lista: " + std::to_string(cards.size()) + " / " + std::to_string(int(Upgrade::Count)) +
                "  |  Pokrętło myszy przewija listę. Limity rodzajów wieżyczek są pomijane tylko "
                "przy ręcznym dodawaniu.",
            12, GameUi::muted);
    r.flushText();
  }
};
} // namespace command
