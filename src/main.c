#include "queu_mcu.h"
#include "rcc_stm32f103c8.h"
#include "usart_stm32f103c8.h"
#include "gpio_stm32f103c8.h"
#include "interact_bit.h"

//USARTDIV = 39.0625, baud 115200, PCLK2 = 72MHz
//8N1

uint8_t usart1_receive(void) {
    while (!(USART1->SR & (1 << 5)));  
    return USART1->DR;                
}

void delay(volatile uint32_t d) {
    while(d--) { __asm("nop"); }
}

int main(){

    RCC->CR |= (1 << 16);
    while (!(RCC->CR & (1 << 17)));
    RCC->CFGR |= (7 << 18);
    RCC->CFGR |= (1 << 16);
    RCC->CR |= (1 << 24); //PLL on
    while (!(RCC->CR & (1 << 25))); //cho pll 
    RCC->CFGR = (RCC->CFGR & ~3) | 2;  //SYSCLK = PLL
    while (((RCC->CFGR >> 2) & 3) != 2); 


    RCC->APB2ENR |= (1 << 14); //USART1
    RCC->APB2ENR |= (1 << 4); //GPIOC
    RCC->APB2ENR |= (1 << 2);//GPIOA

    //config led PC13
    GPIOC->CRH &= ~(0xF << 20);
    GPIOC->CRH |= (1 << 20);
    setbit(&GPIOC->ODR, 13, 1); //tat led PC13 (active low)

    //USART1->BRR = (39 << 4);
    //USART1->BRR |= 1; //0.0625*16=1
    USART1->BRR = 625;

    //reset control reg
    USART1->CR1 = 0;   
    USART1->CR2 = 0;
    USART1->CR3 = 0;   

    USART1->CR1 |= (1 << 13); //enable usart1
    USART1->CR1 |= (1 << 3); //trans enb
    USART1->CR1 |= (1 << 2); //re enb

    //set mode alternate PA9 10
    //PA9 TX
    GPIOA->CRH &= ~(0xF << 4); 
    GPIOA->CRH |=  (0xB << 4);//1011

    //PA10 RX
    GPIOA->CRH &= ~(0xF << 8); 
    GPIOA->CRH |=  (0x4 << 8);//0100

    volatile Queu q;
    q.head=0;
    q.tail=0;
    uint8_t temp;
    uint8_t r;
    while (1){
        temp = usart1_receive(); 
        if (!isFull(&q)){
            enqueu(&q, temp);
        }
        if (isEmpty(&q) == 0){
            dequeu(&q, &r);
        }
    }

    return 0;
}