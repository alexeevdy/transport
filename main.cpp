#include "request.h"

int main() {
    const auto input = Json::Load(std::cin);
    const auto &input_map = input.GetRoot().AsMap();
    const TransportGuide tg(
            Descriptions::ReadJson(input_map.at("base_requests")),
            Transport::ReadFrom(input_map.at("routing_settings")));
    auto responses = Requests::ProcessAll(tg, input_map.at("stat_requests"));
    Json::Print(std::cout, responses);
    return 0;
}
