# Sistema de Irrigação Automatizada com ESP32

Este projeto implementa um sistema inteligente de irrigação utilizando ESP32, sensor de umidade do solo e acionamento automático por relé. O objetivo é controlar a umidade do solo de forma eficiente, evitando desperdício de água e permitindo programação de horários específicos para irrigação.

---

## 🚀 **Visão Geral do Projeto**

O sistema monitora em tempo real a umidade do solo e ativa a bomba d'água quando necessário.

### **Principais Recursos**

* Leitura contínua da umidade do solo.
* Controle automático da bomba via relé.
* Conexão Wi-Fi (opcional, dependendo da versão do código).

---

## 🧰 **Tecnologias Utilizadas**

* **ESP32**
* **Sensor de Umidade do Solo (capacitivo ou resistivo)**
* **Relé 5V ou módulo SSR**
* **Linguagem C++ (Arduino)**
* **IDE Arduino ou PlatformIO**

---

## 📌 **Funcionamento do Sistema**

1. O ESP32 lê constantemente o valor do sensor de umidade.
2. Se o solo estiver seco abaixo do limite definido, a bomba é acionada.


---

## 📸 **Esquema de Conexão (Resumo)**

* Sensor → Pino analógico (ex.: 34)
* Relé → Pino digital (ex.: 14)
* Alimentação conforme especificações de cada módulo

> Recomenda-se sempre testar o sensor antes de instalar em campo e utilizar fonte externa para a bomba.

---

## 🧪 **Testes Realizados**

* Teste de leitura do sensor em diferentes umidades.
* Verificação da estabilidade da bomba com relé.
* Teste completo em ambiente real (solo + reservatório).

---

## 📈 **Resultados Obtidos**

* Automação completa da irrigação, eliminando intervenção manual.
* Economia de água perceptível.
* Sistema robusto após ajustes no código e correção de bugs.
* Integração fácil com aplicativos e dashboards.

---

## 💡 **Possíveis Melhorias Futuras**

* Adicionar interface web para configuração dos horários.
* Criar painel de monitoramento online.
* Adicionar múltiplas zonas de irrigação.
* Envio de alertas via Telegram ou WhatsApp.

---

## 👤 **Autor(a)**

Projeto desenvolvido por **Lívia Araújo, Gabriel Ferreira, Emanuel Santana e Kayo Andrade**, como parte de um trabalho prático de automação e IoT, unindo teoria da disciplina com experiência prática.

---

## 📄 **Licença**

Este projeto está sob a licença MIT. Sinta-se livre para usar, modificar e contribuir.

---

Se desejar, posso incluir imagens, fluxogramas, instruções de instalação, ou até integrar o código completo no README!

