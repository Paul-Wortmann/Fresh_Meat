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
struct sIssue
{
    fs::path file;
    std::string field;
    std::string message;
};

struct sEntity
{
    std::map<std::string, std::string> values;
    std::set<std::string> sections;
};

std::string trim(const std::string& value)
{
    const auto first = value.find_first_not_of(" \t\r\n");
    if (first == std::string::npos)
        return {};

    const auto last = value.find_last_not_of(" \t\r\n");
    return value.substr(first, last - first + 1);
}

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

void addIssue(std::vector<sIssue>& issues, const fs::path& file, const std::string& field, const std::string& message)
{
    issues.push_back({file, field, message});
}

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

        std::size_t position = 0;
        while ((position = line.find('<', position)) != std::string::npos)
        {
            const auto end = line.find('>', position);
            if (end == std::string::npos)
            {
                addIssue(issues, file, {}, "Malformed tag: missing '>'.");
                return false;
            }

            std::string tag = trim(line.substr(position + 1, end - position - 1));
            position = end + 1;

            if (tag.empty() || tag[0] == '?' || tag[0] == '!')
                continue;

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
                continue;
            }

            if (tag.back() == '/')
                tag = trim(tag.substr(0, tag.size() - 1));

            const auto space = tag.find_first_of(" \t");
            const std::string name = space == std::string::npos ? tag : tag.substr(0, space);

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

            if (name == "base" || name == "animation" || name == "audio" || name == "graphics" || name == "physics")
                entity.sections.insert(name);

            stack.push_back(name);
        }

        // Entity data uses one opening tag, value, and closing tag per line.
        // Extract leaf values without pretending this small XML-like format is full XML.
        const auto firstOpen = line.find('<');
        const auto firstClose = line.find('>', firstOpen == std::string::npos ? 0 : firstOpen);
        if (firstOpen != std::string::npos && firstClose != std::string::npos)
        {
            const auto valueStart = firstClose + 1;
            const auto closingOpen = line.find("</", valueStart);
            if (closingOpen != std::string::npos)
            {
                const auto closingEnd = line.find('>', closingOpen);
                if (closingEnd != std::string::npos)
                {
                    const std::string openingTag = trim(line.substr(firstOpen + 1, firstClose - firstOpen - 1));
                    const std::string closingTag = trim(line.substr(closingOpen + 2, closingEnd - closingOpen - 2));
                    const auto space = openingTag.find_first_of(" \t");
                    const std::string field = space == std::string::npos ? openingTag : openingTag.substr(0, space);

                    if (field != closingTag)
                    {
                        addIssue(issues, file, field, "Opening and closing tags do not match.");
                        return false;
                    }

                    if (stack.empty() || stack.back() != field)
                    {
                        addIssue(issues, file, field, "Unexpected closing tag.");
                        return false;
                    }

                    entity.values[field] = trim(line.substr(valueStart, closingOpen - valueStart));
                    stack.pop_back();
                }
            }
        }
    }

    if (!rootSeen)
        addIssue(issues, file, "entity", "Missing root <entity> element.");
    if (!rootClosed || !stack.empty())
        addIssue(issues, file, "entity", "Unclosed XML-like element.");

    return rootSeen && rootClosed && stack.empty();
}

void validateEntity(const fs::path& file, const sEntity& entity, std::vector<sIssue>& issues)
{
    if (!entity.sections.contains("base"))
        addIssue(issues, file, "base", "Missing required <base> section.");

    const auto name = entity.values.find("entity_name");
    if (name == entity.values.end() || trim(name->second).empty())
        addIssue(issues, file, "entity_name", "Missing required entity_name.");

    const auto type = entity.values.find("entity_type");
    if (type == entity.values.end() || !isInteger(trim(type->second)))
        addIssue(issues, file, "entity_type", "entity_type must be an integer.");

    for (const auto& [field, value] : entity.values)
    {
        const std::string trimmed = trim(value);

        if (field == "graphics_scale" ||
            field == "physics_position" || field == "physics_velocity" ||
            field == "physics_acceleration" || field == "physics_deceleration" ||
            field == "physics_direction" || field == "physics_max_velocity" ||
            field == "physics_max_acceleration" || field == "physics_max_deceleration")
        {
            if (!isVec3(trimmed))
                addIssue(issues, file, field, "Expected a three-component numeric vector.");
        }
        else if (field == "physics_radius" || field == "physics_angle" ||
                 field == "physics_angular_velocity" || field == "physics_mass" ||
                 field == "physics_friction" || field == "physics_restitution")
        {
            if (!isNumber(trimmed))
                addIssue(issues, file, field, "Expected a numeric value.");
        }
    }

    const auto bodyType = entity.values.find("physics_body_type");
    if (bodyType != entity.values.end())
    {
        const std::string value = trim(bodyType->second);
        if (value != "static" && value != "dynamic")
            addIssue(issues, file, "physics_body_type", "Unsupported body type. Expected static or dynamic.");

        if (value == "dynamic")
        {
            const auto mass = entity.values.find("physics_mass");
            if (mass == entity.values.end() || !isNumber(trim(mass->second)) || std::stod(trim(mass->second)) <= 0.0)
                addIssue(issues, file, "physics_mass", "Dynamic bodies require a positive mass.");
        }
    }

    const auto shape = entity.values.find("physics_shape");
    if (shape != entity.values.end())
    {
        const std::string value = trim(shape->second);
        if (value != "circle" && value != "aabb")
            addIssue(issues, file, "physics_shape", "Unsupported shape. Expected circle or aabb.");

        if (value == "circle")
        {
            const auto radius = entity.values.find("physics_radius");
            if (radius == entity.values.end() || !isNumber(trim(radius->second)) || std::stod(trim(radius->second)) <= 0.0)
                addIssue(issues, file, "physics_radius", "Circle shapes require a positive radius.");
        }
    }

    const auto friction = entity.values.find("physics_friction");
    if (friction != entity.values.end() && isNumber(trim(friction->second)) && std::stod(trim(friction->second)) < 0.0)
        addIssue(issues, file, "physics_friction", "Friction must not be negative.");

    const auto restitution = entity.values.find("physics_restitution");
    if (restitution != entity.values.end() && isNumber(trim(restitution->second)))
    {
        const double value = std::stod(trim(restitution->second));
        if (value < 0.0 || value > 1.0)
            addIssue(issues, file, "physics_restitution", "Restitution must be between 0 and 1.");
    }
}
}

int main()
{
    const fs::path entityDirectory = fs::path("data") / "entity";
    if (!fs::exists(entityDirectory) || !fs::is_directory(entityDirectory))
    {
        std::cerr << "ERROR: Entity directory not found: " << entityDirectory << '\n';
        return 1;
    }

    std::vector<fs::path> files;
    for (const auto& entry : fs::recursive_directory_iterator(entityDirectory))
    {
        if (entry.is_regular_file() && entry.path().extension() == ".txt")
            files.push_back(entry.path());
    }
    std::sort(files.begin(), files.end());

    std::vector<sIssue> issues;
    std::map<std::string, fs::path> entityNames;
    std::size_t validFiles = 0;

    for (const auto& file : files)
    {
        sEntity entity;
        const std::size_t before = issues.size();
        const bool parsed = parseEntity(file, entity, issues);

        if (parsed)
            validateEntity(file, entity, issues);

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

        if (issues.size() == before)
        {
            ++validFiles;
            std::cout << "OK: " << file.string() << '\n';
        }
    }

    for (const auto& issue : issues)
    {
        std::cerr << "ERROR: " << issue.file.string();
        if (!issue.field.empty())
            std::cerr << ": " << issue.field;
        std::cerr << ": " << issue.message << '\n';
    }

    std::cout << "\nValidated " << files.size() << " entity file(s): "
              << validFiles << " passed, " << issues.size() << " error(s).\n";

    return issues.empty() ? 0 : 1;
}
