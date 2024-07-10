#pragma once
#include <deque>
#include <string>
#include <unordered_map>
#include <vector>
#include <utility>

#include "geo.h"

struct Stop {
    std::string name_;
    Coordinates coordinates;
};

enum class RouteType {
    Ring, // кольцевой маршрут
    ThereAndback // маршрут туда-обратно
};

struct Bus {
    std::string name_;
    std::vector<Stop*> stops;
    RouteType type;
};
//////////////////////////////////////////
struct StopPairHasher {
    size_t operator()(const std::pair<const Stop*, const Stop*>& p) const {
        return std::hash<const void*>()(p.first) ^ std::hash<const void*>()(p.second);
    }
};
//////////////////////////////////////////
class TransportCatalogue {
public:
    void AddStop(Stop stop);
    void AddBus(Bus bus);
    void SetDistance(const Stop* from, const Stop* to, int distance);
    int GetDistance(const Stop* from, const Stop* to) const;
    Stop* GetStopByName(std::string_view stop_name) const;
    Bus* GetBusByName(std::string_view bus_name) const;
    std::vector<std::string> GetBusesByStop(std::string_view stop_name) const;

private:
    std::deque<Stop> stops_;
    std::deque<Bus> buses_;
    std::unordered_map<std::string_view, Stop*> stopname_to_stop_;
    std::unordered_map<std::string_view, Bus*> busname_to_bus_;
    std::unordered_map<std::pair<const Stop*, const Stop*>, int, StopPairHasher> distances_;
};

class Calculation {
public:
    static size_t GetStopsCount(const Bus* bus);
    static size_t GetUniqueStopsCount(const Bus* bus);
    static double GetRouteLength(const Bus* bus, const TransportCatalogue& catalogue);
    static double GetGeographicRouteLength(const Bus* bus);
};
