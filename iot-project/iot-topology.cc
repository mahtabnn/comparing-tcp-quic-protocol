#include "iot-topology.h"
#include "ns3/wifi-net-device.h"
#include <chrono>

using namespace ns3;

IoTTopology::IoTTopology(const TopologyConfig& cfg)
    : m_cfg(cfg) {}

void IoTTopology::Build()
{
    SetupNodes();
    SetupWifi();
    SetupP2P();
    SetupMobility();
    SetupErrorModel();
    ConfigureProtocol();

    InternetStackHelper stack;
    stack.InstallAll();

    Ipv4AddressHelper addr;
    addr.SetBase("10.1.1.0", "255.255.255.0");
    m_iotIfaces = addr.Assign(m_wifiDevices);

    addr.SetBase("10.1.2.0", "255.255.255.0");
    m_p2pIfaces = addr.Assign(m_p2pDevices);

    Ipv4GlobalRoutingHelper::PopulateRoutingTables();

    SetupApplications();
    if (m_cfg.bgTraffic_mbps > 0)
        SetupBackgroundTraffic();

    m_monitor = m_fmHelper.InstallAll();
}

void IoTTopology::Run()
{
    auto wallStart = std::chrono::high_resolution_clock::now();
    Simulator::Stop(Seconds(m_cfg.simDuration));
    Simulator::Run();
    auto wallEnd = std::chrono::high_resolution_clock::now();

    m_connTime_ms = std::chrono::duration<double, std::milli>
                    (wallEnd - wallStart).count();
    Simulator::Destroy();
}

void IoTTopology::SetupNodes()
{
    m_iotDevices.Create(m_cfg.nDevices);
    m_gateway.Create(1);
    m_server.Create(1);
}

void IoTTopology::SetupWifi()
{
    WifiHelper wifi;
    wifi.SetStandard(WIFI_STANDARD_80211g);  // استفاده از استاندارد 802.11

    WifiMacHelper mac;
    YansWifiPhyHelper phy;
    YansWifiChannelHelper channel = YansWifiChannelHelper::Default();
    phy.SetChannel(channel.Create());   // ایجاد کانال رادیویی پیش‌فرض

    Ssid ssid = Ssid("iot-net");  // اسم شبکه وای فای

    mac.SetType("ns3::StaWifiMac",
                "Ssid", SsidValue(ssid),
                "ActiveProbing", BooleanValue(false));
    NetDeviceContainer staDev = wifi.Install(phy, mac, m_iotDevices);



    // تنظیم گیت‌وی به عنوان Access Point
    mac.SetType("ns3::ApWifiMac",
                "Ssid", SsidValue(ssid));
    NetDeviceContainer apDev = wifi.Install(phy, mac, m_gateway);

    m_wifiDevices.Add(staDev);
    m_wifiDevices.Add(apDev);
}

void IoTTopology::SetupP2P()
{
    PointToPointHelper p2p;
    p2p.SetDeviceAttribute("DataRate", StringValue("100Mbps"));   //تعیین پهنای باند
    p2p.SetChannelAttribute("Delay",
        StringValue(std::to_string((int)m_cfg.delayMs) + "ms"));

    NodeContainer gwServer(m_gateway.Get(0), m_server.Get(0));
    m_p2pDevices = p2p.Install(gwServer);      // اتصال گیت‌وی به سرور
}

void IoTTopology::SetupMobility()
{
    MobilityHelper mobility;

    if (m_cfg.enableMobility) {    // حرکت تصادفی در یک مستطیل ۱۰۰ در ۱۰۰ متر با سرعت ۱ تا ۵ متر بر ثانیه
        mobility.SetMobilityModel("ns3::RandomWalk2dMobilityModel",
            "Bounds", RectangleValue(Rectangle(-50, 50, -50, 50)),
            "Speed",  StringValue("ns3::UniformRandomVariable[Min=1|Max=5]"));
        mobility.Install(m_iotDevices);
    } else {
        mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
        mobility.Install(m_iotDevices);
    }
    // سرور و گیت‌وی همیشه ثابت هستند
    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    mobility.Install(m_gateway);
    mobility.Install(m_server);
}

void IoTTopology::SetupErrorModel()
{
    if (m_cfg.lossRate <= 0.0) return;

    // ایجاد مدل خطا بر اساس نرخ گم شدن بسته
    Ptr<RateErrorModel> em = CreateObject<RateErrorModel>();
    em->SetAttribute("ErrorRate", DoubleValue(m_cfg.lossRate));
    em->SetAttribute("ErrorUnit",
        StringValue("ERROR_UNIT_PACKET"));

    // Apply to all WiFi devices
    for (uint32_t i = 0; i < m_wifiDevices.GetN(); i++) {
        Ptr<WifiNetDevice> wifiDev = DynamicCast<WifiNetDevice>(m_wifiDevices.Get(i));
        if (wifiDev) {
            Ptr<WifiPhy> phy = wifiDev->GetPhy();
            phy->SetPostReceptionErrorModel(em); // خطا بعد از دریافت فیزیکی اعمال شود
        }
    }
}
void IoTTopology::ConfigureProtocol()
{
    if (m_cfg.protocol == "TCP") {
        Config::SetDefault("ns3::TcpL4Protocol::SocketType",StringValue("ns3::TcpCubic"));
        Config::SetDefault("ns3::TcpSocket::SegmentSize",UintegerValue(1460));
        Config::SetDefault("ns3::TcpSocket::InitialCwnd",UintegerValue(10));
        Config::SetDefault("ns3::TcpSocketBase::MinRto",TimeValue(MilliSeconds(200)));
    }
    else if (m_cfg.protocol == "QUIC") {
        // شبیه‌سازی QUIC با TCP BBR
        Config::SetDefault("ns3::TcpL4Protocol::SocketType",StringValue("ns3::TcpBbr"));
        Config::SetDefault("ns3::TcpSocket::SegmentSize",UintegerValue(1200));  // UDP-like
        Config::SetDefault("ns3::TcpSocket::InitialCwnd",UintegerValue(10));
        Config::SetDefault("ns3::TcpSocketBase::MinRto",TimeValue(MilliSeconds(10)));  // کوتاه‌تر
    }
}

void IoTTopology::SetupApplications()
{
    // Server: sink on port 9
    uint16_t port = 9;
    Address serverAddr(InetSocketAddress(
        m_p2pIfaces.GetAddress(1), port));
            // ایجاد یک Sink روی سرور برای دریافت داده‌ها
    PacketSinkHelper sink(
        (m_cfg.protocol == "TCP")
            ? "ns3::TcpSocketFactory"
            : "ns3::TcpSocketFactory",   // replace with QUIC when available
        InetSocketAddress(Ipv4Address::GetAny(), port));

    ApplicationContainer sinkApp = sink.Install(m_server.Get(0));
    sinkApp.Start(Seconds(0.0));
    sinkApp.Stop(Seconds(m_cfg.simDuration));

    // IoT devices: bulk send to server
    BulkSendHelper bulkSend(
        (m_cfg.protocol == "TCP")
            ? "ns3::TcpSocketFactory"
            : "ns3::TcpSocketFactory",   // replace with QUIC when available
        serverAddr);
    bulkSend.SetAttribute("MaxBytes", UintegerValue(0));

    for (uint32_t i = 0; i < m_cfg.nDevices; i++) {
        ApplicationContainer app =
        bulkSend.Install(m_iotDevices.Get(i));

        // شروع ارسال با تاخیرهای کوچک (0.1 ثانیه) برای جلوگیری از برخورد شدید اولیه

        app.Start(Seconds(1.0 + i * 0.1));
        app.Stop(Seconds(m_cfg.simDuration - 1.0));
    }
}

void IoTTopology::SetupBackgroundTraffic()
{
    // ایجاد ترافیک مزاحم UDP برای اشغال پهنای باند
    uint16_t bgPort = 8080;
    OnOffHelper onoff("ns3::UdpSocketFactory",
        InetSocketAddress(m_p2pIfaces.GetAddress(1), bgPort));

    std::string rate = std::to_string(m_cfg.bgTraffic_mbps) + "Mbps";
    onoff.SetAttribute("DataRate",  StringValue(rate));
    onoff.SetAttribute("PacketSize", UintegerValue(1024));
    onoff.SetAttribute("OnTime",
        StringValue("ns3::ConstantRandomVariable[Constant=1]"));
    onoff.SetAttribute("OffTime",
        StringValue("ns3::ConstantRandomVariable[Constant=0]"));

   //  این ترافیک فقط از اولین دستگاه IoT ارسال می‌شود
    ApplicationContainer bgApp = onoff.Install(m_iotDevices.Get(0));
    bgApp.Start(Seconds(0.5));
    bgApp.Stop(Seconds(m_cfg.simDuration));
}
