/**
 * Radio SDR App - React Native
 * @format
 */

import React from 'react';
import {NavigationContainer} from '@react-navigation/native';
import {createStackNavigator} from '@react-navigation/stack';
import {StatusBar} from 'react-native';

import MainScreen from './src/screens/MainScreen';
import SpectrumScreen from './src/screens/SpectrumScreen';
import SettingsScreen from './src/screens/SettingsScreen';

const Stack = createStackNavigator();

const App: React.FC = () => {
  return (
    <NavigationContainer>
      <StatusBar backgroundColor="#1976D2" barStyle="light-content" />
      <Stack.Navigator
        initialRouteName="Main"
        screenOptions={{
          headerStyle: {
            backgroundColor: '#1976D2',
          },
          headerTintColor: '#fff',
          headerTitleStyle: {
            fontWeight: 'bold',
          },
        }}>
        <Stack.Screen
          name="Main"
          component={MainScreen}
          options={{title: 'Radio SDR'}}
        />
        <Stack.Screen
          name="Spectrum"
          component={SpectrumScreen}
          options={{title: 'Espectro'}}
        />
        <Stack.Screen
          name="Settings"
          component={SettingsScreen}
          options={{title: 'Configurações'}}
        />
      </Stack.Navigator>
    </NavigationContainer>
  );
};

export default App;
