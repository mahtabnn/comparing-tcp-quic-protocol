#ifndef METRICS_H
#define METRICS_H

#include "ns3/flow-monitor-helper.h"
#include "ns3/ipv4-flow-classifier.h"
#include "ns3/core-module.h"
#include <string>

using namespace ns3;

struct SimResult {
    std::string protocol;
    std::string scenario;
    double paramValue;
    double throughput_mbps;
    double lossRate_pct;
    double avgDelay_ms;
    double jitter_ms;
    double connTime_ms;
};

void CollectMetrics(
    FlowMonitorHelper& fmHelper,
    Ptr<FlowMonitor> monitor,
    SimResult& result
);

void WriteResultToCSV(
    const SimResult& result,
    const std::string& filepath
);

#endif
