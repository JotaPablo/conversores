#include <stdio.h>
#include "pico/stdlib.h"
#include "inc/ssd1306.h"
#include "hardware/adc.h"
#include "pico/bootrom.h"
#include "hardware/pwm.h"


// Definição dos pinos dos LEDs RGB
#define RED_PIN 13
#define GREEN_PIN 11
#define BLUE_PIN 12

// Definição dos pinos dos botões
#define BUTTON_A 5
#define BUTTON_B 6
#define BUTTON_JOYSTICK 22

//Definição das variaveis do joystick
#define VRX_PIN 27  
#define VRY_PIN 26
#define ADC_MAX 4095
#define CENTRO 2047
#define DEADZONE 250  // Zona morta de 200 ao redor do centro (2047)

uint16_t vrx_valor;
uint16_t vry_valor;

uint16_t aplicar_deadzone(uint16_t valor_adc) {
    if (valor_adc >= (CENTRO - DEADZONE) && valor_adc <= (CENTRO + DEADZONE)) {
        return CENTRO;  // Define o valor como centro se estiver dentro da zona morta
    }
    return valor_adc;  // Retorna o valor original se estiver fora da zona morta
}


// Variáveis para debounce dos botões (armazenam tempo do último acionamento)
static volatile uint32_t last_time_button_a = 0;
static volatile uint32_t last_time_button_joystick = 0;

// Protótipos das funções de tratamento de interrupção
static void gpio_button_a_handler(uint gpio, uint32_t events);
static void gpio_button_b_handler(uint gpio, uint32_t events);
static void gpio_button_joystick_handler(uint gpio, uint32_t events);
static void gpio_button_handler(uint gpio, uint32_t events);

// Estrutura e funções do display OLED
static ssd1306_t ssd; // Estrutura de controle do display
void display_init();  // Inicialização do hardware

// Definções para o pwm
#define PWM_DIVISER 1.0      // Divisor de clock 
#define WRAP_PERIOD 2047      // Envolve o contador para trabalhar com valores de 0 a 2047

uint pwm_setup(uint pin);  // Prototipo da função pwm_setup
uint slice_red;            // Variável para o "slice" do PWM do LED vermelho
uint slice_blue;           // Variável para o "slice" do PWM do LED azul
bool pwm_enabled = true;   // Flag que indica se o PWM está habilitado
bool rotina_desativacao = false; // Flag para controlar a desativação do PWM
bool rotina_ativacao = false; // Flag para controlar a ativação do PWM

void atualiza_display();
void desenha_quadrado();
void desenha_borda();
uint borda = 0; // Variável que controla o tipo de borda a ser desenhada
void setup();


int main()
{
    setup();

    while (true)  while (true) {
        // Leitura dos valores dos eixos do joystick
        adc_select_input(1); // Seleciona o canal para o eixo X
        vrx_valor = adc_read();
        vrx_valor = aplicar_deadzone(vrx_valor);  // Aplica deadzone no eixo X

        adc_select_input(0); // Seleciona o canal para o eixo Y
        vry_valor = adc_read();
        vry_valor = aplicar_deadzone(vry_valor);  // Aplica deadzone no eixo Y

        printf("X: %d   /  Y: %d\n", vrx_valor, vry_valor);

        // Controle da potência dos LEDs RGB com PWM
        if(pwm_enabled){
            if(rotina_ativacao){
                rotina_ativacao = false;
                // Reconfigura pinos para PWM
                gpio_set_function(RED_PIN, GPIO_FUNC_PWM);
                gpio_set_function(BLUE_PIN, GPIO_FUNC_PWM);
        
                // Reativa slices PWM
                pwm_set_enabled(slice_red, true);
                pwm_set_enabled(slice_blue, true);
            }

            pwm_set_gpio_level(RED_PIN, abs(vrx_valor - 2047));  // Ajusta a intensidade(Duty Cicle) do LED vermelho com base no valor do eixo X
            pwm_set_gpio_level(BLUE_PIN, abs(vry_valor - 2047)); // Ajusta a intensidade(Duty Cicle) do LED azul com base no valor do eixo Y
        }
        else if(!pwm_enabled && rotina_desativacao == true){
            rotina_desativacao = false;
            pwm_set_gpio_level(RED_PIN, 0);
            pwm_set_gpio_level(BLUE_PIN, 0);
        
            // Desativa os slices PWM
            pwm_set_enabled(slice_red, false);
            pwm_set_enabled(slice_blue, false);
        
            // Reconfigura os pinos como saída padrão e define como LOW
            gpio_set_function(RED_PIN, GPIO_FUNC_SIO);
            gpio_set_dir(RED_PIN, GPIO_OUT);
            gpio_put(RED_PIN, 0);
        
            gpio_set_function(BLUE_PIN, GPIO_FUNC_SIO);
            gpio_set_dir(BLUE_PIN, GPIO_OUT);
            gpio_put(BLUE_PIN, 0);
        }

        atualiza_display();

        sleep_ms(100);
    }

}


uint pwm_setup(uint pin) {
    gpio_set_function(pin, GPIO_FUNC_PWM);  // Define a função do pino como PWM
    uint slice = pwm_gpio_to_slice_num(pin);  // Converte o pino para o número de slice correspondente
    pwm_set_clkdiv(slice, PWM_DIVISER);  // Define o divisor de clock para o PWM
    pwm_set_wrap(slice, WRAP_PERIOD);  // Define o período de wrap do PWM (limite do contador)
    pwm_set_enabled(slice, true);  // Habilita o PWM para o slice

    return slice;  // Retorna o número do slice utilizado
}

void desenha_quadrado(){

    uint pos_x = (vrx_valor/4095.0) * 44;
    uint pos_y = (vry_valor/4095.0) * 44;

    // Ajusta as posições para centralização

    pos_y = abs(44 - pos_y); // Inverte eixo Y
    pos_y += 8;
    pos_x += 43; // Centraliza o eixo x do quadrado

    //printf("PosX: %d   /  PosY: %d\n", pos_x, pos_y);


    // Desenha o quadrado de 8x8 pixels na nova posição
    ssd1306_rect(&ssd, pos_y, pos_x, 8, 8, true, true);  // Quadrado com preenchimento
}

void desenha_borda() {
    switch(borda) {
        case 0:
            ssd1306_rect(&ssd, 0, 32, 70, 64, true, false);  // Desenha uma borda
            break;
        case 1:
            ssd1306_rect(&ssd, 0, 32, 70, 64, true, false);  // Desenha a borda externa
            ssd1306_rect(&ssd, 2, 34, 66, 60, true, false);  // Desenha a borda interna
            break;
    }
}

void atualiza_display(){
    ssd1306_fill(&ssd, false);  // Limpa a tela
    desenha_quadrado();
    desenha_borda();
    ssd1306_send_data(&ssd); // Envia buffer para o display
}

// Tratador central de interrupções de botões
static void gpio_button_handler(uint gpio, uint32_t events) {
    switch(gpio) {
        case BUTTON_A:
            gpio_button_a_handler(gpio, events);
            break;
        case BUTTON_B:
            gpio_button_b_handler(gpio, events);
            break;
        case BUTTON_JOYSTICK:
            gpio_button_joystick_handler(gpio, events);
            break;
    }
}

// Tratador do botão A
static void gpio_button_a_handler(uint gpio, uint32_t events) {
    uint32_t current_time = to_us_since_boot(get_absolute_time());
    
    // Debounce: verifica se passaram pelo menos 200ms desde o último acionamento
    if (current_time - last_time_button_a > 200000) {
        last_time_button_a = current_time;


        //Alterna a flag para alterar os estados do pwm
        pwm_enabled = !pwm_enabled;
        if(pwm_enabled) rotina_ativacao = true;
        else rotina_desativacao = true;

    }
}

// Tratador do botão B
static void gpio_button_b_handler(uint gpio, uint32_t events) {
    printf("HABILITANDO O MODO GRAVAÇÃO");

    // Adicionar feedback no display OLED
    ssd1306_fill(&ssd, false);
    ssd1306_draw_string(&ssd, "  HABILITANDO", 5, 25);
    ssd1306_draw_string(&ssd, " MODO GRAVACAO", 5, 38);
    ssd1306_send_data(&ssd);

    reset_usb_boot(0,0); // Reinicia no modo DFU
}


// Tratador do botão do joystick
static void gpio_button_joystick_handler(uint gpio, uint32_t events) {
    
    uint32_t current_time = to_us_since_boot(get_absolute_time());
    
    // Debounce: verifica se passaram pelo menos 200ms
    if (current_time - last_time_button_joystick > 200000) {
        last_time_button_joystick = current_time;

        //Muda estado do led verde
        gpio_put(GREEN_PIN, !gpio_get(GREEN_PIN));

        //Altera a borda
        borda = (borda+1) % 2;
        
    }
}


void setup(){

    // Inicialização de sistemas básicos
    stdio_init_all(); // USB, stdio
        
    display_init();    // Configura display OLED

    adc_init();
    //adc_gpio_init(VRX_PIN);
    //adc_gpio_init(VRY_PIN);

    // Configura GPIOs dos LEDs
    gpio_init(GREEN_PIN);              
    gpio_set_dir(GREEN_PIN, GPIO_OUT); 
    gpio_put(GREEN_PIN, false); // Estado inicial desligado

    slice_red = pwm_setup(RED_PIN);

    slice_blue = pwm_setup(BLUE_PIN);

    // Configuração dos botões com pull-up
    gpio_init(BUTTON_A);
    gpio_set_dir(BUTTON_A, GPIO_IN);
    gpio_pull_up(BUTTON_A);

    gpio_init(BUTTON_B);
    gpio_set_dir(BUTTON_B, GPIO_IN);
    gpio_pull_up(BUTTON_B);

    gpio_init(BUTTON_JOYSTICK);
    gpio_set_dir(BUTTON_JOYSTICK, GPIO_IN);
    gpio_pull_up(BUTTON_JOYSTICK);

    // Configura interrupções para borda de descida (botão pressionado)
    gpio_set_irq_enabled_with_callback(BUTTON_A, GPIO_IRQ_EDGE_FALL, true, &gpio_button_handler);
    gpio_set_irq_enabled_with_callback(BUTTON_B, GPIO_IRQ_EDGE_FALL, true, &gpio_button_handler);
    gpio_set_irq_enabled_with_callback(BUTTON_JOYSTICK, GPIO_IRQ_EDGE_FALL, true, &gpio_button_handler);


}

// Inicialização do display OLED
void display_init() {
    // Configuração I2C a 400kHz
    i2c_init(I2C_PORT, 400 * 1000);
    
    // Configura pinos I2C
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA); // Ativa pull-ups internos
    gpio_pull_up(I2C_SCL);

    // Inicialização do controlador SSD1306
    ssd1306_init(&ssd, WIDTH, HEIGHT, false, endereco, I2C_PORT);
    ssd1306_config(&ssd);
    ssd1306_send_data(&ssd);

    // Limpa a tela inicialmente
    ssd1306_fill(&ssd, false);
    ssd1306_send_data(&ssd);
}


