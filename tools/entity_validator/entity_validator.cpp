/**
 * entity_validator – Validates entity definition files.
 *
 * Scans all .txt files under ./data/entity/, parses them in a simple
 * XML‑like format, and checks syntactic and semantic rules defined in
 * documentation/entity_schema.md. Reports errors to stderr, prints OK
 * for valid files, and exits with 0 if all pass, non‑zero otherwise.
 *
 * The tool is read‑only and does not modify any files.
 *
 * Limitations (by design):
 *   - Tags must be well‑formed; self‑closing tags (<tag/>) are not supported.
 *   - Each leaf element (<name>value</name>) must appear entirely on one line.
 *   - Sections like <base> and <physics> are markers only; they must be
 *     written as empty elements (<base></base>) and contain no child tags.
 *   - Attributes are ignored.
 *
 * Validates based on documentation/entity_schema.md
 *
 */
 
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <vector>

namespace fs = std::filesystem;

namespace
{
    /**
     * Represents a single validation issue.
     */
    struct sIssue
    {
        fs::path file;      // File where the issue occurred
        std::string field;  // Optional field name (e.g., "entity_name")
        std::string message;// Human‑readable error description
    };

    /**
     * Holds the parsed data of an entity.
     *   values  : map from field name to trimmed value string
     *   sections: set of section names encountered (e.g., "base", "physics")
     */
    struct sEntity
    {
        std::map<std::string, std::string> values;
        std::set<std::string> sections;
    };

    // --------------------------------------------------------------------
    // Helper functions for string manipulation and type checking
    // --------------------------------------------------------------------

    /**
     * Removes leading and trailing whitespace (space, tab, CR, LF).
     * Returns an empty string if the input consists only of whitespace.
     */
    std::string trim(const std::string& value)
    {
        const auto first = value.find_first_not_of(" \t\r\n");
        if (first == std::string::npos)
            return {};

        const auto last = value.find_last_not_of(" \t\r\n");
        return value.substr(first, last - first + 1);
    }

    /**
     * Checks if a string represents a valid floating‑point number.
     * Uses std::stod and verifies that the entire string was consumed.
     */
    bool isNumber(const std::string& value)
    {
        if (value.empty())
            return false;

        std::size_t position = 0;
        try
        {
            std::stod(value, &position);
        }
        catch (...)
        {
            return false;
        }

        return position == value.size();
    }

    /**
     * Checks if a string represents a valid integer (64‑bit signed).
     */
    bool isInteger(const std::string& value)
    {
        if (value.empty())
            return false;

        std::size_t position = 0;
        try
        {
            std::stoll(value, &position);
        }
        catch (...)
        {
            return false;
        }

        return position == value.size();
    }

    /**
     * Checks if a string is a three‑component numeric vector (e.g., "1.0,2,3.5").
     * Each component is trimmed and tested with isNumber().
     */
    bool isVec3(const std::string& value)
    {
        std::stringstream stream(value);
        std::string component;
        int count = 0;

        while (std::getline(stream, component, ','))
        {
            if (!isNumber(trim(component)))
                return false;
            ++count;
        }

        return count == 3;
    }

    /**
     * Helper to append a new issue to the global issues vector.
     */
    void addIssue(std::vector<sIssue>& issues, const fs::path& file, const std::string& field, const std::string& message)
    {
        issues.push_back({file, field, message});
    }

    // --------------------------------------------------------------------
    // Parser – reads a single file and builds an sEntity structure
    // --------------------------------------------------------------------

    /**
     * Parses a .txt entity file.
     *
     * The format is a flat XML‑like structure:
     *   <entity>
     *     <base></base>
     *     <entity_name>Foo</entity_name>
     *     <entity_type>1</entity_type>
     *     ...
     *   </entity>
     *
     * Sections like <base> are empty markers; all data fields are direct
     * children of <entity>. Each leaf field must be on its own line with
     * both opening and closing tags (e.g., <name>value</name>).
     *
     * Returns true if the file was parsed successfully (syntactically valid).
     * In case of errors, issues are appended and false is returned.
     *
     * Known limitations:
     *   - Self‑closing tags (e.g., <base/>) are not supported correctly.
     *   - Tags cannot span multiple lines.
     *   - Attributes are ignored.
     */
    bool parseEntity(const fs::path& file, sEntity& entity, std::vector<sIssue>& issues)
    {
        std::ifstream input(file);
        if (!input)
        {
            addIssue(issues, file, {}, "Unable to open file.");
            return false;
        }

        std::string line;
        std::vector<std::string> stack;
        bool rootSeen = false;
        bool rootClosed = false;

        while (std::getline(input, line))
        {
            line = trim(line);
            if (line.empty())
                continue;

            // Scan the line for tags sequentially.
            std::size_t pos = 0;
            while ((pos = line.find('<', pos)) != std::string::npos)
            {
                const auto end = line.find('>', pos);
                if (end == std::string::npos)
                {
                    addIssue(issues, file, {}, "Malformed tag: missing '>'.");
                    return false;
                }

                std::string tag = trim(line.substr(pos + 1, end - pos - 1));
                const std::size_t nextPos = end + 1; // position after this '>'

                // Skip processing instructions/comments.
                if (tag.empty() || tag[0] == '?' || tag[0] == '!')
                {
                    pos = nextPos;
                    continue;
                }

                // ---- Closing tag ----
                if (tag[0] == '/')
                {
                    const std::string name = trim(tag.substr(1));
                    if (stack.empty() || stack.back() != name)
                    {
                        addIssue(issues, file, name, "Mismatched closing tag.");
                        return false;
                    }
                    stack.pop_back();
                    if (name == "entity")
                        rootClosed = true;
                    pos = nextPos;
                    continue;
                }

                // ---- Opening tag ----
                // Strip trailing slash (self‑closing) – treat as empty element.
                if (tag.back() == '/')
                    tag = trim(tag.substr(0, tag.size() - 1));

                // Extract tag name (ignore attributes).
                const auto space = tag.find_first_of(" \t");
                const std::string name = space == std::string::npos ? tag : tag.substr(0, space);

                // ---- Check for root <entity> ----
                if (name == "entity")
                {
                    if (rootSeen || !stack.empty())
                    {
                        addIssue(issues, file, "entity", "Entity file must contain exactly one root <entity> element.");
                        return false;
                    }
                    rootSeen = true;
                }
                else if (stack.empty())
                {
                    addIssue(issues, file, name, "Element appears outside the root <entity> element.");
                    return false;
                }

                // Record section markers.
                if (name == "base" || name == "animation" || name == "audio" || name == "graphics" || name == "physics")
                    entity.sections.insert(name);

                // ---- Check if this tag is a leaf (opening and closing on the same line) ----
                // Look for a matching closing tag </name> after the opening '>'.
                const std::string closingMarker = "</" + name + ">";
                const auto closingPos = line.find(closingMarker, nextPos);
                if (closingPos != std::string::npos)
                {
                    // Extract the value between the opening and closing tags.
                    const std::string value = trim(line.substr(nextPos, closingPos - nextPos));
                    entity.values[name] = value;

                    // Skip over the closing tag so it isn't processed again.
                    pos = closingPos + closingMarker.length();
                    // Do NOT push this leaf tag onto the stack – it's a self‑contained leaf.
                    continue;
                }
                else
                {
                    // No matching closing tag on this line – push it as a container/section.
                    stack.push_back(name);
                    pos = nextPos;
                }
            }

            // The old leaf‑extraction block is removed entirely.
        }

        // Post‑parse checks.
        if (!rootSeen)
            addIssue(issues, file, "entity", "Missing root <entity> element.");
        if (!rootClosed || !stack.empty())
            addIssue(issues, file, "entity", "Unclosed XML‑like element.");

        return rootSeen && rootClosed && stack.empty();
    }

    // --------------------------------------------------------------------
    // Semantic validation – checks field types, ranges, and dependencies
    // --------------------------------------------------------------------

    /**
     * Validates the parsed entity against the schema rules.
     * Appends issues for any violations.
     */
    void validateEntity(const fs::path& file, const sEntity& entity, std::vector<sIssue>& issues)
    {
        // Required section: base
        if (!entity.sections.contains("base"))
            addIssue(issues, file, "base", "Missing required <base> section.");

        // Required fields
        const auto name = entity.values.find("entity_name");
        if (name == entity.values.end() || trim(name->second).empty())
            addIssue(issues, file, "entity_name", "Missing required entity_name.");

        const auto type = entity.values.find("entity_type");
        if (type == entity.values.end() || !isInteger(trim(type->second)))
            addIssue(issues, file, "entity_type", "entity_type must be an integer.");

        // Type‑specific checks for known physics and graphics fields
        for (const auto& [field, value] : entity.values)
        {
            const std::string trimmed = trim(value);

            // These fields must be 3‑component vectors
            if (field == "graphics_scale" ||
                field == "physics_position" || field == "physics_velocity" ||
                field == "physics_acceleration" || field == "physics_deceleration" ||
                field == "physics_direction" || field == "physics_max_velocity" ||
                field == "physics_max_acceleration" || field == "physics_max_deceleration")
            {
                if (!isVec3(trimmed))
                    addIssue(issues, file, field, "Expected a three-component numeric vector.");
            }
            // These fields must be single numeric values
            else if (field == "physics_radius" || field == "physics_angle" ||
                     field == "physics_angular_velocity" || field == "physics_mass" ||
                     field == "physics_friction" || field == "physics_restitution")
            {
                if (!isNumber(trimmed))
                    addIssue(issues, file, field, "Expected a numeric value.");
            }
        }

        // physics_body_type: must be 'static' or 'dynamic'
        const auto bodyType = entity.values.find("physics_body_type");
        if (bodyType != entity.values.end())
        {
            const std::string value = trim(bodyType->second);
            if (value != "static" && value != "dynamic")
                addIssue(issues, file, "physics_body_type", "Unsupported body type. Expected static or dynamic.");

            // Dynamic bodies require a positive mass
            if (value == "dynamic")
            {
                const auto mass = entity.values.find("physics_mass");
                if (mass == entity.values.end() || !isNumber(trim(mass->second)) || std::stod(trim(mass->second)) <= 0.0)
                    addIssue(issues, file, "physics_mass", "Dynamic bodies require a positive mass.");
            }
        }

        // physics_shape: must be 'circle' or 'aabb'
        const auto shape = entity.values.find("physics_shape");
        if (shape != entity.values.end())
        {
            const std::string value = trim(shape->second);
            if (value != "circle" && value != "aabb")
                addIssue(issues, file, "physics_shape", "Unsupported shape. Expected circle or aabb.");

            // Circle shapes require a positive radius
            if (value == "circle")
            {
                const auto radius = entity.values.find("physics_radius");
                if (radius == entity.values.end() || !isNumber(trim(radius->second)) || std::stod(trim(radius->second)) <= 0.0)
                    addIssue(issues, file, "physics_radius", "Circle shapes require a positive radius.");
            }
        }

        // Friction must be non‑negative
        const auto friction = entity.values.find("physics_friction");
        if (friction != entity.values.end() && isNumber(trim(friction->second)) && std::stod(trim(friction->second)) < 0.0)
            addIssue(issues, file, "physics_friction", "Friction must not be negative.");

        // Restitution must be in [0,1]
        const auto restitution = entity.values.find("physics_restitution");
        if (restitution != entity.values.end() && isNumber(trim(restitution->second)))
        {
            const double value = std::stod(trim(restitution->second));
            if (value < 0.0 || value > 1.0)
                addIssue(issues, file, "physics_restitution", "Restitution must be between 0 and 1.");
        }
    }
}

// --------------------------------------------------------------------
// Main – driver that walks the directory, processes files, and reports
// --------------------------------------------------------------------

int main()
{
    // The entity directory is fixed at ./data/entity relative to the working directory
    const fs::path entityDirectory = fs::path("data") / "entity";
    if (!fs::exists(entityDirectory) || !fs::is_directory(entityDirectory))
    {
        std::cerr << "ERROR: Entity directory not found: " << entityDirectory << '\n';
        return 1;
    }

    // Collect all .txt files recursively
    std::vector<fs::path> files;
    for (const auto& entry : fs::recursive_directory_iterator(entityDirectory))
    {
        if (entry.is_regular_file() && entry.path().extension() == ".txt")
            files.push_back(entry.path());
    }
    std::sort(files.begin(), files.end());

    std::vector<sIssue> issues;
    std::map<std::string, fs::path> entityNames;  // track duplicate entity names globally
    std::size_t validFiles = 0;

    // Process each file
    for (const auto& file : files)
    {
        sEntity entity;
        const std::size_t before = issues.size();
        const bool parsed = parseEntity(file, entity, issues);

        if (parsed)
            validateEntity(file, entity, issues);

        // Check for duplicate entity_name (even if file had parse errors, we may still have a name)
        const auto name = entity.values.find("entity_name");
        if (name != entity.values.end() && !trim(name->second).empty())
        {
            const std::string entityName = trim(name->second);
            const auto existing = entityNames.find(entityName);
            if (existing != entityNames.end())
                addIssue(issues, file, "entity_name", "Duplicate entity name also defined in " + existing->second.string() + ".");
            else
                entityNames.emplace(entityName, file);
        }

        // If no new issues were added for this file, it passed
        if (issues.size() == before)
        {
            ++validFiles;
            std::cout << "OK: " << file.string() << '\n';
        }
    }

    // Report all accumulated issues to stderr
    for (const auto& issue : issues)
    {
        std::cerr << "ERROR: " << issue.file.string();
        if (!issue.field.empty())
            std::cerr << ": " << issue.field;
        std::cerr << ": " << issue.message << '\n';
    }

    // Summary
    std::cout << "\nValidated " << files.size() << " entity file(s): "
              << validFiles << " passed, " << issues.size() << " error(s).\n";

    // Exit with 0 if no issues, otherwise 1
    return issues.empty() ? 0 : 1;
}

