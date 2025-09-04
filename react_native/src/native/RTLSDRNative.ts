import {NativeModules} from 'react-native';

interface RTLSDRInterface {
  connectDevice(): Promise<boolean>;
  disconnectDevice(): Promise<void>;
  setFrequency(frequency: number): Promise<boolean>;
  setGain(gain: number): Promise<boolean>;
  setAutoGain(enable: boolean): Promise<boolean>;
  setSampleRate(rate: number): Promise<boolean>;
  startReading(): Promise<boolean>;
  stopReading(): Promise<void>;
  getSpectrumData(): Promise<number[]>;
  getAudioData(): Promise<number[]>;
}

// Para desenvolvimento/teste - implementação mock
const RTLSDRMock: RTLSDRInterface = {
  async connectDevice(): Promise<boolean> {
    console.log('RTLSDRMock: connectDevice called');
    // Simula sucesso de conexão após 1 segundo
    return new Promise((resolve) => {
      setTimeout(() => resolve(true), 1000);
    });
  },

  async disconnectDevice(): Promise<void> {
    console.log('RTLSDRMock: disconnectDevice called');
    return Promise.resolve();
  },

  async setFrequency(frequency: number): Promise<boolean> {
    console.log(`RTLSDRMock: setFrequency called with ${frequency} MHz`);
    return Promise.resolve(true);
  },

  async setGain(gain: number): Promise<boolean> {
    console.log(`RTLSDRMock: setGain called with ${gain}`);
    return Promise.resolve(true);
  },

  async setAutoGain(enable: boolean): Promise<boolean> {
    console.log(`RTLSDRMock: setAutoGain called with ${enable}`);
    return Promise.resolve(true);
  },

  async setSampleRate(rate: number): Promise<boolean> {
    console.log(`RTLSDRMock: setSampleRate called with ${rate}`);
    return Promise.resolve(true);
  },

  async startReading(): Promise<boolean> {
    console.log('RTLSDRMock: startReading called');
    return Promise.resolve(true);
  },

  async stopReading(): Promise<void> {
    console.log('RTLSDRMock: stopReading called');
    return Promise.resolve();
  },

  async getSpectrumData(): Promise<number[]> {
    // Gera dados de espectro simulados
    const data = [];
    for (let i = 0; i < 50; i++) {
      data.push(Math.random() * 100 - 50); // Valores entre -50 e 50
    }
    return Promise.resolve(data);
  },

  async getAudioData(): Promise<number[]> {
    // Gera dados de áudio simulados
    const data = [];
    for (let i = 0; i < 1024; i++) {
      data.push(Math.sin(i * 0.1) * 32767); // Onda senoidal
    }
    return Promise.resolve(data);
  },
};

// Quando o módulo nativo estiver implementado, use esta linha:
// const RTLSDRNative = NativeModules.RTLSDRModule as RTLSDRInterface;

// Por enquanto, use o mock para desenvolvimento:
const RTLSDRNative = RTLSDRMock;

export {RTLSDRNative};
