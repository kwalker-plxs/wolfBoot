#include <image.h>
#include <string.h>

#include "stm32h7xx_hal.h"

__attribute__((
    section(".flash_buffer"))) uint8_t sector_buffer[FLASH_SECTOR_SIZE];

static void RAMFUNCTION clockconfig() {
  /* Use the HAL library's default clock configuration */
  HAL_Init();
  /* System clock configuration */
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Supply configuration update enable
   */
  HAL_PWREx_ConfigSupply(PWR_DIRECT_SMPS_SUPPLY);

  /** Configure the main internal regulator output voltage
   */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  while (!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {
  }

  /** Initializes the RCC Oscillators according to the specified parameters
   * in the RCC_OscInitTypeDef structure.
   */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_DIV1;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 50;
  RCC_OscInitStruct.PLL.PLLP = 2;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;
  RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1VCIRANGE_3;
  RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOWIDE;
  RCC_OscInitStruct.PLL.PLLFRACN = 0;
  HAL_RCC_OscConfig(&RCC_OscInitStruct);

  /** Initializes the CPU, AHB and APB buses clocks
   */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK |
                                RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2 |
                                RCC_CLOCKTYPE_D3PCLK1 | RCC_CLOCKTYPE_D1PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV2;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV16;
  RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV2;

  HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2);
}

void RAMFUNCTION hal_init(void) { clockconfig(); }

void RAMFUNCTION hal_flash_unlock(void) { HAL_FLASH_Unlock(); }

void RAMFUNCTION hal_flash_lock(void) { HAL_FLASH_Lock(); }

static uint32_t RAMFUNCTION GetSector(uint32_t address) {
  uint32_t sector = 0;

  if (address < (FLASH_BASE + FLASH_BANK_SIZE)) {
    sector = (address - FLASH_BASE) / FLASH_SECTOR_SIZE;
  } else {
    sector = (address - (FLASH_BASE + FLASH_BANK_SIZE)) / FLASH_SECTOR_SIZE;
  }

  return sector;
}

static uint32_t RAMFUNCTION GetBank(uint32_t address) {
#if defined(DUAL_BANK)
  if (IS_FLASH_PROGRAM_ADDRESS_BANK1(address)) {
    return FLASH_BANK_1;
  } else {
    return FLASH_BANK_2;
  }
#else
  return FLASH_BANK_1;
#endif
}

static void RAMFUNCTION flash_clear_errors(void) {
  __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_ALL_ERRORS_BANK1);

#if defined(DUAL_BANK)
  __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_ALL_ERRORS_BANK2);
#endif
}

// len is number of bytes to write
int hal_flash_write(uint32_t address, const uint8_t *data, int len) {
  uint32_t sector = GetSector(address);
  uint32_t sector_base_address = (address & ~(FLASH_SECTOR_SIZE - 1));

  // If the data spans across a sector boundary, this implementation will fail
  // and would need to be extended to handle multiple sectors.
  // We assume the write is confined to a single sector for simplicity.
  if (GetSector(address + len - 1) != sector) {
    return HAL_ERROR;  // Data spans multiple sectors.
  }

  // read existing data
  memcpy(sector_buffer, (void *)sector_base_address, FLASH_SECTOR_SIZE);

  // modify with new data
  // Calculate the offset into the sector buffer where the new data starts.
  uint32_t buffer_offset = address - sector_base_address;
  // Apply the new, arbitrary-sized data to the buffer.
  memcpy(&sector_buffer[buffer_offset], data, len);

  // write new data
  for (uint32_t i = 0; i < FLASH_SECTOR_SIZE; i += STM32H7_WORD_SIZE_BYTES) {
    // The address is naturally aligned by the loop increment (i).
    uint32_t target_address = sector_base_address + i;
    uint64_t *word_data =
        (uint64_t *)&sector_buffer[i];  // Cast to 64-bit for 8-byte transfers
    if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_FLASHWORD, target_address,
                          (uint32_t)word_data) != HAL_OK) {
      return -1;
    }
  }
  return 0;
}

int RAMFUNCTION hal_flash_erase(uint32_t address, int len) {
  uint32_t FirstSector = 0, LastSector = 0, NbOfSectors = 0, BankNumber = 0;
  uint32_t SectorError = 0;
  FLASH_EraseInitTypeDef EraseInitStruct;
  HAL_StatusTypeDef status = 0;
  flash_clear_errors();

  if (len == 0) {
    return -1;
  }

  BankNumber = GetBank(address);
  FirstSector = GetSector(address);
  LastSector = GetSector(address + len - 1);
  NbOfSectors = LastSector - FirstSector + 1;

  EraseInitStruct.TypeErase = FLASH_TYPEERASE_SECTORS;
  EraseInitStruct.Banks = BankNumber;
  EraseInitStruct.Sector = FirstSector;
  EraseInitStruct.NbSectors = NbOfSectors;
  EraseInitStruct.VoltageRange = FLASH_VOLTAGE_RANGE_2;
  status = HAL_FLASHEx_Erase(&EraseInitStruct, &SectorError);

  if (status != HAL_OK) {
    return -1;
  }
  return 0;
}

void RAMFUNCTION hal_prepare_boot(void) { /* Nothing to do */ }
