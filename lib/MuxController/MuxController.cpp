#include "MuxController.h"

MuxController::MuxController() {
  mux[0] = new Driver74HCT4067(12, 13, 14, 15, 16, 36, false);
  mux[1] = new Driver74HCT4067(12, 13, 14, 15, 17, 36, false);

  activeMux = 0;
}

void MuxController::readNext() {
  mux[activeMux]->readNext();  // lit le canal courant du mux actif

  /** Avancer dans le canal
  if (mux[activeMux]->getCurrentIndex() == 0) {
    // Si le mux vient de repasser à 0 → tous ses 16 canaux ont été lus
    activeMux = (activeMux + 1) % 2; // on passe au mux suivant
  }*/
}
uint16_t MuxController::get(uint8_t muxIndex, uint8_t channelIndex) {
  if (muxIndex >= 2 || channelIndex >= 16) return 0.0;
  return mux[muxIndex]->get(channelIndex);
}
