
#include "scenarios.h"
#include "iot-topology.h"
#include "metrics.h"
#include <vector>
#include <iostream>

// ─── Packet Loss ────────────────────────────────────────────────
void ScenarioPacketLoss(const std::string& protocol)
{
    std::vector<double> lossRates = {0.0, 0.01, 0.05, 0.10, 0.20};
    std::string csvPath = "results/iot-project/raw/"
                        + protocol + "_packet_loss.csv";

    for (double lr : lossRates) {
        std::cout << "[PacketLoss] protocol=" << protocol
                  << " loss=" << lr * 100 << "%" << std::endl;

        TopologyConfig cfg;
        cfg.protocol  = protocol;
        cfg.lossRate  = lr;
        cfg.simDuration = 20.0;

        IoTTopology topo(cfg);
        topo.Build();
        topo.Run();

        SimResult result;
        result.protocol   = protocol;
        result.scenario   = "packet_loss";
        result.paramValue = lr * 100.0;
        result.connTime_ms = topo.GetConnTime();

        CollectMetrics(topo.GetFmHelper(), topo.GetMonitor(), result);
        WriteResultToCSV(result, csvPath);
    }
}

// ─── Delay ──────────────────────────────────────────────────────
void ScenarioDelay(const std::string& protocol)
{
    std::vector<double> delays = {1.0, 10.0, 50.0, 100.0, 200.0};
    std::string csvPath = "results/iot-project/raw/"
                        + protocol + "_delay.csv";

    for (double d : delays) {
        std::cout << "[Delay] protocol=" << protocol
                  << " delay=" << d << "ms" << std::endl;

        TopologyConfig cfg;
        cfg.protocol  = protocol;
        cfg.delayMs   = d;
        cfg.simDuration = 20.0;

        IoTTopology topo(cfg);
        topo.Build();
        topo.Run();

        SimResult result;
        result.protocol   = protocol;
        result.scenario   = "delay";
        result.paramValue = d;
        result.connTime_ms = topo.GetConnTime();

        CollectMetrics(topo.GetFmHelper(), topo.GetMonitor(), result);
        WriteResultToCSV(result, csvPath);
    }
}

// ─── Mobility ───────────────────────────────────────────────────
void ScenarioMobility(const std::string& protocol)
{
    std::string csvPath = "results/iot-project/raw/"
                        + protocol + "_mobility.csv";

    for (int mobile = 0; mobile <= 1; mobile++) {
        std::cout << "[Mobility] protocol=" << protocol
                  << " mobile=" << mobile << std::endl;

        TopologyConfig cfg;
        cfg.protocol       = protocol;
        cfg.enableMobility = (mobile == 1);
        cfg.simDuration    = 30.0;

        IoTTopology topo(cfg);
        topo.Build();
        topo.Run();

        SimResult result;
        result.protocol   = protocol;
        result.scenario   = "mobility";
        result.paramValue = mobile;
        result.connTime_ms = topo.GetConnTime();

        CollectMetrics(topo.GetFmHelper(), topo.GetMonitor(), result);
        WriteResultToCSV(result, csvPath);
    }
}

// ─── Background Traffic ─────────────────────────────────────────
void ScenarioBackgroundTraffic(const std::string& protocol)
{
    std::vector<double> bgRates = {0.0, 1.0, 5.0, 10.0, 20.0};
    std::string csvPath = "results/iot-project/raw/"
                        + protocol + "_bg_traffic.csv";

    for (double bg : bgRates) {
        std::cout << "[BgTraffic] protocol=" << protocol
                  << " bg=" << bg << "Mbps" << std::endl;

        TopologyConfig cfg;
        cfg.protocol        = protocol;
        cfg.bgTraffic_mbps  = bg;
        cfg.simDuration     = 20.0;

        IoTTopology topo(cfg);
        topo.Build();
        topo.Run();

        SimResult result;
        result.protocol   = protocol;
        result.scenario   = "bg_traffic";
        result.paramValue = bg;
        result.connTime_ms = topo.GetConnTime();

        CollectMetrics(topo.GetFmHelper(), topo.GetMonitor(), result);
        WriteResultToCSV(result, csvPath);
    }
}

// ─── Connection Time ────────────────────────────────────────────
void ScenarioConnectionTime(const std::string& protocol)
{
    std::string csvPath = "results/iot-project/raw/"
                        + protocol + "_conn_time.csv";

    for (int run = 1; run <= 10; run++) {
        std::cout << "[ConnTime] protocol=" << protocol
                  << " run=" << run << std::endl;

        TopologyConfig cfg;
        cfg.protocol    = protocol;
        cfg.simDuration = 10.0;

        IoTTopology topo(cfg);
        topo.Build();
        topo.Run();

        SimResult result;
        result.protocol   = protocol;
        result.scenario   = "conn_time";
        result.paramValue = run;
        result.connTime_ms = topo.GetConnTime();

        CollectMetrics(topo.GetFmHelper(), topo.GetMonitor(), result);
        WriteResultToCSV(result, csvPath);
    }
}
