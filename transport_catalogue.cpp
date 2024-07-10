#include "transport_catalogue.h"

#include <execution>
#include <unordered_set>

void TransportCatalogue::AddStop(Stop stop) {
    stops_.push_back(stop);
    Stop* ptr_stop = &stops_.back();
    stopname_to_stop_[ptr_stop->name_] = ptr_stop;
}

void TransportCatalogue::AddBus(Bus bus) {
    buses_.push_back(bus);
    Bus* ptr_bus = &buses_.back();
    busname_to_bus_[ptr_bus->name_] = ptr_bus;
}

void TransportCatalogue::SetDistance(const Stop* from, const Stop* to, int distance) {
    distances_[{from, to}] = distance;
}
//////////////////////////////////////////
int TransportCatalogue::GetDistance(const Stop* from, const Stop* to) const {
    auto it = distances_.find({ from, to });
    if (it != distances_.end()) {
        return it->second;
    }
    it = distances_.find({ to, from });
    if (it != distances_.end()) {
        return it->second;
    }
    return 0;
}
//////////////////////////////////////////
Stop* TransportCatalogue::GetStopByName(std::string_view stop_name) const {
    auto iter = stopname_to_stop_.find(stop_name);
    if (iter != stopname_to_stop_.end()) {
        return iter->second;
    }
    return nullptr;
}

Bus* TransportCatalogue::GetBusByName(std::string_view bus_name) const {
    auto iter = busname_to_bus_.find(bus_name);
    if (iter != busname_to_bus_.end()) {
        return iter->second;
    }
    return nullptr;
}

std::vector<std::string> TransportCatalogue::GetBusesByStop(std::string_view stop_name) const {
    std::vector<std::string> buses;
    for (const Bus& bus : buses_) {
        for (const Stop* stop : bus.stops) {
            if (stop->name_ == stop_name) {
                buses.push_back(bus.name_);
                break;
            }
        }
    }
    return buses;
}

size_t Calculation::GetStopsCount(const Bus* bus) {
    return bus->stops.size();
}

size_t Calculation::GetUniqueStopsCount(const Bus* bus) {
    std::unordered_set<Stop*> unique_stops(bus->stops.cbegin(), bus->stops.cend());
    return unique_stops.size();
}
//////////////////////////////////////////
double Calculation::GetRouteLength(const Bus* bus, const TransportCatalogue& catalogue) {
    double route_length = 0;
    for (size_t i = 1; i < bus->stops.size(); ++i) {
        route_length += catalogue.GetDistance(bus->stops[i - 1], bus->stops[i]);
    }
    return route_length;
}
//////////////////////////////////////////
double Calculation::GetGeographicRouteLength(const Bus* bus) {
    double geo_length = 0;
    for (size_t i = 1; i < bus->stops.size(); ++i) {
        geo_length += ComputeDistance(bus->stops[i - 1]->coordinates, bus->stops[i]->coordinates);
    }
    return geo_length;
}
