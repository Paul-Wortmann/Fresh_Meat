/**
 * map_validator – Validates map definition files.
 *
 * Scans all .txt files under ./data/map/, parses them in a simple
 * XML‑like format, and checks syntactic and semantic rules defined in
 * documentation/map_schema.md. Reports errors to stderr, prints OK
 * for valid files, and exits with 0 if all pass, non‑zero otherwise.
 *
 * The tool is read‑only and does not modify any files.
 *
 * Limitations (by design):
 *   - Tags must be well‑formed; self‑closing tags (<tag/>) are not supported.
 *   - Each leaf element (<name>value</name>) must appear entirely on one line.
 *   - Attributes are ignored.
 *
 * Validates based on documentation/map_schema.md
 */

#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <vector>

namespace fs = std::filesystem;

namespace {

// --------------------------------------------------------------------
// Helper types
// --------------------------------------------------------------------

struct Issue {
    fs::path file;
    std::string field;
    std::string message;
};

// Holds parsed data for one map file.
struct MapData {
    std::map<std::string, std::string> values;   // all scalar fields (name -> trimmed value)
    std::vector<std::string> map_tiles_rows;     // each <map_tiles> row value (comma‑separated ints)
    std::set<std::string> sections;              // not strictly needed, kept for future use
};

// --------------------------------------------------------------------
// String helpers
// --------------------------------------------------------------------

std::string trim(const std::string& s) {
    const auto first = s.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return {};
    const auto last = s.find_last_not_of(" \t\r\n");
    return s.substr(first, last - first + 1);
}

bool isInteger(const std::string& s) {
    if (s.empty()) return false;
    std::size_t pos = 0;
    try {
        std::stoll(s, &pos);
    } catch (...) {
        return false;
    }
    return pos == s.size();
}

bool isNumber(const std::string& s) {
    if (s.empty()) return false;
    std::size_t pos = 0;
    try {
        std::stod(s, &pos);
    } catch (...) {
        return false;
    }
    return pos == s.size();
}

// Parse a string with two comma‑separated integers (e.g., "30,30").
// Returns true and fills a and b if successful.
bool parseVec2Int(const std::string& value, int& a, int& b) {
    std::stringstream ss(value);
    std::string part;
    std::vector<int> parts;
    while (std::getline(ss, part, ',')) {
        part = trim(part);
        if (!isInteger(part)) return false;
        parts.push_back(std::stoi(part));
        if (parts.size() > 2) return false;
    }
    if (parts.size() != 2) return false;
    a = parts[0];
    b = parts[1];
    return true;
}

// Parse a comma‑separated list of integers.
bool parseTileRow(const std::string& row, std::vector<int>& out, int expectedWidth) {
    out.clear();
    std::stringstream ss(row);
    std::string part;
    while (std::getline(ss, part, ',')) {
        part = trim(part);
        if (!isInteger(part)) return false;
        out.push_back(std::stoi(part));
    }
    if (expectedWidth >= 0 && static_cast<int>(out.size()) != expectedWidth) return false;
    return true;
}

// Check if a file exists (relative to current working directory).
bool fileExists(const std::string& path) {
    return fs::exists(fs::path(path)) && fs::is_regular_file(fs::path(path));
}

// --------------------------------------------------------------------
// Parser – reads one .txt map file and builds a MapData structure
// --------------------------------------------------------------------

bool parseMapFile(const fs::path& file, MapData& data, std::vector<Issue>& issues) {
    std::ifstream input(file);
    if (!input) {
        issues.push_back({file, {}, "Unable to open file."});
        return false;
    }

    std::string line;
    std::vector<std::string> stack;
    bool rootSeen = false;
    bool rootClosed = false;

    while (std::getline(input, line)) {
        line = trim(line);
        if (line.empty()) continue;

        std::size_t pos = 0;
        while ((pos = line.find('<', pos)) != std::string::npos) {
            auto end = line.find('>', pos);
            if (end == std::string::npos) {
                issues.push_back({file, {}, "Malformed tag: missing '>'."});
                return false;
            }

            std::string tag = trim(line.substr(pos + 1, end - pos - 1));
            std::size_t nextPos = end + 1;

            // Skip processing instructions or comments.
            if (tag.empty() || tag[0] == '?' || tag[0] == '!') {
                pos = nextPos;
                continue;
            }

            // ---- Closing tag ----
            if (tag[0] == '/') {
                std::string name = trim(tag.substr(1));
                if (stack.empty() || stack.back() != name) {
                    issues.push_back({file, name, "Mismatched closing tag."});
                    return false;
                }
                stack.pop_back();
                if (name == "map") rootClosed = true;
                pos = nextPos;
                continue;
            }

            // ---- Opening tag ----
            // Strip trailing slash (self‑closing) – treat as empty element.
            if (tag.back() == '/') tag = trim(tag.substr(0, tag.size() - 1));

            // Ignore attributes (keep only the name).
            auto space = tag.find_first_of(" \t");
            std::string name = (space == std::string::npos) ? tag : tag.substr(0, space);

            // ---- Root <map> ----
            if (name == "map") {
                if (rootSeen || !stack.empty()) {
                    issues.push_back({file, "map", "Map file must contain exactly one root <map> element."});
                    return false;
                }
                rootSeen = true;
            } else if (stack.empty()) {
                issues.push_back({file, name, "Element appears outside the root <map> element."});
                return false;
            }

            // Record section (for future extensibility)
            if (name == "map_size" || name == "map_tiles" || name == "playerStartTile" ||
                name == "map_music" || name == "map_texture_atlas_diffuse" ||
                name == "map_texture_atlas_normal" || name == "map_texture_atlas_specular" ||
                name == "map_texture_atlas_size" || name == "map_biome" ||
                name == "entity_wall" || name == "portal_tile" || name == "boss_alert_tile") {
                data.sections.insert(name);
            }

            // ---- Check if this tag is a leaf (opening and closing on the same line) ----
            std::string closingMarker = "</" + name + ">";
            auto closingPos = line.find(closingMarker, nextPos);
            if (closingPos != std::string::npos) {
                std::string value = trim(line.substr(nextPos, closingPos - nextPos));
                // Special handling for repeated <map_tiles>
                if (name == "map_tiles") {
                    data.map_tiles_rows.push_back(value);
                } else {
                    data.values[name] = value;
                }
                pos = closingPos + closingMarker.length();
                continue;
            } else {
                // Container tag (should not happen for map schema, but we push it)
                stack.push_back(name);
                pos = nextPos;
            }
        }
    }

    if (!rootSeen) issues.push_back({file, "map", "Missing root <map> element."});
    if (!rootClosed || !stack.empty()) issues.push_back({file, "map", "Unclosed XML‑like element."});

    return rootSeen && rootClosed && stack.empty();
}

// --------------------------------------------------------------------
// Semantic validation
// --------------------------------------------------------------------

void validateMap(const fs::path& file, const MapData& data, std::vector<Issue>& issues) {
    // Helper to check required scalar field
    auto requiredField = [&](const std::string& name, const std::string& description) {
        auto it = data.values.find(name);
        if (it == data.values.end() || trim(it->second).empty()) {
            issues.push_back({file, name, "Missing required field: " + description});
            return std::string();
        }
        return trim(it->second);
    };

    // ---- map_name ----
    std::string mapName = requiredField("map_name", "map_name");
    if (!mapName.empty() && mapName.find_first_of(" \t") != std::string::npos) {
        issues.push_back({file, "map_name", "map_name should not contain spaces."});
    }

    // ---- map_size ----
    int width = 0, height = 0;
    std::string sizeStr = requiredField("map_size", "map_size");
    if (!sizeStr.empty()) {
        if (!parseVec2Int(sizeStr, width, height)) {
            issues.push_back({file, "map_size", "Expected two positive integers (width,height)."});
        } else if (width <= 0 || height <= 0) {
            issues.push_back({file, "map_size", "Width and height must be greater than zero."});
        }
    }

    // ---- map_tiles rows ----
    if (data.map_tiles_rows.empty()) {
        issues.push_back({file, "map_tiles", "At least one <map_tiles> row is required."});
    } else if (width > 0 && height > 0) {
        if (static_cast<int>(data.map_tiles_rows.size()) != height) {
            issues.push_back({file, "map_tiles",
                "Expected " + std::to_string(height) + " tile rows, found " +
                std::to_string(data.map_tiles_rows.size()) + "."});
        } else {
            // Validate each row length and tile values
            for (size_t rowIdx = 0; rowIdx < data.map_tiles_rows.size(); ++rowIdx) {
                std::vector<int> rowVals;
                if (!parseTileRow(data.map_tiles_rows[rowIdx], rowVals, width)) {
                    issues.push_back({file, "map_tiles",
                        "Row " + std::to_string(rowIdx + 1) + " does not contain exactly " +
                        std::to_string(width) + " integers."});
                } else {
                    for (int val : rowVals) {
                        if (val < 0 || val > 3) {
                            issues.push_back({file, "map_tiles",
                                "Row " + std::to_string(rowIdx + 1) + " contains invalid tile value " +
                                std::to_string(val) + " (must be 0..3)."});
                        }
                    }
                }
            }
        }
    }

    // ---- playerStartTile ----
    int startX = -1, startY = -1;
    std::string startStr = requiredField("playerStartTile", "playerStartTile");
    if (!startStr.empty()) {
        if (!parseVec2Int(startStr, startX, startY)) {
            issues.push_back({file, "playerStartTile", "Expected two integers (x,y)."});
        } else if (width > 0 && height > 0) {
            if (startX < 0 || startX >= width || startY < 0 || startY >= height) {
                issues.push_back({file, "playerStartTile",
                    "Coordinates (" + std::to_string(startX) + "," + std::to_string(startY) +
                    ") are outside map bounds (0.." + std::to_string(width-1) + ",0.." +
                    std::to_string(height-1) + ")."});
            } else {
                // Check that the starting tile is not a wall (tile value 2)
                // We need to access the tile at that position.
                // Since we already validated rows, we can look up the row.
                if (static_cast<size_t>(startY) < data.map_tiles_rows.size()) {
                    std::vector<int> rowVals;
                    if (parseTileRow(data.map_tiles_rows[startY], rowVals, -1)) {
                        if (static_cast<size_t>(startX) < rowVals.size()) {
                            int tileVal = rowVals[startX];
                            if (tileVal == 2) { // wall
                                issues.push_back({file, "playerStartTile",
                                    "Player start tile is a wall (value 2)."});
                            }
                        }
                    }
                }
            }
        }
    }

    // ---- Texture atlases (required) ----
    std::string diffuse = requiredField("map_texture_atlas_diffuse", "diffuse atlas");
    std::string normal = requiredField("map_texture_atlas_normal", "normal atlas");
    std::string specular = requiredField("map_texture_atlas_specular", "specular atlas");
    std::string atlasSizeStr = requiredField("map_texture_atlas_size", "atlas size");

    if (!diffuse.empty() && !fileExists(diffuse))
        issues.push_back({file, "map_texture_atlas_diffuse", "File not found: " + diffuse});
    if (!normal.empty() && !fileExists(normal))
        issues.push_back({file, "map_texture_atlas_normal", "File not found: " + normal});
    if (!specular.empty() && !fileExists(specular))
        issues.push_back({file, "map_texture_atlas_specular", "File not found: " + specular});

    int atlasCols = 0, atlasRows = 0;
    if (!atlasSizeStr.empty()) {
        if (!parseVec2Int(atlasSizeStr, atlasCols, atlasRows)) {
            issues.push_back({file, "map_texture_atlas_size", "Expected two positive integers (columns,rows)."});
        } else if (atlasCols <= 0 || atlasRows <= 0) {
            issues.push_back({file, "map_texture_atlas_size", "Atlas dimensions must be positive."});
        }
        // Note: tile index validation against atlas size is not strictly required by schema,
        // but we could add a warning if tile values exceed columns*rows-1.
        // We'll skip for simplicity.
    }

    // ---- map_music (optional) ----
    auto musicIt = data.values.find("map_music");
    if (musicIt != data.values.end() && !trim(musicIt->second).empty()) {
        if (!fileExists(trim(musicIt->second))) {
            issues.push_back({file, "map_music", "File not found: " + trim(musicIt->second)});
        }
    }

    // ---- map_biome (optional) ----
    auto biomeIt = data.values.find("map_biome");
    if (biomeIt != data.values.end() && !trim(biomeIt->second).empty()) {
        std::string biomeVal = trim(biomeIt->second);
        if (!isInteger(biomeVal)) {
            issues.push_back({file, "map_biome", "Expected an integer."});
        } else {
            int b = std::stoi(biomeVal);
            if (b < 0 || b > 3) {
                issues.push_back({file, "map_biome", "Biome value must be 0..3 (plains, desert, forrest, tundra)."});
            }
        }
    }

    // ---- entity_wall ----
    std::string wallEntity = requiredField("entity_wall", "entity_wall");
    if (!wallEntity.empty()) {
        // Entity files are stored in data/entity/
        fs::path entityPath = fs::path("data") / "entity" / wallEntity;
        if (!fs::exists(entityPath) || !fs::is_regular_file(entityPath)) {
            issues.push_back({file, "entity_wall", "Entity file not found: " + entityPath.string()});
        }
    }

    // ---- portal_tile (optional) ----
    auto portalIt = data.values.find("portal_tile");
    if (portalIt != data.values.end() && !trim(portalIt->second).empty()) {
        int px, py;
        if (!parseVec2Int(trim(portalIt->second), px, py)) {
            issues.push_back({file, "portal_tile", "Expected two integers (x,y)."});
        } else if (width > 0 && height > 0) {
            if (px < 0 || px >= width || py < 0 || py >= height) {
                issues.push_back({file, "portal_tile",
                    "Coordinates (" + std::to_string(px) + "," + std::to_string(py) +
                    ") are outside map bounds."});
            }
        }
    }

    // ---- boss_alert_tile (optional) ----
    auto bossIt = data.values.find("boss_alert_tile");
    if (bossIt != data.values.end() && !trim(bossIt->second).empty()) {
        int bx, by;
        if (!parseVec2Int(trim(bossIt->second), bx, by)) {
            issues.push_back({file, "boss_alert_tile", "Expected two integers (x,y)."});
        } else if (width > 0 && height > 0) {
            if (bx < 0 || bx >= width || by < 0 || by >= height) {
                issues.push_back({file, "boss_alert_tile",
                    "Coordinates (" + std::to_string(bx) + "," + std::to_string(by) +
                    ") are outside map bounds."});
            }
        }
    }
}

} // anonymous namespace

// --------------------------------------------------------------------
// Main
// --------------------------------------------------------------------

int main() {
    const fs::path mapDir = fs::path("data") / "map";
    if (!fs::exists(mapDir) || !fs::is_directory(mapDir)) {
        std::cerr << "ERROR: Map directory not found: " << mapDir << '\n';
        return 1;
    }

    std::vector<fs::path> files;
    for (const auto& entry : fs::recursive_directory_iterator(mapDir)) {
        if (entry.is_regular_file() && entry.path().extension() == ".txt") {
            files.push_back(entry.path());
        }
    }
    std::sort(files.begin(), files.end());

    std::vector<Issue> issues;
    std::map<std::string, fs::path> mapNames; // track duplicate map_name
    size_t validFiles = 0;

    for (const auto& file : files) {
        MapData data;
        size_t before = issues.size();

        bool parsed = parseMapFile(file, data, issues);
        if (parsed) {
            validateMap(file, data, issues);
        }

        // Check duplicate map_name (even if parse failed partly, we may have a name)
        auto nameIt = data.values.find("map_name");
        if (nameIt != data.values.end()) {
            std::string name = trim(nameIt->second);
            if (!name.empty()) {
                auto existing = mapNames.find(name);
                if (existing != mapNames.end()) {
                    issues.push_back({file, "map_name",
                        "Duplicate map name also defined in " + existing->second.string() + "."});
                } else {
                    mapNames.emplace(name, file);
                }
            }
        }

        if (issues.size() == before) {
            ++validFiles;
            std::cout << "OK: " << file.string() << '\n';
        }
    }

    // Report all issues to stderr
    for (const auto& issue : issues) {
        std::cerr << "ERROR: " << issue.file.string();
        if (!issue.field.empty()) std::cerr << ": " << issue.field;
        std::cerr << ": " << issue.message << '\n';
    }

    std::cout << "\nValidated " << files.size() << " map file(s): "
              << validFiles << " passed, " << issues.size() << " error(s).\n";

    return issues.empty() ? 0 : 1;
}

