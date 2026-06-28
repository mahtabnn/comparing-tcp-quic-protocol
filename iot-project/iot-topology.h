#ifndef IOT_TOPOLOGY_H
#define IOT_TOPOLOGY_H

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/wifi-module.h"
#include "ns3/mobility-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/flow-monitor-helper.h"
using namespace ns3;

struct TopologyConfig {
    uint32_t    nDevices        = 10;
    std::string protocol        = "TCP";   // "TCP" or "QUIC"
    double      lossRate        = 0.0;     // 0.0 to 1.0
    double      delayMs         = 10.0;    // ms
    bool        enableMobility  = false;
    double      bgTraffic_mbps  = 0.0;
    double      simDuration     = 20.0;    // seconds
};

class IoTTopology {
public:
    IoTTopology(const TopologyConfig& cfg);

    void Build();
    void Run();

    Ptr<FlowMonitor>    GetMonitor()   { return m_monitor; }
    FlowMonitorHelper&  GetFmHelper()  { return m_fmHelper; }
    double              GetConnTime()  { return m_connTime_ms; }

private:
    TopologyConfig    m_cfg;           //تنظیمات شبیه‌سازی
    NodeContainer     m_iotDevices;    //لیست دستگاه‌های IoT
    NodeContainer     m_gateway;       //دستگاه مرکزی شبکه
    NodeContainer     m_server;        //سروری که داده‌ها به آن ارسال می‌شود
    NetDeviceContainer m_wifiDevices;  //device های WiFi روی IoT.
    NetDeviceContainer m_p2pDevices;   //لینک gateway به server.


    Ipv4InterfaceContainer m_iotIfaces;  //IP address های دستگاه‌های IoT.
    Ipv4InterfaceContainer m_p2pIfaces;  //IP های لینک gateway-server.



    FlowMonitorHelper m_fmHelper;
    Ptr<FlowMonitor>  m_monitor;
    double            m_connTime_ms = 0.0;  //زمان handshake یا connection setup.

    void SetupNodes();
    void SetupWifi();
    void SetupP2P();
    void SetupMobility();
    void SetupErrorModel();
    void ConfigureProtocol();
    void SetupApplications();
    void SetupBackgroundTraffic();
};

#endif
