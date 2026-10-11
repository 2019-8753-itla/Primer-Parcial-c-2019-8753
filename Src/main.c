/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <string.h>
#include <stdio.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim6;

UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */

volatile uint32_t g_ms = 0;
volatile uint8_t buttonEdgePending = 0;

uint8_t buttonRawState = 0;
uint8_t buttonStableState = 0;

uint8_t debounceActive = 0;
uint32_t debounceStart = 0;

uint32_t pressStart = 0;
uint8_t buttonPressed = 0;
uint8_t longPressReported = 0;

uint32_t shortPressCount = 0;
uint32_t longPressCount = 0;

/* Recepcion UART por interrupciones */
#define UART_RX_BUFFER_SIZE 128U

static uint8_t uartRxByte;

static volatile uint8_t uartRxBuffer[UART_RX_BUFFER_SIZE];

static volatile uint16_t uartRxHead = 0U;
static volatile uint16_t uartRxTail = 0U;

static volatile uint32_t uartRxOverflowCount = 0U;

/* Procesamiento de comandos UART */
#define UART_LINE_MAX_LENGTH 63U

static char uartLine[UART_LINE_MAX_LENGTH + 1U];

static uint8_t uartLineLength = 0U;

static uint8_t uartLineDiscard = 0U;

static uint8_t uartLastWasCR = 0U;

static volatile uint32_t uartLineOverflowCount = 0U;

/* Transmision UART no bloqueante */
#define UART_TX_BUFFER_SIZE 256U

static uint8_t uartTxBuffer[UART_TX_BUFFER_SIZE];

static volatile uint16_t uartTxHead = 0U;
static volatile uint16_t uartTxTail = 0U;

static volatile uint8_t uartTxBusy = 0U;

static volatile uint32_t uartTxOverflowCount = 0U;

static uint8_t uartTxActiveByte;

static uint8_t pwmDuty = 25U;

typedef enum
{
    MODE_MANUAL = 0,
    MODE_AUTO,
    MODE_PAUSE
} SystemMode;

static SystemMode currentMode = MODE_MANUAL;
static SystemMode previousMode = MODE_MANUAL;

static uint8_t savedDuty = 25U;

/* Control de rampa automatica */
static uint8_t autoDuty = 10U;
static int8_t autoDirection = 1;
static uint32_t autoLastUpdate = 0U;

static uint8_t manualDuty = 25U;

/* Control de transmision periodica UART */
static uint8_t streamEnabled = 0U;
static uint32_t streamLastUpdate = 0U;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_TIM2_Init(void);
static void MX_TIM6_Init(void);
/* USER CODE BEGIN PFP */

uint8_t UART_SendText(const char *text);
void UART_ServiceTx(void);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

uint8_t UART_ParseDuty(const char *text, uint8_t *duty)
{
    uint32_t value = 0U;

    if ((text == NULL) || (duty == NULL) || (*text == '\0'))
    {
        return 0U;
    }

    while (*text != '\0')
    {
        if ((*text < '0') || (*text > '9'))
        {
            return 0U;
        }

        value = (value * 10U) + (uint32_t)(*text - '0');

        if (value > 100U)
        {
            return 0U;
        }

        text++;
    }

    *duty = (uint8_t)value;

    return 1U;
}

void PWM_SetDuty(uint8_t duty)
{
    if (duty > 100U)
    {
        duty = 100U;
    }

    /* Guardar el ultimo duty configurado */
    pwmDuty = duty;

    /* Obtener el periodo actual de TIM2 */
    uint32_t period = __HAL_TIM_GET_AUTORELOAD(&htim2);

    /* Calcular el valor del registro CCR1 */
    uint32_t pulse = ((period + 1U) * duty) / 100U;

    /* Actualizar el PWM */
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, pulse);
}

uint8_t PWM_SetFrequency(uint32_t frequency)
{
    uint32_t timerClock = 64000000U;

    /* Validar las frecuencias permitidas */
    if ((frequency != 500U) &&
        (frequency != 1000U) &&
        (frequency != 2000U))
    {
        return 0U;
    }

    /* Leer el prescaler actual */
    uint32_t prescaler = htim2.Instance->PSC;

    /* Calcular el nuevo periodo */
    uint32_t newPeriod =
        (timerClock / ((prescaler + 1U) * frequency)) - 1U;

    /* Actualizar el periodo del PWM */
    __HAL_TIM_SET_AUTORELOAD(&htim2, newPeriod);

    /* Conservar el duty cycle configurado */
    PWM_SetDuty(pwmDuty);

    return 1U;
}

uint8_t UART_ReadByte(uint8_t *data)
{
    if (data == NULL)
    {
        return 0U;
    }

    /* Comprobar si hay caracteres disponibles */
    if (uartRxHead == uartRxTail)
    {
        return 0U;
    }

    /* Recuperar el siguiente caracter */
    *data = uartRxBuffer[uartRxTail];

    /* Avanzar la posicion de lectura */
    uartRxTail =
        (uartRxTail + 1U) % UART_RX_BUFFER_SIZE;

    return 1U;
}

uint8_t UART_ParseFrequency(const char *text, uint32_t *frequency)
{
    uint32_t value = 0U;

    if ((text == NULL) || (frequency == NULL) || (*text == '\0'))
    {
        return 0U;
    }

    while (*text != '\0')
    {
        if ((*text < '0') || (*text > '9'))
        {
            return 0U;
        }

        value = (value * 10U) + (uint32_t)(*text - '0');

        /* Evitar valores excesivos */
        if (value > 2000U)
        {
            return 0U;
        }

        text++;
    }

    if ((value != 500U) &&
        (value != 1000U) &&
        (value != 2000U))
    {
        return 0U;
    }

    *frequency = value;

    return 1U;
}

void UART_ProcessCommand(const char *command)
{
    if (strcmp(command, "HELP") == 0)
    {
        UART_SendText(
            "OK\r\n"
            "HELP\r\n"
            "STATUS\r\n"
            "MODE MANUAL\r\n"
            "MODE AUTO\r\n"
            "DUTY n\r\n"
            "FREQ n\r\n"
            "PAUSE\r\n"
            "RESUME\r\n"
            "STREAM ON\r\n"
            "STREAM OFF\r\n"
        );
    }
    else if (strcmp(command, "STATUS") == 0)
    {
        char response[96];

        uint32_t timerClock = 64000000U;

        uint32_t prescaler =
            htim2.Instance->PSC;

        uint32_t period =
            __HAL_TIM_GET_AUTORELOAD(&htim2);

        uint32_t frequency =
            timerClock / ((prescaler + 1U) * (period + 1U));

        const char *modeText = "MANUAL";

        if (currentMode == MODE_AUTO)
        {
            modeText = "AUTO";
        }
        else if (currentMode == MODE_PAUSE)
        {
            modeText = "PAUSA";
        }

        snprintf(response, sizeof(response),
                 "OK\r\n"
                 "MODE: %s\r\n"
                 "PWM FREQ: %lu Hz\r\n"
                 "PWM DUTY: %u%%\r\n",
                 modeText,
                 (unsigned long)frequency,
                 (unsigned int)pwmDuty);

        UART_SendText(response);
    }

    else if (strncmp(command, "DUTY ", 5) == 0)
    {
        uint8_t newDuty;

        if (UART_ParseDuty(command + 5, &newDuty))
        {
            PWM_SetDuty(newDuty);

            UART_SendText("OK DUTY\r\n");
        }
        else
        {
            UART_SendText("ERR DUTY\r\n");
        }
    }
    else if (strncmp(command, "FREQ ", 5) == 0)
    {
        uint32_t newFrequency;

        if (UART_ParseFrequency(command + 5, &newFrequency))
        {
            if (PWM_SetFrequency(newFrequency))
            {
                UART_SendText("OK FREQ\r\n");
            }
            else
            {
                UART_SendText("ERR FREQ\r\n");
            }
        }
        else
        {
            UART_SendText("ERR FREQ\r\n");
        }
    }

    else if (strcmp(command, "MODE MANUAL") == 0)
    {
        if (currentMode == MODE_PAUSE)
        {
            UART_SendText("ERR PAUSED\r\n");
        }
        else
        {
            if (currentMode == MODE_AUTO)
            {
                currentMode = MODE_MANUAL;
                PWM_SetDuty(manualDuty);
            }

            UART_SendText("OK MODE MANUAL\r\n");
        }
    }
    else if (strcmp(command, "MODE AUTO") == 0)
    {
        if (currentMode == MODE_PAUSE)
        {
            UART_SendText("ERR PAUSED\r\n");
        }
        else
        {
            if (currentMode == MODE_MANUAL)
            {
                manualDuty = pwmDuty;

                autoDuty = 10U;
                autoDirection = 1;
                autoLastUpdate = g_ms;

                currentMode = MODE_AUTO;
                PWM_SetDuty(autoDuty);
            }

            UART_SendText("OK MODE AUTO\r\n");
        }
    }

    else if (strcmp(command, "PAUSE") == 0)
    {
        if (currentMode == MODE_PAUSE)
        {
            UART_SendText("ERR ALREADY PAUSED\r\n");
        }
        else
        {
            /* Guardar el estado y duty actuales */
            previousMode = currentMode;
            savedDuty = pwmDuty;

            /* Entrar en PAUSA */
            currentMode = MODE_PAUSE;
            PWM_SetDuty(0U);

            UART_SendText("OK PAUSE\r\n");
        }
    }
    else if (strcmp(command, "RESUME") == 0)
    {
        if (currentMode != MODE_PAUSE)
        {
            UART_SendText("ERR NOT PAUSED\r\n");
        }
        else
        {
            /* Recuperar estado y duty anteriores */
            currentMode = previousMode;
            PWM_SetDuty(savedDuty);

            /* Evitar acumular tiempo durante PAUSA */
            autoLastUpdate = g_ms;

            UART_SendText("OK RESUME\r\n");
        }
    }
    else if (strcmp(command, "STREAM ON") == 0)
    {
        streamEnabled = 1U;
        streamLastUpdate = g_ms;

        UART_SendText("OK STREAM ON\r\n");
    }
    else if (strcmp(command, "STREAM OFF") == 0)
    {
        streamEnabled = 0U;

        UART_SendText("OK STREAM OFF\r\n");
    }
    else
    {
        UART_SendText("ERR COMMAND\r\n");
    }
}

void UART_ProcessRx(void)
{
    uint8_t receivedByte;
    uint16_t processed = 0U;

    /* Limitar trabajo por iteracion del programa principal */
    while ((processed < 32U) && UART_ReadByte(&receivedByte))
    {
        processed++;

        /* Evitar procesar CRLF dos veces */
        if ((receivedByte == '\n') && uartLastWasCR)
        {
            uartLastWasCR = 0U;
            continue;
        }

        uartLastWasCR = (receivedByte == '\r') ? 1U : 0U;

        /* Detectar final de linea */
        if ((receivedByte == '\r') || (receivedByte == '\n'))
        {
            if (uartLineDiscard)
            {
                uartLineOverflowCount++;
            }
            else if (uartLineLength > 0U)
            {
                uartLine[uartLineLength] = '\0';



                UART_ProcessCommand(uartLine);
            }

            /* Preparar la siguiente linea */
            uartLineLength = 0U;
            uartLineDiscard = 0U;
        }
        else if (!uartLineDiscard)
        {
            if (uartLineLength < UART_LINE_MAX_LENGTH)
            {
                uartLine[uartLineLength++] = (char)receivedByte;
            }
            else
            {
                /* Descartar hasta el siguiente fin de linea */
                uartLineDiscard = 1U;
            }
        }
    }
}

uint8_t UART_SendText(const char *text)

               {
                   uint16_t length = 0U;
                   uint16_t freeSpace;

                   if (text == NULL)
                   {
                       return 0U;
                   }

                   /* Calcular longitud del mensaje */
                   while (text[length] != '\0')
                   {
                       length++;

                       if (length >= UART_TX_BUFFER_SIZE)
                       {
                           uartTxOverflowCount++;
                           return 0U;
                       }
                   }

                   /* Calcular espacio disponible */
                   uint32_t primask = __get_PRIMASK();

                   __disable_irq();

                   uint16_t used =
                       (uartTxHead + UART_TX_BUFFER_SIZE - uartTxTail)
                       % UART_TX_BUFFER_SIZE;

                   freeSpace = (UART_TX_BUFFER_SIZE - 1U) - used;

                   __set_PRIMASK(primask);

                   if (length > freeSpace)
                   {
                       uartTxOverflowCount++;
                       return 0U;
                   }

                   /* Agregar mensaje al buffer */
                   for (uint16_t i = 0U; i < length; i++)
                   {
                       uartTxBuffer[uartTxHead] = (uint8_t)text[i];

                       uartTxHead =
                           (uartTxHead + 1U) % UART_TX_BUFFER_SIZE;
                   }

                   return 1U;
               }

               void UART_ServiceTx(void)
               {
                   if (uartTxBusy != 0U)
                   {
                       return;
                   }

                   if (uartTxHead == uartTxTail)
                   {
                       return;
                   }

                   uartTxActiveByte = uartTxBuffer[uartTxTail];
                   uartTxBusy = 1U;

                   if (HAL_UART_Transmit_IT(&huart2,
                                            &uartTxActiveByte,
                                            1U) != HAL_OK)
                   {
                       uartTxBusy = 0U;
                   }
               }
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */
  

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART2_UART_Init();
  MX_TIM2_Init();
  MX_TIM6_Init();
  /* USER CODE BEGIN 2 */

  HAL_TIM_Base_Start_IT(&htim6);
  HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);
  PWM_SetDuty(25);

  if (HAL_UART_Receive_IT(&huart2, &uartRxByte, 1U) != HAL_OK)
  {
      Error_Handler();
  }

  buttonRawState =
      HAL_GPIO_ReadPin(USER_BTN_GPIO_Port, USER_BTN_Pin);

  buttonStableState = buttonRawState;

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

	  UART_ProcessRx();
	  UART_ServiceTx();

	  uint32_t now = g_ms;

	  /* Se detectó un flanco mediante EXTI */
	  if (buttonEdgePending)
	  {
	      buttonEdgePending = 0;

	      buttonRawState =
	          HAL_GPIO_ReadPin(USER_BTN_GPIO_Port, USER_BTN_Pin);

	      debounceStart = now;
	      debounceActive = 1;
	  }

	  /* Validación no bloqueante de 30 ms */
	  if (debounceActive)
	  {
	      uint8_t currentState =
	          HAL_GPIO_ReadPin(USER_BTN_GPIO_Port, USER_BTN_Pin);

	      /* Si vuelve a cambiar, reiniciamos los 30 ms */
	      if (currentState != buttonRawState)
	      {
	          buttonRawState = currentState;
	          debounceStart = now;
	      }

	      /* Debe permanecer estable durante 30 ms */
	      if ((uint32_t)(now - debounceStart) >= 30U)
	      {
	          debounceActive = 0;

	          if (buttonStableState != buttonRawState)
	          {
	              buttonStableState = buttonRawState;

	              /* B1 es activo en bajo */
	              if (buttonStableState == GPIO_PIN_RESET)
	              {
	                  /* Pulsación validada */
	                  buttonPressed = 1;
	                  pressStart = now;
	                  longPressReported = 0;
	              }
	              else
	              {
	                  /* Liberación validada */
	                  if (buttonPressed)
	                  {
	                      if (!longPressReported)
	                      {
	                          uint32_t pressDuration =
	                              (uint32_t)(now - pressStart);

	                          if (pressDuration >= 30U &&
	                              pressDuration < 1500U)
	                          {
	                              shortPressCount++;

	                              /* Alternar entre MANUAL y AUTO */
	                              if (currentMode == MODE_MANUAL)
	                              {
	                                  /* Guardar la configuracion manual */
	                                  manualDuty = pwmDuty;

	                                  /* Iniciar la rampa desde 10% */
	                                  autoDuty = 10U;
	                                  autoDirection = 1;
	                                  autoLastUpdate = now;

	                                  currentMode = MODE_AUTO;
	                                  PWM_SetDuty(autoDuty);
	                              }
	                              else if (currentMode == MODE_AUTO)
	                              {
	                                  /* Recuperar el duty manual */
	                                  currentMode = MODE_MANUAL;
	                                  PWM_SetDuty(manualDuty);
	                              }
	                          }
	                      }

	                      buttonPressed = 0;
	                  }
	              }
	          }
	      }
	  }
	  /* Detección de pulsación larga */
	  if (buttonPressed && !longPressReported)
	  {
	      if ((uint32_t)(now - pressStart) >= 1500U)
	      {
	          longPressReported = 1;
	          longPressCount++;

	          /* Entrar o salir del modo PAUSA */
	          if (currentMode != MODE_PAUSE)
	          {
	              /* Guardar el modo y duty anteriores */
	              previousMode = currentMode;
	              savedDuty = pwmDuty;

	              /* Activar PAUSA y apagar PWM */
	              currentMode = MODE_PAUSE;
	              PWM_SetDuty(0U);
	          }
	          else
	          {
	              /* Recuperar el modo y duty anteriores */
	              currentMode = previousMode;
	              PWM_SetDuty(savedDuty);
	          }
	      }
	  }

	  /* Rampa automatica PWM */
	  if (currentMode == MODE_AUTO)
	  {
	      if ((uint32_t)(now - autoLastUpdate) >= 20U)
	      {
	          autoLastUpdate = now;

	          if (autoDirection > 0)
	          {
	              if (autoDuty < 90U)
	              {
	                  autoDuty++;
	              }
	              else
	              {
	                  autoDirection = -1;
	                  autoDuty--;
	              }
	          }
	          else
	          {
	              if (autoDuty > 10U)
	              {
	                  autoDuty--;
	              }
	              else
	              {
	                  autoDirection = 1;
	                  autoDuty++;
	              }
	          }

	          PWM_SetDuty(autoDuty);
	      }
	  }
	  else
	  {
	      /* Evita acumular tiempo fuera de AUTO */
	      autoLastUpdate = now;
	  }

	  /* Transmision periodica de estado por UART */
	  if (streamEnabled)
	  {
	      if ((uint32_t)(now - streamLastUpdate) >= 1000U)
	      {
	          streamLastUpdate = now;

	          const char *modeText = "MANUAL";

	          if (currentMode == MODE_AUTO)
	          {
	              modeText = "AUTO";
	          }
	          else if (currentMode == MODE_PAUSE)
	          {
	              modeText = "PAUSA";
	          }

	          uint32_t timerClock = 64000000U;
	          uint32_t prescaler = htim2.Instance->PSC;
	          uint32_t period = __HAL_TIM_GET_AUTORELOAD(&htim2);

	          uint32_t frequency =
	              timerClock / ((prescaler + 1U) * (period + 1U));

	          char streamMessage[96];

	          snprintf(streamMessage, sizeof(streamMessage),
	                   "STREAM MODE:%s FREQ:%lu DUTY:%u%%\r\n",
	                   modeText,
	                   (unsigned long)frequency,
	                   (unsigned int)pwmDuty);

	          UART_SendText(streamMessage);
	      }
	  }
  }


  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL16;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 63;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 999;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV2;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_PWM_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 250;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */
  HAL_TIM_MspPostInit(&htim2);

}

/**
  * @brief TIM6 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM6_Init(void)
{

  /* USER CODE BEGIN TIM6_Init 0 */

  /* USER CODE END TIM6_Init 0 */

  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM6_Init 1 */

  /* USER CODE END TIM6_Init 1 */
  htim6.Instance = TIM6;
  htim6.Init.Prescaler = 63;
  htim6.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim6.Init.Period = 999;
  htim6.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim6) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim6, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM6_Init 2 */

  /* USER CODE END TIM6_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  huart2.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : USER_BTN_Pin */
  GPIO_InitStruct.Pin = USER_BTN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(USER_BTN_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : LD2_Pin */
  GPIO_InitStruct.Pin = LD2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LD2_GPIO_Port, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

}

/* USER CODE BEGIN 4 */

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM6)
    {
        g_ms++;
    }
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if (GPIO_Pin == USER_BTN_Pin)
    {
        buttonEdgePending = 1;
    }
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART2)
    {
        uint16_t nextHead =
            (uartRxHead + 1U) % UART_RX_BUFFER_SIZE;

        if (nextHead != uartRxTail)
        {
            uartRxBuffer[uartRxHead] = uartRxByte;
            uartRxHead = nextHead;
        }
        else
        {
            uartRxOverflowCount++;
        }

        HAL_UART_Receive_IT(&huart2, &uartRxByte, 1U);
    }
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART2)
    {
        uartTxTail =
            (uartTxTail + 1U) % UART_TX_BUFFER_SIZE;

        uartTxBusy = 0U;
    }
}

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */

  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(char *file, uint32_t line)
{ 
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
