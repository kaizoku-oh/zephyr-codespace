// Lib C
#include <assert.h>

// Zephyr includes
#include <zephyr/net/net_core.h>
#include <zephyr/net/net_context.h>
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(Network);

// User C++ class headers
#include "Network.h"

static void netMgmtCallback(struct net_mgmt_event_callback *cb, uint32_t event, struct net_if *iface);

// Define the static member
Network Network::instance;

Network& Network::getInstance() {
  // Return the singleton instance
  return instance;
}

Network::Network() {
  net_mgmt_init_event_callback(&this->mgmtEventCb, netMgmtCallback, NET_EVENT_IPV4_ADDR_ADD);
  net_mgmt_add_event_callback(&this->mgmtEventCb);
  this->netIface = net_if_get_default();
}

Network::~Network() {
}

void Network::start() {
  if (!this->netIface) {
    LOG_ERR("No default network interface found");
    return;
  }

  net_dhcpv4_start(this->netIface);
}

void Network::onNetworkEvent(std::function<void(NetworkEvent, void *)> callback) {
  assert(callback);

  this->callback = callback;
}

static void netMgmtCallback(struct net_mgmt_event_callback *cb, uint32_t event, struct net_if *iface) {
  char ipBuffer[NET_IPV4_ADDR_LEN] = {0};

  switch (event) {
    case NET_EVENT_IF_DOWN: {
      LOG_WRN("Network interface is down (physical disconnection)");
      if (Network::getInstance().callback) {
        Network::getInstance().callback(NetworkEvent::DOWN, NULL);
      }
      break;
    }
    case NET_EVENT_IF_UP: {
      LOG_INF("Network interface is up (physical connection restored)");
      if (Network::getInstance().callback) {
        Network::getInstance().callback(NetworkEvent::UP, NULL);
      }
      break;
    }
    case NET_EVENT_IPV4_ADDR_ADD: {
      // Handle IP address acquisition
      if (iface->config.ip.ipv4->unicast[0].addr_type == NET_ADDR_DHCP) {
        if (net_addr_ntop(AF_INET,
                          &iface->config.ip.ipv4->unicast[0].address.in_addr,
                          ipBuffer,
                          sizeof(ipBuffer))) {
          if (Network::getInstance().callback) {
            Network::getInstance().callback(NetworkEvent::GOT_IP, (void *)ipBuffer);
          }
        } else {
        LOG_ERR("Error while converting IP address to string form\r\n");
        }
      }
      break;
    }
    case NET_EVENT_IPV4_ADDR_DEL: {
      LOG_WRN("IP address removed");
      break;
    }
    default: {
      break;
    }
  }
}
