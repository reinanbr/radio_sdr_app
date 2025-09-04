import React, {useState, useEffect} from 'react';
import {
  View,
  Text,
  StyleSheet,
  ScrollView,
  TouchableOpacity,
  Switch,
  TextInput,
  Alert,
} from 'react-native';
import Slider from '@react-native-community/slider';
import AsyncStorage from '@react-native-async-storage/async-storage';

const SettingsScreen: React.FC = () => {
  // Settings state
  const [sampleRate, setSampleRate] = useState(2048000);
  const [frequencyCorrection, setFrequencyCorrection] = useState(0);
  const [enableBiasT, setEnableBiasT] = useState(false);
  const [enableDirectSampling, setEnableDirectSampling] = useState(false);
  const [tunerGainMode, setTunerGainMode] = useState(true); // true = auto, false = manual
  const [audioSampleRate, setAudioSampleRate] = useState(48000);
  const [audioBufferSize, setAudioBufferSize] = useState(4096);
  const [enableAGC, setEnableAGC] = useState(true);
  const [enableNotifications, setEnableNotifications] = useState(true);

  // Sample rate options
  const sampleRateOptions = [
    {label: '250 kHz', value: 250000},
    {label: '1.024 MHz', value: 1024000},
    {label: '1.536 MHz', value: 1536000},
    {label: '1.792 MHz', value: 1792000},
    {label: '1.920 MHz', value: 1920000},
    {label: '2.048 MHz', value: 2048000},
    {label: '2.160 MHz', value: 2160000},
    {label: '2.560 MHz', value: 2560000},
    {label: '2.880 MHz', value: 2880000},
    {label: '3.200 MHz', value: 3200000},
  ];

  useEffect(() => {
    loadSettings();
  }, []);

  const loadSettings = async () => {
    try {
      const savedSettings = await AsyncStorage.getItem('rtlsdr_settings');
      if (savedSettings) {
        const settings = JSON.parse(savedSettings);
        setSampleRate(settings.sampleRate || 2048000);
        setFrequencyCorrection(settings.frequencyCorrection || 0);
        setEnableBiasT(settings.enableBiasT || false);
        setEnableDirectSampling(settings.enableDirectSampling || false);
        setTunerGainMode(settings.tunerGainMode !== false);
        setAudioSampleRate(settings.audioSampleRate || 48000);
        setAudioBufferSize(settings.audioBufferSize || 4096);
        setEnableAGC(settings.enableAGC !== false);
        setEnableNotifications(settings.enableNotifications !== false);
      }
    } catch (error) {
      console.error('Erro ao carregar configurações:', error);
    }
  };

  const saveSettings = async () => {
    try {
      const settings = {
        sampleRate,
        frequencyCorrection,
        enableBiasT,
        enableDirectSampling,
        tunerGainMode,
        audioSampleRate,
        audioBufferSize,
        enableAGC,
        enableNotifications,
      };
      await AsyncStorage.setItem('rtlsdr_settings', JSON.stringify(settings));
      Alert.alert('Sucesso', 'Configurações salvas com sucesso!');
    } catch (error) {
      console.error('Erro ao salvar configurações:', error);
      Alert.alert('Erro', 'Erro ao salvar configurações');
    }
  };

  const resetSettings = () => {
    Alert.alert(
      'Restaurar Padrões',
      'Tem certeza que deseja restaurar todas as configurações para os valores padrão?',
      [
        {text: 'Cancelar', style: 'cancel'},
        {
          text: 'Restaurar',
          style: 'destructive',
          onPress: () => {
            setSampleRate(2048000);
            setFrequencyCorrection(0);
            setEnableBiasT(false);
            setEnableDirectSampling(false);
            setTunerGainMode(true);
            setAudioSampleRate(48000);
            setAudioBufferSize(4096);
            setEnableAGC(true);
            setEnableNotifications(true);
          },
        },
      ]
    );
  };

  const formatSampleRate = (rate: number): string => {
    if (rate >= 1000000) {
      return `${(rate / 1000000).toFixed(3)} MHz`;
    } else {
      return `${(rate / 1000).toFixed(0)} kHz`;
    }
  };

  return (
    <ScrollView style={styles.container}>
      <View style={styles.section}>
        <Text style={styles.sectionTitle}>Configurações do RTL-SDR</Text>

        <View style={styles.settingItem}>
          <Text style={styles.settingLabel}>Taxa de Amostragem</Text>
          <Text style={styles.settingValue}>{formatSampleRate(sampleRate)}</Text>
          <View style={styles.sampleRateButtons}>
            {sampleRateOptions.map((option) => (
              <TouchableOpacity
                key={option.value}
                style={[
                  styles.sampleRateButton,
                  sampleRate === option.value && styles.sampleRateButtonActive,
                ]}
                onPress={() => setSampleRate(option.value)}>
                <Text
                  style={[
                    styles.sampleRateButtonText,
                    sampleRate === option.value && styles.sampleRateButtonTextActive,
                  ]}>
                  {option.label}
                </Text>
              </TouchableOpacity>
            ))}
          </View>
        </View>

        <View style={styles.settingItem}>
          <Text style={styles.settingLabel}>Correção de Frequência (PPM)</Text>
          <TextInput
            style={styles.textInput}
            value={frequencyCorrection.toString()}
            onChangeText={(text) => setFrequencyCorrection(parseInt(text) || 0)}
            keyboardType="numeric"
            placeholder="0"
          />
          <Text style={styles.settingDescription}>
            Ajuste fino da frequência do cristal oscilador (-1000 a +1000 PPM)
          </Text>
        </View>

        <View style={styles.settingItem}>
          <View style={styles.switchItem}>
            <Text style={styles.settingLabel}>Bias-T (Alimentação de Antena)</Text>
            <Switch
              value={enableBiasT}
              onValueChange={setEnableBiasT}
              trackColor={{false: '#CCCCCC', true: '#1976D2'}}
              thumbColor={enableBiasT ? '#FFFFFF' : '#FFFFFF'}
            />
          </View>
          <Text style={styles.settingDescription}>
            Habilita alimentação para amplificadores LNA externos
          </Text>
        </View>

        <View style={styles.settingItem}>
          <View style={styles.switchItem}>
            <Text style={styles.settingLabel}>Amostragem Direta</Text>
            <Switch
              value={enableDirectSampling}
              onValueChange={setEnableDirectSampling}
              trackColor={{false: '#CCCCCC', true: '#1976D2'}}
              thumbColor={enableDirectSampling ? '#FFFFFF' : '#FFFFFF'}
            />
          </View>
          <Text style={styles.settingDescription}>
            Para recepção de frequências HF (abaixo de 28 MHz)
          </Text>
        </View>

        <View style={styles.settingItem}>
          <View style={styles.switchItem}>
            <Text style={styles.settingLabel}>Ganho Automático do Tuner</Text>
            <Switch
              value={tunerGainMode}
              onValueChange={setTunerGainMode}
              trackColor={{false: '#CCCCCC', true: '#1976D2'}}
              thumbColor={tunerGainMode ? '#FFFFFF' : '#FFFFFF'}
            />
          </View>
          <Text style={styles.settingDescription}>
            Controle automático de ganho do tuner RTL2832U
          </Text>
        </View>
      </View>

      <View style={styles.section}>
        <Text style={styles.sectionTitle}>Configurações de Áudio</Text>

        <View style={styles.settingItem}>
          <Text style={styles.settingLabel}>Taxa de Amostragem de Áudio</Text>
          <Text style={styles.settingValue}>{audioSampleRate} Hz</Text>
          <Slider
            style={styles.slider}
            minimumValue={8000}
            maximumValue={48000}
            step={8000}
            value={audioSampleRate}
            onValueChange={setAudioSampleRate}
            minimumTrackTintColor="#1976D2"
            maximumTrackTintColor="#CCCCCC"
          />
        </View>

        <View style={styles.settingItem}>
          <Text style={styles.settingLabel}>Tamanho do Buffer de Áudio</Text>
          <Text style={styles.settingValue}>{audioBufferSize} samples</Text>
          <Slider
            style={styles.slider}
            minimumValue={1024}
            maximumValue={8192}
            step={1024}
            value={audioBufferSize}
            onValueChange={setAudioBufferSize}
            minimumTrackTintColor="#1976D2"
            maximumTrackTintColor="#CCCCCC"
          />
        </View>

        <View style={styles.settingItem}>
          <View style={styles.switchItem}>
            <Text style={styles.settingLabel}>AGC de Áudio</Text>
            <Switch
              value={enableAGC}
              onValueChange={setEnableAGC}
              trackColor={{false: '#CCCCCC', true: '#1976D2'}}
              thumbColor={enableAGC ? '#FFFFFF' : '#FFFFFF'}
            />
          </View>
          <Text style={styles.settingDescription}>
            Controle automático de ganho do áudio demodulado
          </Text>
        </View>
      </View>

      <View style={styles.section}>
        <Text style={styles.sectionTitle}>Configurações do App</Text>

        <View style={styles.settingItem}>
          <View style={styles.switchItem}>
            <Text style={styles.settingLabel}>Notificações</Text>
            <Switch
              value={enableNotifications}
              onValueChange={setEnableNotifications}
              trackColor={{false: '#CCCCCC', true: '#1976D2'}}
              thumbColor={enableNotifications ? '#FFFFFF' : '#FFFFFF'}
            />
          </View>
          <Text style={styles.settingDescription}>
            Receber notificações sobre status de conexão e gravações
          </Text>
        </View>
      </View>

      <View style={styles.buttonSection}>
        <TouchableOpacity style={styles.saveButton} onPress={saveSettings}>
          <Text style={styles.buttonText}>Salvar Configurações</Text>
        </TouchableOpacity>

        <TouchableOpacity style={styles.resetButton} onPress={resetSettings}>
          <Text style={styles.buttonText}>Restaurar Padrões</Text>
        </TouchableOpacity>
      </View>

      <View style={styles.infoSection}>
        <Text style={styles.infoTitle}>Sobre o App</Text>
        <Text style={styles.infoText}>Radio SDR v1.0.0</Text>
        <Text style={styles.infoText}>
          App para controle de dongles RTL-SDR baseado em RTL2832U
        </Text>
        <Text style={styles.infoText}>
          Suporta frequências de 24 MHz a 1766 MHz
        </Text>
      </View>
    </ScrollView>
  );
};

const styles = StyleSheet.create({
  container: {
    flex: 1,
    backgroundColor: '#F5F5F5',
  },
  section: {
    backgroundColor: '#FFFFFF',
    marginVertical: 8,
    marginHorizontal: 16,
    borderRadius: 12,
    padding: 16,
    shadowColor: '#000',
    shadowOffset: {
      width: 0,
      height: 2,
    },
    shadowOpacity: 0.1,
    shadowRadius: 4,
    elevation: 3,
  },
  sectionTitle: {
    fontSize: 20,
    fontWeight: 'bold',
    color: '#333333',
    marginBottom: 16,
    borderBottomWidth: 1,
    borderBottomColor: '#E0E0E0',
    paddingBottom: 8,
  },
  settingItem: {
    marginBottom: 20,
  },
  settingLabel: {
    fontSize: 16,
    fontWeight: '600',
    color: '#333333',
    marginBottom: 8,
  },
  settingValue: {
    fontSize: 14,
    color: '#666666',
    marginBottom: 8,
  },
  settingDescription: {
    fontSize: 12,
    color: '#999999',
    marginTop: 4,
    fontStyle: 'italic',
  },
  switchItem: {
    flexDirection: 'row',
    justifyContent: 'space-between',
    alignItems: 'center',
  },
  textInput: {
    borderWidth: 1,
    borderColor: '#CCCCCC',
    borderRadius: 8,
    padding: 12,
    fontSize: 16,
    backgroundColor: '#FAFAFA',
  },
  slider: {
    width: '100%',
    height: 40,
    marginTop: 8,
  },
  sampleRateButtons: {
    flexDirection: 'row',
    flexWrap: 'wrap',
    marginTop: 8,
  },
  sampleRateButton: {
    backgroundColor: '#F0F0F0',
    paddingHorizontal: 12,
    paddingVertical: 6,
    borderRadius: 16,
    margin: 4,
    borderWidth: 1,
    borderColor: '#CCCCCC',
  },
  sampleRateButtonActive: {
    backgroundColor: '#1976D2',
    borderColor: '#1976D2',
  },
  sampleRateButtonText: {
    fontSize: 12,
    color: '#666666',
    fontWeight: '500',
  },
  sampleRateButtonTextActive: {
    color: '#FFFFFF',
  },
  buttonSection: {
    marginHorizontal: 16,
    marginVertical: 16,
  },
  saveButton: {
    backgroundColor: '#4CAF50',
    paddingVertical: 12,
    paddingHorizontal: 24,
    borderRadius: 8,
    alignItems: 'center',
    marginBottom: 12,
  },
  resetButton: {
    backgroundColor: '#FF9800',
    paddingVertical: 12,
    paddingHorizontal: 24,
    borderRadius: 8,
    alignItems: 'center',
  },
  buttonText: {
    color: '#FFFFFF',
    fontSize: 16,
    fontWeight: 'bold',
  },
  infoSection: {
    backgroundColor: '#FFFFFF',
    marginHorizontal: 16,
    marginBottom: 20,
    borderRadius: 12,
    padding: 16,
    shadowColor: '#000',
    shadowOffset: {
      width: 0,
      height: 2,
    },
    shadowOpacity: 0.1,
    shadowRadius: 4,
    elevation: 3,
  },
  infoTitle: {
    fontSize: 18,
    fontWeight: 'bold',
    color: '#333333',
    marginBottom: 12,
  },
  infoText: {
    fontSize: 14,
    color: '#666666',
    lineHeight: 20,
    marginBottom: 4,
  },
});

export default SettingsScreen;
