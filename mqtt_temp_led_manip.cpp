#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "lwip/apps/mqtt.h"
#include "bsp/board.h"
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include "hardware/adc.h"

#define END_MQTT "35.172.255.228"

float temperatura_sensor = 0;
int speed_led = 0;
// Conjunto de variáveis para verificar topico do LED e mensagem de numero inteiro positivo
bool topico_led_acessado = false;
bool mensagem_valida = false;
// Variável cujo valor é o resultado de uma operação AND entre as duas variáveis acima
bool chave_led = topico_led_acessado && mensagem_valida;

struct mqtt_connect_client_info_t info_cliente =
{
    "wendell_navarro", /*Identificador do Usuário*/
    NULL, /*Usuário*/
    NULL, /*Senha*/
    0, /*Tempo de keep alive*/
    NULL, /*Tópico do Ultimo desejo*/
    NULL, /*Mensagem do Ultimo desejo*/
    0, /*Quality Of Service do ultimo desejo*/
    0 /*Ultimo Desejo Retentivo*/
};

// Função reutilizada do exercício 1 para piscar um LED, neste caso o LED embutido da própria placa
void built_in_led_blink(int speed) {
  cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
  sleep_ms(speed);
  cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
}

// Função para verificar se uma string é composta apenas por números inteiros positivos
bool verificacao_mensagem(const char *str) {
    // Verificando se a string é nula, vazia ou composta pelo número zero
    printf("(ANTES IF) Verificando mensagem: %s\n", str);
    if (str == NULL || *str == '\0' || str == 0) return false;
    printf("(DEPOIS PRIMEIRO IF) Verificando mensagem: %s\n", str);
    if (atoi(str) == 0) return false;
    printf("(DPS SEGUNDO IF) Verificando mensagem: %s\n", str);
    return true;
}

// Função para checkar se o tópico recebido condiz com o tópico que queremos manipular
bool verificacao_topico(const char *topico) {
  printf("Chegou tópico: %s\n", topico);
  const char *topico_esperado = "/wendell_navarro/240025427/led";
  printf("Tópico esperado: %s\n", topico_esperado);
  return strncmp(topico, topico_esperado, strlen(topico_esperado)) == 0;
}

static void mqtt_dados_recebidos_cb(void *arg, const uint8_t *dados, uint16_t tamanho, uint8_t flags){
  printf("dados brutos recebidos:\n %s\n", dados);
  printf("tamanho dos dados recebidos: %d\n", tamanho);
  char limite_velocidade = 4;
  if (tamanho > limite_velocidade) {
    printf("Erro: Tamanho dos dados recebidos excede o buffer\n");
    return;
  }
  if(verificacao_mensagem((const char*)dados)) {
    mensagem_valida = true;
    printf("Status da mensagem: %d\n", mensagem_valida);
    // printf("dados depois da validação:\n %s\n", dados_tratados);
    int velocidade = atoi((const char*)dados);
    speed_led = velocidade;
    printf("Mensagem válida recebida\n");
  } else {
    mensagem_valida = false;
    printf("Mensagem inválida recebida\n");
  }
}

static void mqtt_recebendo_publicacao_cb(void *arg, const char *topico, uint32_t tamanho){
  printf("recebendo mensagem no topico %s\n", *topico);
  if (verificacao_topico(topico)) {
    topico_led_acessado = true;
    printf("Tópico LED acessado com sucesso\n");
  } else {
    topico_led_acessado = false;
    printf("Tópico recebido não é o tópico de LED\n");
  }
}

static void mqtt_requisicao_cb(void *arg, err_t error){
  printf("requisicao recebida com o codigo %d\n", error);
}

static bool connectado_com_sucesso = false;
static void mqtt_conectado_cb(mqtt_client_t *cliente, void *arg, mqtt_connection_status_t status) {
    printf("Cliente MQTT conectado com o status %d\n", status);
    if (status == MQTT_CONNECT_ACCEPTED) {
        printf("Conexão MQTT aceita\n");
        connectado_com_sucesso = true;
        err_t erro_led = mqtt_sub_unsub(cliente, "/wendell_navarro/240025427/led", 0, mqtt_requisicao_cb, arg, 1);
        if (erro_led == ERR_OK) {
          printf("Inscrito no tópico de LED com sucesso\n");
        }
    }
}

int main()
{
    stdio_init_all();

    // inicialização do Analogic-Digital Conversor
    adc_init();

    /*
    Inicialização do sensor de temperatura interno

    Vale lembrar que segundo pesquisas, essa joça buga no Wokwi
    então não, mesmo o Wokwi falando que sim, não era pra plaquinha
    estar derretendo a 437º C
    */
    adc_set_temp_sensor_enabled(true);

    // Selecionando o sensor como source do ADC
    adc_select_input(4);

    sleep_ms(5000);
    // Initialise the Wi-Fi chip
    if (cyw43_arch_init()) {
        printf("Wi-Fi init failed\n");
        return -1;
    }

    /*
     * Ligando o Wifi como cliente 
    */
    cyw43_arch_enable_sta_mode();
    printf("Conectando ao Wifi\n");

    if(cyw43_arch_wifi_connect_timeout_ms("Wesley_Zeppeli", "zeppeliGyro", CYW43_AUTH_WPA2_AES_PSK, 30000)) {
        printf("Erro ao conectar ao wifi\n");
        return -1;
    } else {
        printf("Conectado!!!!\n");
        uint8_t *end_ip = (uint8_t*) &(cyw43_state.netif[0].ip_addr.addr);
        printf("Endereço IP: %d.%d.%d.%d\n", end_ip[0], end_ip[1], end_ip[2], end_ip[3]);
    }

    ip_addr_t end_mqtt;
    ip4addr_aton(END_MQTT, &end_mqtt);

    mqtt_client_t *mqtt_cliente = mqtt_client_new();

    mqtt_set_inpub_callback(mqtt_cliente, &mqtt_recebendo_publicacao_cb, &mqtt_dados_recebidos_cb, NULL);

    err_t houve_erro = mqtt_client_connect(mqtt_cliente, &end_mqtt, 1883, &mqtt_conectado_cb, NULL, &info_cliente);

    if (houve_erro != ERR_OK){
        printf("Falha na requisião de conexão MQTT\n");
        return 1;
    }

    // Example to turn on the Pico W LED
    // cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);


    while (true) {
        // Lendo o valor vindo do ADC
        uint16_t raw_val = adc_read();

        /*
        Convertendo esse valor bruto em voltagem com base na refêrncia
        de 3.3V, com resolução de 12 bits pra poder transformar o resultado
        lido no ADC em uma voltagem que podemos usar
        */

        const float conversion_factor = 3.3f / (1 << 12);
        float voltage = raw_val * conversion_factor;

        /*
        Convertendo a voltagem obtida acima em temperatura (em Celsius)
        seguindo a fórmula encontrada na datasheet a seguir:
        https://pip-assets.raspberrypi.com/categories/814-rp2040/documents/RP-008371-DS-1-rp2040-datasheet.pdf?disposition=inline
        */
        float temperature = (float)27.0f - ((voltage - 0.706f) / 0.001721f);
        temperatura_sensor = temperature;

        // Criando uma varíavel para poder mostrar a temperatura no tópico do MQTT, já que o publish não aceita formatação de string
        char mensagem_temperatura[50];

        // Formatando e guardando a mensagem no buffer
        snprintf(mensagem_temperatura, sizeof(mensagem_temperatura), "Internal Temperature: %.2f °C\n", temperature);

        // Mostrando temperatura toda vez que o botão é pressioando:
        if(board_button_read()) {
          mqtt_publish(mqtt_cliente, "/wendell_navarro/240025427/temperatura", mensagem_temperatura, strlen(mensagem_temperatura), 0, 0, &mqtt_requisicao_cb, NULL);
          
          // Verificação de erro na publicação da temperatura, caso haja algum problema na conexão ou na publicação
          err_t erro_temperatura = mqtt_publish(mqtt_cliente, "/wendell_navarro/240025427/temperatura", mensagem_temperatura, strlen(mensagem_temperatura), 0, 0, &mqtt_requisicao_cb, NULL);
          if (erro_temperatura != ERR_OK) {
            printf("Erro imediato ao tentar publicar: %d\n", erro_temperatura);
          } else {
            printf("Mensagem colocada na fila de envio com sucesso!\n");
          }
        }
        sleep_ms(1000);
        
        if (mensagem_valida && topico_led_acessado) {
          chave_led = true;
        } else {
          chave_led = false;
        }

        if (chave_led) {
          built_in_led_blink(speed_led);
        } else {
          /*
          Caso a chave seja falsa, por conta do tópico do LED não ser acessado ou a mensagem
          recebida não ser um número inteiro positivo, o LED é desligado
          */
          cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
        }
    }
} 