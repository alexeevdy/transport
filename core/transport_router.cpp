#include "transport_router.h"

void TransportRouter::ResizeToFit(size_t column, size_t row) {
    if (column >= distance_table.size()) {
        distance_table.resize(column + 1);
    }
    if (row >= distance_table[column].size()) {
        distance_table[column].resize(row + 1);
    }
}

void TransportRouter::UpdateDistance(const std::string &from, const std::string &to, int distance) {
    size_t from_id = stops_.SetId(from);
    size_t to_id = stops_.SetId(to);
    ResizeToFit(from_id, to_id);
    distance_table[from_id][to_id] = distance;
    ResizeToFit(to_id, from_id);
    if (!distance_table[to_id][from_id]) {
        distance_table[to_id][from_id] = distance;
    }
}

int TransportRouter::GetDistance(const std::string &from, const std::string &to) const {
    size_t from_id = stops_.GetId(from);
    size_t to_id = stops_.GetId(to);
    return distance_table[from_id][to_id].value();
}
