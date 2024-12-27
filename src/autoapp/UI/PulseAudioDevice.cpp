//
// Created by Simon Dean on 20/12/2024.
//#include <f1x/openauto/autoapp/UI/PulseAudioDeviceModel.hpp>

PulseAudioDeviceModel::PulseAudioDeviceModel(pa_context* context, pa_direction_t direction, QObject *parent)
: ComboBoxModel(parent),
m_context(std::move(context)),
m_direction(std::move(direction)) {
  ComboBoxModel::populateComboBoxItems();
}

void PulseAudioDeviceModel::populateComboBoxItems() {
  PulseAudioDeviceModel::getPulseAudioDevices();
}

std::vector<std::pair<std::string, std::string>> PulseAudioDeviceModel::getPulseAudioDevices() {
  std::vector<std::pair<std::string, std::string>> devices;

  // Callback for collecting device info
  auto info_callback = [](pa_context *c, const pa_source_info *i, int eol, void *userdata) {
    auto* devices = static_cast<std::vector<std::pair<std::string, std::string>>*>(userdata);

    if (eol > 0) return; // End of list

    if (i) {
      std::string name = i->name ? i->name : "Unknown";
      std::string description = i->description ? i->description : "No description";
      // Here, we use the device's name as a pseudo-hardware address, which isn't ideal but
      // can be used for identification within PulseAudio. For real hardware addresses,
      // you'd need to correlate with system commands like lspci or lsusb.
      devices->emplace_back(name, description);
    }
  };

  pa_operation* op = pa_context_get_source_info_list(m_context, info_callback, &devices);
  while (pa_operation_get_state(op) == PA_OPERATION_RUNNING) {
    pa_mainloop_iterate(pa_context_get_mainloop_api(m_context), 1, NULL);
  }
  pa_operation_unref(op);

  return devices;
}