//For the TI Stellaris Tiva C Launchpad Board (ARM Cortex M4):

#define RCGCGPIO ( *( ( volatile unsigned long *) 0x400fe608U ) ) //Part of SYSCTL
                                                
#define GPIOF_BASE 0x40025000U
#define GPIO_PORTF_AHB_DIR_R    (*((volatile unsigned long *)0x4005D400))
#define GPIO_PORTF_AHB_DEN_R    (*((volatile unsigned long *)0x4005D51C))
#define GPIOF_DATA ( *( ( volatile unsigned long * ) ( GPIOF_BASE + 0x3fcU ) ) )

#define GPIO_PORTF_AHB_DATA_BITS_R \
                                ((volatile unsigned long *)0x4005D000)

#define SYSCTL_GPIOHBCTL_R      (*((volatile unsigned long *)0x400FE06C))

#define LED_RED (1U << 1)
#define LED_BLUE (1U << 2)
#define LED_GREEN (1U << 3)

int main() {
    
    RCGCGPIO = (1U << 5); //Clock Gating for PortF
    
    SYSCTL_GPIOHBCTL_R = (1U << 5); //AHB for PortF
    
    GPIO_PORTF_AHB_DIR_R = ( LED_RED | LED_BLUE | LED_GREEN );//Pin Direction
                                                   
    GPIO_PORTF_AHB_DEN_R = ( LED_RED | LED_BLUE | LED_GREEN );//Digital Function Register
                                                   
    GPIO_PORTF_AHB_DATA_BITS_R [ LED_BLUE ] = LED_BLUE; //Independent Write to Bits
    
    while ( 1 ) {
     GPIO_PORTF_AHB_DATA_BITS_R [ LED_RED ] = LED_RED;
     GPIO_PORTF_AHB_DATA_BITS_R [ LED_RED ] = 0;
    }
    return 0;

}
