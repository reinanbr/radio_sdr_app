import React, {useState, useEffect} from 'react';
import {
  View,
  Text,
  StyleSheet,
  TouchableOpacity,
  TextInput,
  Switch,
  ScrollView,
  Alert,
  PermissionsAndroid,
  Platform,
} from 'react-native';
import Slider from '@react-native-community/slider';
import {LineChart} from 'react-native-chart-kit';
import {Dimensions} from 'react-native';
import {StackNavigationProp} from '@react-navigation/stack';

import {RTLSDRNative} from '../native/RTLSDRNative';

type RootStackParamList = {
  Main: undefined;
  Spectrum: undefined;
  Settings: undefined;
};

type MainScreenNavigationProp = StackNavigationProp<RootStackParamList, 'Main'>;

interface Props {
  navigation: MainScreenNavigationProp;
}

const MainScreen: React.FC<Props> = ({navigation}) => {
  // State variables
  const [isConnected, setIsConnected] = useState(false);
  const [isRunning, setIsRunning] = useState(false);
  const [isRecording, setIsRecording] = useState(false);
  const [isMuted, setIsMuted] = useState(false);
  const [frequency, setFrequency] = useState('88.5');
  const [gain, setGain] = useState(248);
  const [volume, setVolume] = useState(50);
  const [autoGain, setAutoGain] = useState(true);
  const [connectionStatus, setConnectionStatus] = useState('Dispositivo Desconectado');
  const [spectrumData, setSpectrumData] = useState<number[]>([]);

  // Screen dimensions for chart
  const screenData = Dimensions.get('window');

  useEffect(() => {
    checkPermissions();
  }, []);

  const checkPermissions = async () => {
    if (Platform.OS === 'android') {
      try {
        const permissions = [
          PermissionsAndroid.PERMISSIONS.RECORD_AUDIO,
          PermissionsAndroid.PERMISSIONS.POST_NOTIFICATIONS,
        ];

        const granted = await PermissionsAndroid.requestMultiple(permissions);
        
        const audioGranted = granted[PermissionsAndroid.PERMISSIONS.RECORD_AUDIO] === PermissionsAndroid.RESULTS.GRANTED;
        const notifGranted = granted[PermissionsAndroid.PERMISSIONS.POST_NOTIFICATIONS] === PermissionsAndroid.RESULTS.GRANTED;

        if (audioGranted) {
          console.log('Permissões concedidas');
        } else {
          Alert.alert('Permissões', 'Permissão de áudio é necessária para o funcionamento do app');
        }
      } catch (err) {
        console.warn(err);
      }
    }
  };

  const connectToDevice = async () => {
    try {
      const connected = await RTLSDRNative.connectDevice();
      if (connected) {
        setIsConnected(true);
        setConnectionStatus('Dispositivo Conectado');
        Alert.alert('Sucesso', 'Dispositivo RTL-SDR conectado com sucesso!');
      } else {
        Alert.alert('Erro', 'Nenhum dispositivo RTL-SDR encontrado');
      }
    } catch (error) {
      Alert.alert('Erro', 'Erro ao conectar ao dispositivo');
      console.error(error);
    }
  };

  const toggleSDR = async () => {
    if (isRunning) {
      await RTLSDRNative.stopReading();
      setIsRunning(false);
    } else {
      if (!isConnected) {
        Alert.alert('Erro', 'Conecte um dispositivo primeiro');
        return;
      }
      const started = await RTLSDRNative.startReading();
      if (started) {
        setIsRunning(true);
        startSpectrumUpdate();
      }
    }
  };

  const startSpectrumUpdate = () => {
    const interval = setInterval(async () => {
      if (isRunning) {
        const data = await RTLSDRNative.getSpectrumData();
        setSpectrumData(data);
      } else {
        clearInterval(interval);
      }
    }, 100);
  };

  const toggleRecording = () => {
    setIsRecording(!isRecording);
    if (!isRecording) {
      Alert.alert('Gravação', 'Gravação iniciada');
    } else {
      Alert.alert('Gravação', 'Gravação parada');
    }
  };

  const toggleMute = () => {
    setIsMuted(!isMuted);
  };

  const onFrequencyChange = (text: string) => {
    setFrequency(text);
    const freq = parseFloat(text);
    if (freq >= 24.0 && freq <= 1766.0 && isConnected) {
      RTLSDRNative.setFrequency(freq);
    }
  };

  const onGainChange = (value: number) => {
    setGain(value);
    if (!autoGain && isConnected) {
      RTLSDRNative.setGain(value);
    }
  };

  const onAutoGainChange = (value: boolean) => {
    setAutoGain(value);
    if (isConnected) {
      RTLSDRNative.setAutoGain(value);
    }
  };

  // Chart data preparation
  const chartData = {
    labels: [],
    datasets: [
      {
        data: spectrumData.length > 0 ? spectrumData : [0],
        color: (opacity = 1) => `rgba(25, 118, 210, ${opacity})`,
        strokeWidth: 1,
      },
    ],
  };

  return (
    <ScrollView style={styles.container}>
      <View style={styles.statusContainer}>
        <Text style={[styles.statusText, {color: isConnected ? '#4CAF50' : '#F44336'}]}>
          {connectionStatus}
        </Text>
      </View>

      <View style={styles.buttonRow}>
        <TouchableOpacity
          style={[styles.button, styles.connectButton]}
          onPress={connectToDevice}>
          <Text style={styles.buttonText}>Conectar</Text>
        </TouchableOpacity>

        <TouchableOpacity
          style={[styles.button, isRunning ? styles.stopButton : styles.startButton]}
          onPress={toggleSDR}
          disabled={!isConnected}>
          <Text style={styles.buttonText}>
            {isRunning ? 'Parar SDR' : 'Iniciar SDR'}
          </Text>
        </TouchableOpacity>
      </View>

      <View style={styles.frequencyContainer}>
        <Text style={styles.label}>Frequência (MHz):</Text>
        <TextInput
          style={styles.frequencyInput}
          value={frequency}
          onChangeText={onFrequencyChange}
          keyboardType="decimal-pad"
          placeholder="88.5"
        />
      </View>

      <View style={styles.sliderContainer}>
        <Text style={styles.label}>Frequência: {frequency} MHz</Text>
        <Slider
          style={styles.slider}
          minimumValue={24}
          maximumValue={1766}
          value={parseFloat(frequency)}
          onValueChange={(value) => setFrequency(value.toFixed(1))}
          minimumTrackTintColor="#1976D2"
          maximumTrackTintColor="#CCCCCC"
          thumbStyle={styles.sliderThumb}
        />
      </View>

      <View style={styles.gainContainer}>
        <View style={styles.switchContainer}>
          <Text style={styles.label}>Ganho Automático:</Text>
          <Switch
            value={autoGain}
            onValueChange={onAutoGainChange}
            trackColor={{false: '#CCCCCC', true: '#1976D2'}}
            thumbColor={autoGain ? '#FFFFFF' : '#FFFFFF'}
          />
        </View>

        {!autoGain && (
          <View style={styles.sliderContainer}>
            <Text style={styles.label}>Ganho: {(gain / 10).toFixed(1)} dB</Text>
            <Slider
              style={styles.slider}
              minimumValue={0}
              maximumValue={496}
              value={gain}
              onValueChange={onGainChange}
              minimumTrackTintColor="#1976D2"
              maximumTrackTintColor="#CCCCCC"
              thumbStyle={styles.sliderThumb}
              disabled={autoGain}
            />
          </View>
        )}
      </View>

      <View style={styles.sliderContainer}>
        <Text style={styles.label}>Volume: {volume}%</Text>
        <Slider
          style={styles.slider}
          minimumValue={0}
          maximumValue={100}
          value={volume}
          onValueChange={setVolume}
          minimumTrackTintColor="#1976D2"
          maximumTrackTintColor="#CCCCCC"
          thumbStyle={styles.sliderThumb}
        />
      </View>

      <View style={styles.controlRow}>
        <TouchableOpacity
          style={[styles.button, styles.muteButton]}
          onPress={toggleMute}>
          <Text style={styles.buttonText}>
            {isMuted ? 'Desmutar' : 'Mutar'}
          </Text>
        </TouchableOpacity>

        <TouchableOpacity
          style={[styles.button, isRecording ? styles.stopButton : styles.recordButton]}
          onPress={toggleRecording}
          disabled={!isRunning}>
          <Text style={styles.buttonText}>
            {isRecording ? 'Parar Gravação' : 'Gravar'}
          </Text>
        </TouchableOpacity>
      </View>

      <View style={styles.spectrumContainer}>
        <Text style={styles.label}>Espectro:</Text>
        {spectrumData.length > 0 && (
          <LineChart
            data={chartData}
            width={screenData.width - 40}
            height={200}
            chartConfig={{
              backgroundColor: '#FFFFFF',
              backgroundGradientFrom: '#FFFFFF',
              backgroundGradientTo: '#FFFFFF',
              decimalPlaces: 0,
              color: (opacity = 1) => `rgba(25, 118, 210, ${opacity})`,
              labelColor: (opacity = 1) => `rgba(0, 0, 0, ${opacity})`,
              style: {
                borderRadius: 16,
              },
              propsForDots: {
                r: '0',
              },
            }}
            bezier
            style={styles.chart}
          />
        )}
      </View>

      <View style={styles.buttonRow}>
        <TouchableOpacity
          style={[styles.button, styles.spectrumButton]}
          onPress={() => navigation.navigate('Spectrum')}>
          <Text style={styles.buttonText}>Espectro</Text>
        </TouchableOpacity>

        <TouchableOpacity
          style={[styles.button, styles.settingsButton]}
          onPress={() => navigation.navigate('Settings')}>
          <Text style={styles.buttonText}>Configurações</Text>
        </TouchableOpacity>
      </View>
    </ScrollView>
  );
};

const styles = StyleSheet.create({
  container: {
    flex: 1,
    backgroundColor: '#F5F5F5',
    padding: 16,
  },
  statusContainer: {
    alignItems: 'center',
    marginBottom: 20,
  },
  statusText: {
    fontSize: 18,
    fontWeight: 'bold',
  },
  buttonRow: {
    flexDirection: 'row',
    justifyContent: 'space-between',
    marginBottom: 20,
  },
  button: {
    flex: 1,
    paddingVertical: 12,
    paddingHorizontal: 16,
    borderRadius: 8,
    marginHorizontal: 8,
    alignItems: 'center',
  },
  connectButton: {
    backgroundColor: '#4CAF50',
  },
  startButton: {
    backgroundColor: '#2196F3',
  },
  stopButton: {
    backgroundColor: '#F44336',
  },
  recordButton: {
    backgroundColor: '#FF9800',
  },
  muteButton: {
    backgroundColor: '#9C27B0',
  },
  spectrumButton: {
    backgroundColor: '#00BCD4',
  },
  settingsButton: {
    backgroundColor: '#607D8B',
  },
  buttonText: {
    color: '#FFFFFF',
    fontSize: 16,
    fontWeight: 'bold',
  },
  frequencyContainer: {
    marginBottom: 20,
  },
  frequencyInput: {
    borderWidth: 1,
    borderColor: '#CCCCCC',
    borderRadius: 8,
    padding: 12,
    fontSize: 16,
    backgroundColor: '#FFFFFF',
    marginTop: 8,
  },
  label: {
    fontSize: 16,
    fontWeight: 'bold',
    color: '#333333',
    marginBottom: 8,
  },
  sliderContainer: {
    marginBottom: 20,
  },
  slider: {
    width: '100%',
    height: 40,
  },
  sliderThumb: {
    backgroundColor: '#1976D2',
    width: 20,
    height: 20,
  },
  gainContainer: {
    marginBottom: 20,
  },
  switchContainer: {
    flexDirection: 'row',
    justifyContent: 'space-between',
    alignItems: 'center',
    marginBottom: 16,
  },
  controlRow: {
    flexDirection: 'row',
    justifyContent: 'space-between',
    marginBottom: 20,
  },
  spectrumContainer: {
    marginBottom: 20,
  },
  chart: {
    marginVertical: 8,
    borderRadius: 16,
  },
});

export default MainScreen;
