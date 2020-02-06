#ifndef __KB_MANAGER_H__
#define __KB_MANAGER_H__

#define KEY_1 0
#define KEY_2 1
#define KEY_3 2
#define KEY_W 3
#define KEY_A 4
#define KEY_S 5
#define KEY_D 6
#define KEY_P 7
#define KEY_SPACE 8
#define KEY_ESC 9


typedef uint16_t keys_t;

/**
 * @brief Bitmasks for key state tracking
 * 
 */
typedef struct {
  keys_t keys_hold; /**< Bitmask for keys being held */
  keys_t keys_down; /**< Bitmask for keys pressed in the current frame */
  keys_t keys_up;   /**< Bitmask for keys released in the current frame */
} kb_state_t;

/**
 * @brief Initialize keyboard manager and device
 * 
 * @param out_bit bit value to be filled on subscribe
 * @return int execution return code
 */
int kb_init(uint8_t* out_bit);

/**
 * @brief Clean up keyboard manager and unsubscribe from device
 * 
 */
void kb_destroy();

/**
 * @brief Set key in bitmask as being pressed
 * 
 * @param key bit corresponding to a scancode
 */
void kb_set_key_down(uint8_t key);

/**
 * @brief Set key in bitmask as being not pressed
 * 
 * @param key bit corresponding to a scancode
 */
void kb_set_key_up(uint8_t key);

/**
 * @brief Set all bits in bitmasks to zero
 * 
 */
void kb_clear_keys_down_up();

/**
 * @brief Check if key is being held
 * 
 * @param key bit corresponding to a scancode
 * @return int 1 for held or 0 for not held
 */
int kb_is_key_held(uint8_t key);

/**
 * @brief Check if key is down in this frame
 * 
 * @param key bit corresponding to a scancode
 * @return int 1 for held or 0 for not held
 */
int kb_is_key_down(uint8_t key);

/**
 * @brief Check if key is up in this frame
 * 
 * @param key bit corresponding to a scancode
 * @return int 1 for held or 0 for not held
 */
int kb_is_key_up(uint8_t key);

/**
 * @brief Read scancodes from KBC and updates the key bitmasks.
 * (Function to be called with every keyboard interruption)
 * 
 * @return int 0 if a new scancode was processed. Otherwise -1.
 */
int kb_update_keys();

/**
 * @brief Print key bitmasks
 * 
 */
void print_kb_state();

#endif /* __KB_MANAGER_H__ */
