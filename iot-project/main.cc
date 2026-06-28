#include "ns3/core-module.h"
#include "scenarios.h"
#include <iostream>

using namespace ns3;
using namespace std;

int main(int argc, char* argv[])
{
    // Optional: change via command line  --protocol=TCP  or  --protocol=QUIC
    string protocol = "TCP";

    CommandLine cmd;
    cmd.AddValue("protocol", "Protocol to test: TCP or QUIC", protocol);
    cmd.Parse(argc, argv);

    cout << "\n=============================" << endl;
    cout << " IoT Simulation — " << protocol  << endl;
    cout << "=============================" << endl;

    ScenarioPacketLoss      (protocol);
    ScenarioDelay           (protocol);
    ScenarioMobility        (protocol);
    ScenarioBackgroundTraffic(protocol);
    ScenarioConnectionTime  (protocol);

    cout << "\nAll scenarios done." << endl;

    return 0;
}
