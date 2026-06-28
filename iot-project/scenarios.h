#ifndef SCENARIOS_H
#define SCENARIOS_H

#include <string>

void ScenarioPacketLoss     (const std::string& protocol);
void ScenarioDelay          (const std::string& protocol);
void ScenarioMobility       (const std::string& protocol);
void ScenarioBackgroundTraffic(const std::string& protocol);
void ScenarioConnectionTime (const std::string& protocol);

#endif
