import React, {useState, useEffect} from 'react';
import {
  View,
  Text,
  StyleSheet,
  ScrollView,
  Dimensions,
  TouchableOpacity,
} from 'react-native';
import {LineChart} from 'react-native-chart-kit';
import {RTLSDRNative} from '../native/RTLSDRNative';

const SpectrumScreen: React.FC = () => {
  const [spectrumData, setSpectrumData] = useState<number[]>([]);
  const [isRunning, setIsRunning] = useState(false);
  const [updateInterval, setUpdateInterval] = useState<NodeJS.Timeout | null>(null);

  const screenData = Dimensions.get('window');

  useEffect(() => {
    startSpectrumUpdate();
    return () => {
      if (updateInterval) {
        clearInterval(updateInterval);
      }
    };
  }, []);

  const startSpectrumUpdate = () => {
    setIsRunning(true);
    const interval = setInterval(async () => {
      try {
        const data = await RTLSDRNative.getSpectrumData();
        setSpectrumData(data);
      } catch (error) {
        console.error('Erro ao obter dados do espectro:', error);
      }
    }, 50); // Update every 50ms for smoother animation

    setUpdateInterval(interval);
  };

  const stopSpectrumUpdate = () => {
    setIsRunning(false);
    if (updateInterval) {
      clearInterval(updateInterval);
      setUpdateInterval(null);
    }
  };

  const toggleUpdate = () => {
    if (isRunning) {
      stopSpectrumUpdate();
    } else {
      startSpectrumUpdate();
    }
  };

  // Prepare chart data
  const chartData = {
    labels: [], // Empty labels for cleaner look
    datasets: [
      {
        data: spectrumData.length > 0 ? spectrumData : [0],
        color: (opacity = 1) => `rgba(0, 188, 212, ${opacity})`,
        strokeWidth: 2,
      },
    ],
  };

  // Generate waterfall data (simplified version)
  const waterfallHeight = 300;
  const waterfallData = [];
  for (let i = 0; i < waterfallHeight; i++) {
    waterfallData.push(spectrumData.map(val => val + Math.random() * 10));
  }

  return (
    <ScrollView style={styles.container}>
      <View style={styles.header}>
        <Text style={styles.title}>Analisador de Espectro</Text>
        <TouchableOpacity
          style={[styles.button, isRunning ? styles.stopButton : styles.startButton]}
          onPress={toggleUpdate}>
          <Text style={styles.buttonText}>
            {isRunning ? 'Parar' : 'Iniciar'}
          </Text>
        </TouchableOpacity>
      </View>

      <View style={styles.spectrumContainer}>
        <Text style={styles.sectionTitle}>Espectro de Frequência</Text>
        {spectrumData.length > 0 && (
          <LineChart
            data={chartData}
            width={screenData.width - 32}
            height={250}
            chartConfig={{
              backgroundColor: '#000000',
              backgroundGradientFrom: '#000000',
              backgroundGradientTo: '#1a1a1a',
              decimalPlaces: 0,
              color: (opacity = 1) => `rgba(0, 255, 255, ${opacity})`,
              labelColor: (opacity = 1) => `rgba(255, 255, 255, ${opacity})`,
              style: {
                borderRadius: 16,
              },
              propsForDots: {
                r: '0',
              },
              propsForBackgroundLines: {
                strokeDasharray: '',
                strokeWidth: 1,
                stroke: 'rgba(255, 255, 255, 0.2)',
              },
            }}
            bezier={false}
            style={styles.chart}
            withDots={false}
            withInnerLines={true}
            withOuterLines={true}
            withVerticalLines={true}
            withHorizontalLines={true}
          />
        )}
      </View>

      <View style={styles.waterfallContainer}>
        <Text style={styles.sectionTitle}>Diagrama de Cascata (Waterfall)</Text>
        <View style={styles.waterfallPlaceholder}>
          <Text style={styles.placeholderText}>
            Waterfall Display
          </Text>
          <Text style={styles.placeholderSubtext}>
            (Implementação em desenvolvimento)
          </Text>
        </View>
      </View>

      <View style={styles.infoContainer}>
        <Text style={styles.sectionTitle}>Informações</Text>
        <View style={styles.infoRow}>
          <Text style={styles.infoLabel}>Taxa de Atualização:</Text>
          <Text style={styles.infoValue}>20 Hz</Text>
        </View>
        <View style={styles.infoRow}>
          <Text style={styles.infoLabel}>Resolução FFT:</Text>
          <Text style={styles.infoValue}>1024 pontos</Text>
        </View>
        <View style={styles.infoRow}>
          <Text style={styles.infoLabel}>Largura de Banda:</Text>
          <Text style={styles.infoValue}>2.048 MHz</Text>
        </View>
        <View style={styles.infoRow}>
          <Text style={styles.infoLabel}>Status:</Text>
          <Text style={[styles.infoValue, {color: isRunning ? '#4CAF50' : '#F44336'}]}>
            {isRunning ? 'Ativo' : 'Parado'}
          </Text>
        </View>
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
  header: {
    flexDirection: 'row',
    justifyContent: 'space-between',
    alignItems: 'center',
    marginBottom: 20,
  },
  title: {
    fontSize: 24,
    fontWeight: 'bold',
    color: '#333333',
  },
  button: {
    paddingVertical: 8,
    paddingHorizontal: 16,
    borderRadius: 8,
    alignItems: 'center',
  },
  startButton: {
    backgroundColor: '#4CAF50',
  },
  stopButton: {
    backgroundColor: '#F44336',
  },
  buttonText: {
    color: '#FFFFFF',
    fontSize: 14,
    fontWeight: 'bold',
  },
  spectrumContainer: {
    marginBottom: 30,
  },
  sectionTitle: {
    fontSize: 18,
    fontWeight: 'bold',
    color: '#333333',
    marginBottom: 12,
  },
  chart: {
    marginVertical: 8,
    borderRadius: 16,
    alignSelf: 'center',
  },
  waterfallContainer: {
    marginBottom: 30,
  },
  waterfallPlaceholder: {
    height: 200,
    backgroundColor: '#000000',
    borderRadius: 16,
    justifyContent: 'center',
    alignItems: 'center',
    borderWidth: 1,
    borderColor: '#333333',
  },
  placeholderText: {
    color: '#FFFFFF',
    fontSize: 18,
    fontWeight: 'bold',
  },
  placeholderSubtext: {
    color: '#CCCCCC',
    fontSize: 14,
    marginTop: 8,
  },
  infoContainer: {
    backgroundColor: '#FFFFFF',
    borderRadius: 16,
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
  infoRow: {
    flexDirection: 'row',
    justifyContent: 'space-between',
    alignItems: 'center',
    paddingVertical: 8,
    borderBottomWidth: 1,
    borderBottomColor: '#F0F0F0',
  },
  infoLabel: {
    fontSize: 16,
    color: '#666666',
  },
  infoValue: {
    fontSize: 16,
    fontWeight: 'bold',
    color: '#333333',
  },
});

export default SpectrumScreen;
