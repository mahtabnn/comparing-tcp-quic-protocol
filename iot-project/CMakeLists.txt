add_executable(iot-project
  main.cc
  iot-topology.cc
  scenarios.cc
  metrics.cc
)

target_link_libraries(iot-project
  ${libcore}
  ${libnetwork}
  ${libinternet}
  ${libwifi}
  ${libpoint-to-point}
  ${libapplications}
  ${libflow-monitor}
  ${libmobility}
  ${libcsma}
)
