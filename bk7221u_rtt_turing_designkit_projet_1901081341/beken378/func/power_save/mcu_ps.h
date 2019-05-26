#ifndef _MCU_PS_H_
#define _MCU_PS_H_

//#define MCU_PS_DEBUG

#ifdef MCU_PS_DEBUG
#define MCU_PS_PRT                 os_printf

#else
#define MCU_PS_PRT                 os_null_printf

#endif

#if 1
#if (CFG_SOC_NAME == SOC_BK7231)

#define PWM_BASE                                     (0x00802A00)
#define PWM_INTERRUPT_STATUS                         (PWM_BASE + 1 * 4)

#define PWM0_BIT                                        (0)
#define PWM0_COUNTER                                 (PWM_BASE + 2 * 4)

#define PWM0_END_POSI                                (0)
#define PWM0_END_MASK                                (0xFFFF)
#define PWM0_DC_POSI                                 (16)
#define PWM0_DC_MASK                                 (0xFFFF)

#define PWM0_CAPTURE                                 (PWM_BASE + 3 * 4)

#define PWM0_CAP_OUT_POSI                            (0)
#define PWM0_CAP_OUT_MASK                            (0xFFFF)

#define PWM_CTL                                      (PWM_BASE + 0 * 4)
#else
#define PWM_NEW_BASE                                 (0x00802A00)
#define PWM_BASE                                     (PWM_NEW_BASE + 0x20 * 4 )
#define PWM_INTERRUPT_STATUS                         (PWM_BASE + 1 * 4)

#define PWM0_BIT                                        (0)
#define PWM0_COUNTER                                 (PWM_BASE + 2 * 4)

#define PWM0_END_POSI                                (0)
#define PWM0_END_MASK                                (0xFFFFFFFF)

#define PWM0_DUTY_CYCLE                              (PWM_BASE + 3 * 4)

#define PWM0_DC_POSI                                 (0)
#define PWM0_DC_MASK                                 (0xFFFFFFFF)

#define PWM0_CAPTURE                                 (PWM_BASE + 4 * 4)

#define PWM0_CAP_OUT_POSI                            (0)
#define PWM0_CAP_OUT_MASK                            (0xFFFFFFFF)


#define PWM_CTL                                      (PWM_BASE + 0 * 4)
#define ICU_PERI_CLK_MUX                             (ICU_BASE + 0 * 4)

#endif

#if (CFG_SOC_NAME == SOC_BK7231)
#else
#define TIMER0_CNT                                     (PWM_NEW_BASE + 0 * 4)

#define TIMER1_CNT                                     (PWM_NEW_BASE + 1 * 4)

#define TIMER2_CNT                                     (PWM_NEW_BASE + 2 * 4)

#define TIMER0_2_CTL                                   (PWM_NEW_BASE + 3 * 4)
#define TIMERCTL0_EN_BIT                               (0x01UL << 0)
#define TIMERCTL1_EN_BIT                               (0x01UL << 1)
#define TIMERCTL2_EN_BIT                               (0x01UL << 2)
#define TIMERCTLA_CLKDIV_POSI                          (3)
#define TIMERCTLA_CLKDIV_MASK                          (0x07)
#define TIMERCTL0_INT_BIT                              (0x01UL << 7)
#define TIMERCTL1_INT_BIT                              (0x01UL << 8)
#define TIMERCTL2_INT_BIT                              (0x01UL << 9)

#define TIMER3_CNT                                     (PWM_NEW_BASE + 0x10 * 4)

#define TIMER4_CNT                                     (PWM_NEW_BASE + 0x11 * 4)

#define TIMER5_CNT                                     (PWM_NEW_BASE + 0x12 * 4)

#define TIMER3_5_CTL                                   (PWM_NEW_BASE + 0x13 * 4)
#define TIMERCTL3_EN_BIT                               (0x01UL << 0)
#define TIMERCTL4_EN_BIT                               (0x01UL << 1)
#define TIMERCTL5_EN_BIT                               (0x01UL << 2)
#define TIMERCTLB_CLKDIV_POSI                          (3)
#define TIMERCTLB_CLKDIV_MASK                          (0x07)
#define TIMERCTL3_INT_BIT                              (0x01UL << 7)
#define TIMERCTL4_INT_BIT                              (0x01UL << 8)
#define TIMERCTL5_INT_BIT                              (0x01UL << 9)
#endif





#define ICU_PWM_CLK_MUX                              (ICU_BASE + 1 * 4)
#endif

#define PWM_CLK_LPO                                  (1)
#define PWM_CLK_PCLK                                 (0)
#define PWD_PWM0_CLK                                 (1 <<  9)

#define ICU_BASE                                     (0x00802000)

#define PWD_TIMER_32K_CLK                                  (1 << 21)
#define PWD_TIMER_26M_CLK                                  (1 << 20)
#define PWD_UART2_CLK                                (1 <<  1)
#define PWD_UART1_CLK                                (1 <<  0)

#define ICU_ARM_WAKEUP_EN                            (ICU_BASE + 20 * 4)

#define     MS_TO_TICK      2

#endif

