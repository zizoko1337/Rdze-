#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shellapi.h>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>
#include "bundle_id.hpp"

extern "C" {
extern const unsigned char rdzen_bundle_start[];
extern const unsigned char rdzen_bundle_end[];
}
namespace fs = std::filesystem;

static std::wstring environment(const wchar_t* name) {
  DWORD size = GetEnvironmentVariableW(name, nullptr, 0);
  if (!size)
    return {};
  std::wstring value(size, L'\0');
  DWORD written = GetEnvironmentVariableW(name, value.data(), size);
  if (written >= size)
    throw std::runtime_error("Environment variable changed during startup.");
  value.resize(written);
  return value;
}
static fs::path cacheRoot() {
  auto custom = environment(L"RDZEN_PACKAGE_CACHE");
  if (!custom.empty())
    return fs::path(custom) / RDZEN_BUNDLE_ID;
  auto local = environment(L"LOCALAPPDATA");
  if (local.empty()) {
    wchar_t temporary[MAX_PATH + 1]{};
    if (!GetTempPathW(MAX_PATH, temporary))
      throw std::runtime_error("Cannot find a writable data directory.");
    local = temporary;
  }
  return fs::path(local) / L"Rdzen" / L"Prototyp" / RDZEN_BUNDLE_ID;
}
static void unpack(const fs::path& destination) {
  const unsigned char* cursor = rdzen_bundle_start;
  const unsigned char* end = rdzen_bundle_end;
  auto take = [&](size_t bytes) {
    if (uintptr_t(end) - uintptr_t(cursor) < bytes)
      throw std::runtime_error("Incomplete embedded game data.");
    auto result = cursor;
    cursor += bytes;
    return result;
  };
  auto u32 = [&]() {
    uint32_t v;
    std::memcpy(&v, take(sizeof(v)), sizeof(v));
    return v;
  };
  auto u64 = [&]() {
    uint64_t v;
    std::memcpy(&v, take(sizeof(v)), sizeof(v));
    return v;
  };
  if (std::memcmp(take(8), "RDZENPK1", 8))
    throw std::runtime_error("Invalid embedded game data.");
  uint32_t count = u32();
  fs::create_directories(destination);
  for (uint32_t i = 0; i < count; ++i) {
    uint32_t nameLength = u32();
    uint64_t length = u64();
    std::u8string name(nameLength, u8'\0');
    std::memcpy(name.data(), take(nameLength), nameLength);
    fs::path relative(name);
    if (relative.empty() || relative.is_absolute())
      throw std::runtime_error("Invalid embedded file path.");
    for (const auto& component : relative)
      if (component == L"..")
        throw std::runtime_error("Invalid embedded file path.");
    const auto* data = take(size_t(length));
    fs::path target = destination / relative;
    if (fs::exists(target) && fs::file_size(target) == length)
      continue;
    fs::create_directories(target.parent_path());
    fs::path temporary = target;
    temporary += L"." + std::to_wstring(GetCurrentProcessId()) + L".tmp";
    {
      std::ofstream file(temporary, std::ios::binary | std::ios::trunc);
      file.exceptions(std::ios::badbit | std::ios::failbit);
      file.write(reinterpret_cast<const char*>(data), std::streamsize(length));
      file.close();
    }
    if (!MoveFileExW(temporary.c_str(), target.c_str(),
                     MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH))
      throw std::runtime_error("Cannot save game data.");
  }
  if (cursor != end)
    throw std::runtime_error("Unexpected embedded game data.");
}
static std::wstring quote(const std::wstring& argument) {
  std::wstring result = L"\"";
  size_t slashes = 0;
  for (wchar_t c : argument) {
    if (c == L'\\') {
      ++slashes;
      continue;
    }
    result.append(slashes * (c == L'\"' ? 2 : 1), L'\\');
    slashes = 0;
    if (c == L'\"')
      result += L'\\';
    result += c;
  }
  result.append(slashes * 2, L'\\');
  return result + L'\"';
}
int WINAPI wWinMain(HINSTANCE, HINSTANCE, PWSTR, int) {
  try {
    fs::path data = cacheRoot();
    unpack(data);
    int argc = 0;
    wchar_t** argv = CommandLineToArgvW(GetCommandLineW(), &argc);
    if (!argv)
      throw std::runtime_error("Cannot read startup arguments.");
    bool unpackOnly = argc == 2 && std::wstring(argv[1]) == L"--unpack-only";
    fs::path game = data / L"rdzen-game.exe";
    std::wstring command = quote(game.wstring());
    for (int i = 1; i < argc; ++i)
      command += L" " + quote(argv[i]);
    LocalFree(argv);
    if (unpackOnly)
      return 0;
    STARTUPINFOW startup{};
    startup.cb = sizeof(startup);
    PROCESS_INFORMATION process{};
    if (!CreateProcessW(game.c_str(), command.data(), nullptr, nullptr, false, CREATE_NO_WINDOW,
                        nullptr, data.c_str(), &startup, &process))
      throw std::runtime_error("Cannot start the game. Windows error: " +
                               std::to_string(GetLastError()));
    CloseHandle(process.hThread);
    WaitForSingleObject(process.hProcess, INFINITE);
    DWORD code = 1;
    GetExitCodeProcess(process.hProcess, &code);
    CloseHandle(process.hProcess);
    return int(code);
  } catch (const std::exception& error) {
    std::wstring details;
    for (unsigned char c : std::string(error.what()))
      details += wchar_t(c);
    MessageBoxW(nullptr, (L"Nie udalo sie uruchomic gry.\n\n" + details).c_str(), L"RDZEN",
                MB_OK | MB_ICONERROR);
    return 1;
  }
}
