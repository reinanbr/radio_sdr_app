# Contribuindo para SDR Radio Android Apps 🤝

Obrigado pelo interesse em contribuir! Este documento fornece diretrizes para contribuir com o projeto.

## 🚀 Como Contribuir

### 1. Preparando o Ambiente

```bash
# Clone o repositório
git clone https://github.com/reinanbr/radio_sdr_app.git
cd radio_sdr_app

# Configure o JDK 21
export JAVA_HOME=/usr/lib/jvm/java-21-openjdk-amd64

# Teste os builds
cd kotlin && ./gradlew assembleDebug
cd ../java && ./gradlew assembleDebug
```

### 2. Fluxo de Desenvolvimento

1. **Fork** o repositório
2. **Clone** seu fork
3. **Crie** uma branch para sua feature:
   ```bash
   git checkout -b feature/minha-nova-feature
   ```
4. **Desenvolva** e **teste** suas mudanças
5. **Commit** com mensagens descritivas:
   ```bash
   git commit -m "feat: adiciona análise FFT melhorada"
   ```
6. **Push** para seu fork:
   ```bash
   git push origin feature/minha-nova-feature
   ```
7. **Abra** um Pull Request

## 📝 Diretrizes de Código

### Kotlin (Recomendado)
```kotlin
// Use nomenclatura descritiva
class SpectrumAnalyzer {
    fun analyzeFrequency(samples: FloatArray): FrequencyData {
        // Documente código complexo
        // Implemente DSP aqui
    }
}
```

### Java
```java
// Siga convenções Java padrão
public class RTLSDRDevice {
    private static final int DEFAULT_SAMPLE_RATE = 2048000;
    
    public void configure(int sampleRate) {
        // Validação de parâmetros
        if (sampleRate <= 0) {
            throw new IllegalArgumentException("Sample rate deve ser positivo");
        }
    }
}
```

### C++ (Nativo)
```cpp
// Use C++17 features
extern "C" JNIEXPORT jfloatArray JNICALL
Java_com_sdrradio_SDRRadio_nativeProcessSignal(JNIEnv *env, jobject thiz, jfloatArray input) {
    // Otimize para performance
    // Use SIMD quando possível
}
```

## 🧪 Testes

### Testando Builds
```bash
# Teste todos os projetos
for project in java java_2 kotlin cpp; do
    echo "Testing $project..."
    cd $project && ./gradlew test && cd ..
done
```

### Testando Funcionalidades
- ✅ Detecção USB RTL-SDR
- ✅ Permissões de áudio/armazenamento
- ✅ Análise de espectro
- ✅ Gravação de sinais
- ✅ Interface responsiva

## 📋 Tipos de Contribuição

### 🐛 Correções de Bug
- Descreva o problema claramente
- Inclua passos para reproduzir
- Teste a correção em dispositivos reais

### ✨ Novas Features
- Discuta a feature em uma issue primeiro
- Mantenha compatibilidade com Android 7+
- Documente mudanças de API

### 📖 Documentação
- Melhore README.md
- Adicione comentários no código
- Crie tutoriais de uso

### 🎨 UI/UX
- Siga Material Design Guidelines
- Teste em diferentes tamanhos de tela
- Mantenha acessibilidade

## 🔧 Configuração de Desenvolvimento

### Android Studio
- **Versão**: Arctic Fox ou superior
- **Plugins recomendados**:
  - Kotlin
  - Android NDK
  - GitLab/GitHub integration

### Configurações Gradle
```properties
# gradle.properties locais
org.gradle.jvmargs=-Xmx4g -Dfile.encoding=UTF-8
org.gradle.parallel=true
org.gradle.caching=true
android.useAndroidX=true
```

## 📱 Testes em Dispositivos

### Dispositivos Recomendados
- **Android 7-15** (API 24-35)
- **USB OTG** habilitado
- **RTL-SDR dongle** para testes completos

### Testes Obrigatórios
1. **Instalação limpa** do APK
2. **Permissões** concedidas corretamente
3. **Detecção USB** funcional
4. **Interface** responsiva
5. **Performance** aceitável

## 📊 Pull Request Guidelines

### Checklist antes do PR
- [ ] Código buildado sem erros
- [ ] Testes passando
- [ ] Documentação atualizada
- [ ] Screenshots adicionados (se UI)
- [ ] Compatibilidade verificada

### Template de PR
```markdown
## Descrição
Breve descrição das mudanças.

## Tipo de mudança
- [ ] Bug fix
- [ ] Nova feature
- [ ] Breaking change
- [ ] Documentação

## Como testar
1. Clone o branch
2. Rode `./gradlew assembleDebug`
3. Teste funcionalidade X

## Screenshots (se aplicável)
![Antes](antes.png) ![Depois](depois.png)

## Checklist
- [ ] Testado em dispositivo real
- [ ] Documentação atualizada
- [ ] Sem warnings de build
```

## 🏷️ Convenções de Commit

Use [Conventional Commits](https://www.conventionalcommits.org/):

```bash
# Features
git commit -m "feat: adiciona filtro passa-baixa no DSP"

# Bug fixes  
git commit -m "fix: corrige detecção USB em Android 13+"

# Documentação
git commit -m "docs: atualiza instruções de build"

# Performance
git commit -m "perf: otimiza FFT com SIMD"

# Refactoring
git commit -m "refactor: reorganiza classes de UI"
```

## 🎯 Áreas que Precisam de Contribuição

### Alta Prioridade
- 🔴 **Otimização DSP** - Implementar algoritmos mais eficientes
- 🔴 **Suporte a mais dongles** - Adicionar novos dispositivos RTL-SDR
- 🔴 **Testes automatizados** - Expandir cobertura de testes

### Média Prioridade  
- 🟡 **Interface melhorada** - Material Design 3
- 🟡 **Modo escuro** - Theme adaptativo
- 🟡 **Exportação de dados** - Formatos CSV, JSON

### Baixa Prioridade
- 🟢 **Localização** - Suporte a mais idiomas
- 🟢 **Tutorial integrado** - Onboarding para novos usuários
- 🟢 **Análise avançada** - Waterfall, demodulação

## 🤔 Precisa de Ajuda?

- 💬 **Discussions**: Para perguntas gerais
- 🐛 **Issues**: Para bugs e feature requests  
- 📧 **Email**: Para questões privadas
- 📚 **Wiki**: Para documentação extensa

## 🙏 Reconhecimento

Contribuidores serão listados no README.md e releases notes.

### Tipos de Contribuição
- 🏆 **Core Developer** - Contribuições significativas ao código
- 🔧 **Maintainer** - Manutenção e reviews
- 📖 **Documentation** - Melhorias na documentação
- 🎨 **Design** - UI/UX e assets
- 🧪 **Testing** - Testes e QA
- 🌍 **Translation** - Localização

---

**Obrigado por contribuir! Juntos tornamos o SDR mais acessível! 📻✨**
