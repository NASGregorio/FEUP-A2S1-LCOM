#include <stdio.h>
#include <stdlib.h>  
#include <stdbool.h>

#define BIT(n) (1 << (n))
#define TIMER_LSB     BIT(4)                  /**< @brief Initialize Counter LSB only */
#define TIMER_MSB     BIT(5)                  /**< @brief Initialize Counter MSB only */
#define TIMER_LSB_MSB (TIMER_LSB | TIMER_MSB) /**< @brief Initialize LSB first and MSB afterwards */
#define TIMER_BCD 0x01 /**< @brief Count in BCD */


int test(int port, __uint8_t *value) {

	__uint32_t value32 = port;


	__uint8_t* b = (__uint8_t*)&value32;

	__uint8_t value80 = b[0];
	__uint8_t value81 = b[1];
	__uint8_t value82 = b[2];
	__uint8_t value83 = b[3];

	fprintf(stderr, "%d:", value83);
	fprintf(stderr, "%d:", value82);
	fprintf(stderr, "%d:", value81);
	fprintf(stderr, "%d\n", value80);

	//*value = value80;
	*value = ((__uint8_t*)&value32)[0];

	return 0;
}

enum timer_init {
	INVAL_val,    /*!< Invalid initialization mode */
	LSB_only,     /*!< Initialization only of the LSB */
	MSB_only,     /*!< Initialization only of the MSB */
	MSB_after_LSB /*!< Initialization of LSB and MSB, in this order */
};

union timer_status_field_val {
	__uint8_t byte;            /*!< status */
	enum timer_init in_mode; /*!< initialization mode */
	__uint8_t count_mode;      /*!< counting mode: 0, 1,.., 5 */
	bool bcd;                /*!< counting base, true if BCD */
};

const char *bit_rep[16] = {
    [ 0] = "0000", [ 1] = "0001", [ 2] = "0010", [ 3] = "0011",
    [ 4] = "0100", [ 5] = "0101", [ 6] = "0110", [ 7] = "0111",
    [ 8] = "1000", [ 9] = "1001", [10] = "1010", [11] = "1011",
    [12] = "1100", [13] = "1101", [14] = "1110", [15] = "1111",
};

void print_byte(int d, __uint8_t byte)
{
    printf("%d: %s%s\n", d, bit_rep[byte >> 4], bit_rep[byte & 0x0F]);
}

int main(int argc, char const *argv[])
{
	__uint16_t val = 1025;

	__uint8_t lsb = (__uint8_t)val;
	__uint8_t msb = val >> 8;
	fprintf(stderr, "lsb: %u\n", lsb);
	fprintf(stderr, "msb: %u\n", msb);


	//__uint8_t st = 0b11111101;

	// union timer_status_field_val conf;
	// conf.byte = st & (BIT(7) | BIT(6));
	// //fprintf(stderr, "76: %u\n", conf.byte);
	// print_byte(76, conf.byte);


	// //__uint8_t in_mode = st & TIMER_LSB_MSB; //BITs 4 and 5
    // __uint8_t in_mode = (st & TIMER_LSB_MSB) >> 4; //BITs 4 and 5
	// fprintf(stderr, "00: %u\n", in_mode);
	// print_byte(00, in_mode);


	// switch (in_mode)
	// {
	// case TIMER_LSB:
	// 	conf.in_mode = LSB_only;
	// 	break;
	// case TIMER_MSB:
	// 	conf.in_mode = MSB_only;
	// 	break;
	// case TIMER_LSB_MSB:
	// 	conf.in_mode = MSB_after_LSB;
	// 	break;
	// default:
	// 	fprintf(stderr, "Invalid initialization mode: %u\n", in_mode);
	// 	break;
	// }

	// conf.byte |= (st & TIMER_LSB_MSB);

	// //fprintf(stderr, "54: %u\n", conf.byte);
	// print_byte(54, conf.byte);


	// __uint8_t count_mode = (st & (BIT(3) | BIT(2) | BIT(1))) >> 1; //BITs 1, 2 and 3
	// count_mode -= (count_mode > 5) ? 4 : 0;
	// conf.byte |= (count_mode << 1);
	// print_byte(123, conf.byte);


  	// conf.byte |= (st & TIMER_BCD); //BIT 0

	// //fprintf(stderr, "%u\n", conf.byte);
	// print_byte(0, conf.byte);



	// switch (timer) //BIT 4&5
	// {
	// case 0:
	//   conf.byte &= ~(TIMER_SEL1 | TIMER_SEL2);
	//   break;
	// case 1:
	//   conf.byte |= TIMER_SEL1;
	//   break;
	// case 2:
	//   conf.byte |= TIMER_SEL2;
	//   break;
	// default:
	//   err = EINVAL;
	//   break;
	// }
	return 0;
}
