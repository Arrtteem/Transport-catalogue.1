#include "input_reader.h"

#include <algorithm>
#include <cassert>
#include <iterator>
#include <utility>

Coordinates ParseCoordinates(std::string_view str) {
    auto not_space = str.find_first_not_of(' ');
    auto comma = str.find(',');

    if (comma == str.npos) {
        return { std::nan(""), std::nan("") };
    }

    auto not_space2 = str.find_first_not_of(' ', comma + 1);
    double lat = std::stod(std::string(str.substr(not_space, comma - not_space)));
    double lng = std::stod(std::string(str.substr(not_space2)));

    return { lat, lng };
}

std::string_view Trim(std::string_view string) {
    const auto start = string.find_first_not_of(' ');
    if (start == string.npos) {
        return {};
    }
    return string.substr(start, string.find_last_not_of(' ') + 1 - start);
}

std::vector<std::string_view> Split(std::string_view string, char delim) {
    std::vector<std::string_view> result;

    size_t pos = 0;
    while ((pos = string.find_first_not_of(' ', pos)) < string.length()) {
        auto delim_pos = string.find(delim, pos);
        if (delim_pos == string.npos) {
            delim_pos = string.size();
        }
        if (auto substr = Trim(string.substr(pos, delim_pos - pos)); !substr.empty()) {
            result.push_back(substr);
        }
        pos = delim_pos + 1;
    }

    return result;
}

std::pair<RouteType, std::vector<std::string_view>> ParseRoute(std::string_view route) {
    if (route.find('>') != route.npos) {
        return { RouteType::Ring, Split(route, '>') };
    }

    auto stops = Split(route, '-');
    std::vector<std::string_view> results(stops.begin(), stops.end());
    results.insert(results.end(), std::next(stops.rbegin()), stops.rend());

    return { RouteType::ThereAndback, results };
}

detail::CommandDescription ParseCommandDescription(std::string_view line) {
    auto colon_pos = line.find(':');
    if (colon_pos == line.npos) {
        return {};
    }

    auto space_pos = line.find(' ');
    if (space_pos >= colon_pos) {
        return {};
    }

    auto not_space = line.find_first_not_of(' ', space_pos);
    if (not_space >= colon_pos) {
        return {};
    }

    return { std::string(line.substr(0, space_pos)),
            std::string(line.substr(not_space, colon_pos - not_space)),
            std::string(line.substr(colon_pos + 1)) };
}

void InputReader::ParseLine(std::string_view line) {
    auto command_description = ParseCommandDescription(line);
    if (command_description) {
        commands_.push_back(std::move(command_description));
    }
}

void InputReader::ApplyCommands(TransportCatalogue& catalogue) const {
    std::vector<std::pair<std::string, std::vector<std::string_view>>> buses_with_stops;
    std::vector<detail::CommandDescription> stops_with_distances;

    for (const detail::CommandDescription& command : commands_) {
        if (command.command == "Stop") {
            Stop stop{ command.id, ParseCoordinates(command.description) };
            catalogue.AddStop(std::move(stop));
            stops_with_distances.push_back(command);
        }
        else if (command.command == "Bus") {
            auto [type, parsed_stops] = ParseRoute(command.description);
            buses_with_stops.emplace_back(command.id, parsed_stops);
        }
    }

    for (const auto& [bus_name, parsed_stops] : buses_with_stops) {
        std::vector<Stop*> stops;
        for (const auto& stop_name : parsed_stops) {
            stops.push_back(catalogue.GetStopByName(stop_name));
        }
        RouteType type = bus_name.find('>') != std::string::npos ? RouteType::Ring : RouteType::ThereAndback;
        Bus bus{ bus_name, stops, type };
        catalogue.AddBus(std::move(bus));
    }

    for (const detail::CommandDescription& stop_command : stops_with_distances) {
        auto stop = catalogue.GetStopByName(stop_command.id);
        auto pos = stop_command.description.find(',');
        pos = stop_command.description.find(',', pos + 1);

        while (true) {
            auto next_pos = stop_command.description.find('m', pos + 1);
            if (next_pos == std::string::npos) {
                break;
            }

            int distance = std::stoi(std::string(stop_command.description.substr(pos + 1, next_pos - pos - 1)));
            auto to_stop_name_pos = stop_command.description.find("to", next_pos);
            if (to_stop_name_pos == std::string::npos) {
                break;
            }

            auto to_stop_name_end = stop_command.description.find(',', to_stop_name_pos);
            auto to_stop_name = Trim(stop_command.description.substr(to_stop_name_pos + 2, to_stop_name_end - to_stop_name_pos - 2));
            auto to_stop = catalogue.GetStopByName(to_stop_name);
            catalogue.SetDistance(stop, to_stop, distance);

            if (to_stop_name_end == std::string::npos) {
                break;
            }
            pos = to_stop_name_end;
        }
    }
}
