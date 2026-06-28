#include "metrics.h"
#include <fstream>
#include <iostream>
#include <sys/stat.h>

using namespace ns3;

  //آمار FlowMonitor می‌گیرد و در struct ذخیره می‌کند
void CollectMetrics(FlowMonitorHelper& fmHelper,
                    Ptr<FlowMonitor> monitor,
                    SimResult& result)
{
//پکت‌های از دست رفته را محاسبه می کند
    monitor->CheckForLostPackets();
    Ptr<Ipv4FlowClassifier> classifier =              // تشخیص جریان‌ داده ها
        DynamicCast<Ipv4FlowClassifier>(fmHelper.GetClassifier());

    auto stats = monitor->GetFlowStats();    //گرفتن آمار flow ها

    double totalThroughput = 0;
    double totalDelay      = 0;
    double totalJitter     = 0;
    uint64_t totalRx       = 0;  //متغیرهای جمع‌آوری متریک
    uint64_t totalTx       = 0;
    uint64_t totalLost     = 0;
    int flowCount          = 0;

    for (auto& flow : stats) {
        if (flow.second.rxPackets == 0) continue;         //اگر هیچ پکت دریافتی ندارد، آن را نادیده می‌گیرد.

        double duration = flow.second.timeLastRxPacket.GetSeconds()     // محاسبه مدت جریان
                        - flow.second.timeFirstTxPacket.GetSeconds();
        if (duration <= 0) continue;

        totalThroughput += (flow.second.rxBytes * 8.0) / duration / 1e6;    //محاسبه Throughput
        totalDelay      += flow.second.delaySum.GetMilliSeconds()                //محاسبه delay
                           / flow.second.rxPackets;
        totalJitter     += flow.second.jitterSum.GetMilliSeconds()
                           / flow.second.rxPackets;
        totalRx         += flow.second.rxPackets;
        totalTx         += flow.second.txPackets;                //جمع بسته ها
        totalLost       += flow.second.lostPackets;              //شمارش flow
        flowCount++;
    }

    if (flowCount > 0) {
        result.throughput_mbps = totalThroughput / flowCount;
        result.avgDelay_ms     = totalDelay      / flowCount;         // محاسبه میانگین متریک‌ها
        result.jitter_ms       = totalJitter     / flowCount;
    }

    result.lossRate_pct = (totalTx > 0)
        ? (double)totalLost / totalTx * 100.0                         // محاسبه Packet Loss Rate
        : 0.0;
}

void WriteResultToCSV(const SimResult& result,
                      const std::string& filepath)           //csv
{
    // Check if file exists to write header
    struct stat buffer;
    bool fileExists = (stat(filepath.c_str(), &buffer) == 0);

    std::ofstream out(filepath, std::ios::app);
    if (!out.is_open()) {
        std::cerr << "Cannot open file: " << filepath << std::endl;
        return;
    }

    if (!fileExists) {
        out << "protocol,scenario,param_value,"
            << "throughput_mbps,loss_rate_pct,"
            << "avg_delay_ms,jitter_ms,conn_time_ms\n";
    }

    out << result.protocol    << ","
        << result.scenario    << ","
        << result.paramValue  << ","
        << result.throughput_mbps << ","
        << result.lossRate_pct    << ","
        << result.avgDelay_ms     << ","
        << result.jitter_ms       << ","
        << result.connTime_ms     << "\n";

    out.close();
}
