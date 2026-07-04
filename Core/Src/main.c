/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Acordor Mandolina - Editie Revizuita PRO (High-Res ZCD)
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "i2c-lcd.h" // Includem biblioteca pentru ecranul LCD
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef enum {
    STARE_IDLE = 0,
    STARE_LISTENING,
    STARE_TUNING
} StareEcran;
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;
I2C_HandleTypeDef hi2c1;
UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_ADC1_Init(void);
static void MX_I2C1_Init(void);

/* USER CODE BEGIN PFP */
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

// Functie pentru desenarea barei de acordare pe ecran
void afisare_lcd_acordor(float eroare_hz, const char* nume_coarda, int grade) {
    char rand1_temp[32];
    char rand1_final[17];
    char rand2[17] = "-------|------- "; // Exact 16 caractere. Centrul matematic este la indexul 7.
    
    int pozitie = 7;

    // Daca eroarea e in intervalul de toleranta acustica (max +/- 1 Hz)
    if (eroare_hz >= -1.0f && eroare_hz <= 1.0f) {
        snprintf(rand1_temp, sizeof(rand1_temp), "%-6s PERFECT!", nume_coarda);
        pozitie = 7; // Fortam cursorul fix pe bara din mijloc
    } else {
        // Prevenim afisarea "0 grade" cand instrumentul inca nu e perfect acordat
        int afisare_grade = (grade == 0) ? 1 : grade;
        
        // Corelam VIZUALUL barei direct cu numarul de GRADE
        // Impartim la 2 pentru a avea o tranzitie lina (ex: 1-2 grade = offset 1, 3-4 grade = offset 2)
        int offset = (afisare_grade + 1) / 2; 
        if (offset > 7) offset = 7; // Limita ecranului
        
        if (eroare_hz < 0) { // Frecventa curenta e mai mica -> trebuie STRANS
            snprintf(rand1_temp, sizeof(rand1_temp), "%-6s Str:%d%c ", nume_coarda, afisare_grade, 223);
            pozitie = 7 - offset; // Cursorul se misca proportional la stanga
        } else { // Frecventa curenta e mai mare -> trebuie SLABIT
            snprintf(rand1_temp, sizeof(rand1_temp), "%-6s Sla:%d%c ", nume_coarda, afisare_grade, 223);
            pozitie = 7 + offset; // Cursorul se misca proportional la dreapta
        }
    }

    // Limitare stricta la lungimea barei pentru a preveni erorile de memorie LCD
    if (pozitie < 0) pozitie = 0;
    if (pozitie > 14) pozitie = 14;

    // Punem cursorul (O) pe pozitia calculata
    rand2[pozitie] = 'O'; 

    // Asiguram ca randul 1 are exact 16 caractere (umplem cu spatii goale) pt a sterge ce era inainte
    snprintf(rand1_final, sizeof(rand1_final), "%-16s", rand1_temp);

    lcd_put_cur(0, 0);
    lcd_send_string(rand1_final);
    lcd_put_cur(1, 0);
    lcd_send_string(rand2);
}

// Functie auxiliara pentru afisare text simplu pe 2 randuri (maxim 16 caractere per rand)
void afisare_lcd_text(const char* linie1, const char* linie2) {
    char text1[17];
    char text2[17];

    snprintf(text1, sizeof(text1), "%-16s", linie1);
    snprintf(text2, sizeof(text2), "%-16s", linie2);

    lcd_put_cur(0, 0);
    lcd_send_string(text1);
    lcd_put_cur(1, 0);
    lcd_send_string(text2);
}

// Functie de mapare a frecventei calculate cu corzile mandolinei
bool determina_nota_si_parametri(float frecventa_hz, char* nume_coarda, float* target_freq, float* grade_per_hz) {
    if (frecventa_hz >= 540.0f && frecventa_hz < 800.0f) {
        strcpy(nume_coarda, "Mi(E)");
        *target_freq = 659.25f;
        *grade_per_hz = 0.5f; // Mandolina are tensiune mare: jumatate de grad schimba 1 Hz
        return true;
    } else if (frecventa_hz >= 350.0f && frecventa_hz < 540.0f) {
        strcpy(nume_coarda, "La(A)");
        *target_freq = 440.0f;
        *grade_per_hz = 0.8f;
        return true;
    } else if (frecventa_hz >= 240.0f && frecventa_hz < 350.0f) {
        strcpy(nume_coarda, "Re(D)");
        *target_freq = 293.66f;
        *grade_per_hz = 1.2f;
        return true;
    } else if (frecventa_hz >= 140.0f && frecventa_hz < 240.0f) {
        strcpy(nume_coarda, "Sol(G)");
        *target_freq = 196.00f;
        *grade_per_hz = 1.8f;
        return true;
    }
    return false;
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
  MX_ADC1_Init();
  MX_I2C1_Init();
  
  /* USER CODE BEGIN 2 */
  // =========================================================================
  // ACTIVARE DWT (Data Watchpoint and Trace) PENTRU PRECIZIE DE NANOSECUNDE
  // =========================================================================
  CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
  DWT->CYCCNT = 0;
  DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;

  HAL_Delay(200); // Pauza de stabilizare tensiune la pornire
  HAL_ADC_Start(&hadc1); // Pornim convertorul analog-digital in mod continuu

  lcd_init();            
  lcd_clear();
  lcd_put_cur(0, 0);
  lcd_send_string("Acordor Mandolină");
  lcd_put_cur(1, 0);
  lcd_send_string("   Pornire...   ");
  HAL_Delay(1500); 
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
      static char current_note[10] = "";
      static float current_freq = 0.0f;
      static float current_target_freq = 0.0f;
      static float current_grade_per_hz = 0.0f;
      
      static uint8_t candidate_count = 0;
      static uint32_t last_valid_tick = 0;
      static uint32_t last_display_tick = 0;
      static StareEcran stare_ecran = STARE_IDLE;
      
      // Variabile pentru Acumulatorul de Medie si Stabilitate
      static float sum_frecvente = 0.0f;
      static uint8_t numar_masuratori = 0;

      const uint32_t prag_sunet = 110;       // Prag sensibilitate microfon
      const uint32_t hold_note_ms = 4000;    // Mentinere nota pe ecran (4 secunde)

      // ==============================================================
      // ETAPA 1: Achizitie pentru zgomot si volum (30ms)
      // ==============================================================
      uint32_t val_max = 0;
      uint32_t val_min = 4095;
      uint32_t start_test = HAL_GetTick();
      
      while((HAL_GetTick() - start_test) < 30) {
          if (HAL_ADC_PollForConversion(&hadc1, 1) == HAL_OK) {
              uint32_t v = HAL_ADC_GetValue(&hadc1);
              if(v > val_max) val_max = v;
              if(v < val_min) val_min = v;
          }
      }
      
      uint32_t amplitudine = val_max - val_min;
      uint32_t tick = HAL_GetTick();

      // ==============================================================
      // ETAPA 2: Procesare Semnal si Detecție de Frecvență (ZCD)
      // ==============================================================
      if (amplitudine > prag_sunet) {
          
          if (stare_ecran == STARE_IDLE) {
              HAL_Delay(80); // Ignoram atacul (bufnitul) initial al penei
              stare_ecran = STARE_LISTENING;
              continue;
          }

          uint32_t dc_offset = (val_max + val_min) / 2;
          uint32_t prag_sus = dc_offset + (amplitudine / 6); 
          uint32_t prag_jos = dc_offset - (amplitudine / 6);
          
          uint32_t start_freq = HAL_GetTick();
          uint32_t first_cross_cycles = 0;
          uint32_t last_cross_cycles = 0;
          uint32_t cycle_count = 0;
          uint8_t stare = 0;
          
          // Ascultam semnalul (timeout 200ms)
          while((HAL_GetTick() - start_freq) < 200) { 
              if (HAL_ADC_PollForConversion(&hadc1, 1) == HAL_OK) {
                  uint32_t v = HAL_ADC_GetValue(&hadc1);
                  
                  if (v > prag_sus && stare == 0) {
                      stare = 1;
                      uint32_t current_cycles = DWT->CYCCNT; 

                      if (cycle_count == 0) {
                          first_cross_cycles = current_cycles;
                      } else {
                          last_cross_cycles = current_cycles;
                      }
                      cycle_count++;
                      
                      if (cycle_count >= 30) break; 
                  } else if (v < prag_jos && stare == 1) {
                      stare = 0;
                  }
              }
          }
          
          if (cycle_count >= 3) {
              // CALCUL MATEMATIC ULTRA-PRECIS
              // Masuram diferenta exacta in cicli de procesor intre prima si ultima unda
              uint32_t delta_cycles = last_cross_cycles - first_cross_cycles;
              
              // Transformam ciclii in secunde reale, folosind frecventa bruta a procesorului (ex: 72 MHz)
              float duration_sec = (float)delta_cycles / (float)HAL_RCC_GetHCLKFreq();
              
              // Frecventa = numarul de unde depline impartit la timpul extrem de precis luat de ele
              float frecventa_hz = (float)(cycle_count - 1) / duration_sec;
              
              if (frecventa_hz > 100.0f && frecventa_hz < 1000.0f) {
                  float target_freq = 0.0f;
                  float grade_per_hz = 0.0f;
                  char nume_coarda[10] = "";

                  if (determina_nota_si_parametri(frecventa_hz, nume_coarda, &target_freq, &grade_per_hz)) {
                      
                      // Daca se detecteaza o nota DIVERITĂ fata de cea de pe ecran
                      if (strcmp(current_note, nume_coarda) != 0) {
                          candidate_count++;
                          if (candidate_count >= 3) { // Debouncing pt schimbarea corzii
                              strcpy(current_note, nume_coarda);
                              current_target_freq = target_freq;
                              current_grade_per_hz = grade_per_hz;
                              
                              // Resetam bufferul de medie deoarece e o nota noua
                              sum_frecvente = 0.0f;
                              numar_masuratori = 0;
                              candidate_count = 0;
                          }
                      } else {
                          candidate_count = 0; // Confirmam aceeasi coarda
                      }

                      // Daca suntem pe o nota confirmata, aplicam Logica de Stabilitate
                      if (current_note[0] != '\0' && strcmp(current_note, nume_coarda) == 0) {
                          
                          // OUTLIER REJECTION: Daca frecventa noua sare brusc cu > 8 Hz (ciupitura noua)
                          if (numar_masuratori > 0) {
                              float media_temporara = sum_frecvente / numar_masuratori;
                              if (frecventa_hz > media_temporara + 8.0f || frecventa_hz < media_temporara - 8.0f) {
                                  // Secventa rupta (zgomot penei la o noua ciupire). Resetam contorul!
                                  sum_frecvente = 0.0f;
                                  numar_masuratori = 0;
                              }
                          }
                          
                          // Adaugam citirea (care stim acum ca e stabila)
                          sum_frecvente += frecventa_hz;
                          numar_masuratori++;
                          last_valid_tick = tick;
                          
                          // Cand am adunat 4 masuratori STABILE, afisam ecranul
                          if (numar_masuratori >= 4) {
                              current_freq = sum_frecvente / 4.0f; // Frecventa MEDIE perfecta
                              
                              float eroare_hz = current_freq - current_target_freq; 
                              
                              // Aplicam un filtru de "Lock-In" (Daca eroarea e infima, o anulam ca sa nu palpaie cursorul)
                              if (eroare_hz > -0.15f && eroare_hz < 0.15f) {
                                  eroare_hz = 0.0f; 
                              }

                              int grade_rotatie = (int)((eroare_hz > 0) ? (eroare_hz * current_grade_per_hz) : (-eroare_hz * current_grade_per_hz));
                              
                              afisare_lcd_acordor(eroare_hz, current_note, grade_rotatie);
                              
                              last_display_tick = tick;
                              stare_ecran = STARE_TUNING;
                              
                              // ROLLING WINDOW: Pastram ultimele 3 valori
                              sum_frecvente = current_freq * 3.0f; 
                              numar_masuratori = 3;
                          }
                      }
                      
                      // Transmitere Debug Serial pentru Teleplot (Frecventa castata la intreg pentru afisare vizuala rapida)
                      char buf[128];
                      snprintf(buf, sizeof(buf), ">Freq:%d\n>Amp:%lu\n", (int)frecventa_hz, amplitudine);
                      HAL_UART_Transmit(&huart2, (uint8_t*)buf, strlen(buf), 10);
                  }
              }
          }
      } else {
          // ==============================================================
          // ETAPA 3: Logica de Liniste (Silence & Timeout)
          // ==============================================================
          if (current_note[0] != '\0' && (tick - last_valid_tick) < hold_note_ms) {
              // Mentinem ultima nota pe ecran
          } else if (stare_ecran != STARE_IDLE && (tick - last_valid_tick) >= hold_note_ms) {
              current_note[0] = '\0';
              current_freq = 0.0f;
              
              // Stergem și mediile din memorie la liniste
              sum_frecvente = 0.0f;
              numar_masuratori = 0;
              
              afisare_lcd_text(" Astept sunet... ", "                ");
              last_display_tick = tick;
              stare_ecran = STARE_IDLE;
          }
      }
    /* USER CODE END WHILE */
    /* USER CODE BEGIN 3 */
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
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI_DIV2;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                                |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV2;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{
  ADC_ChannelConfTypeDef sConfig = {0};
  hadc1.Instance = ADC1;
  hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc1.Init.ContinuousConvMode = ENABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 1;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }
  sConfig.Channel = ADC_CHANNEL_0;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_71CYCLES_5;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 100000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
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
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);

  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = LD2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LD2_GPIO_Port, &GPIO_InitStruct);

  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);
}

void Error_Handler(void)
{
  __disable_irq();
  while (1)
  {
  }
}

#ifdef  USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line)
{
}
#endif /* USE_FULL_ASSERT */